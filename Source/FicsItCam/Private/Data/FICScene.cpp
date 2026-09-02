#include "Data/FICScene.h"

#include "ArchiveObjectTOCProxy.h"
#include "BlueprintArchiveObjectDataProxy.h"
#include "BlueprintArchiveObjectTOCProxy.h"
#include "FICSubsystem.h"
#include "FICUtils.h"
#include "Misc/FileHelper.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Serialization/ObjectWriter.h"
#include "SaveCustomVersion.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateObjectReferenceCollector.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Serialization/TextReferenceCollector.h"
#include "Editor/FICEditorSubsystem.h"
#include "Engine/GameViewportClient.h"
#include "Slate/SceneViewport.h"

FCustomVersionRegistration GRegisterFICSceneVersion{FFICSceneVersion::GUID, FFICSceneVersion::Type::LatestVersion, TEXT("FicsIt-Cam Scene Version")};

void AFICScene::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	IFGSaveInterface::PostLoadGame_Implementation(saveVersion, gameVersion);

	SceneObjects.Remove(nullptr);
}

void AFICScene::MoveSceneObject(UObject* Object, int Delta) {
	int Index = SceneObjects.Find(Object);
	SceneObjects.RemoveAt(Index);
	SceneObjects.Insert(Object, UFICUtils::Modulo(Index + Delta, SceneObjects.Num()+1));
}

UFICCamera* AFICScene::GetActiveCamera(FICFrameFloat Time) {
	for (UObject* SceneObject : GetSceneObjects()) {
		UFICCamera* Camera = Cast<UFICCamera>(SceneObject);
		if (Camera) {
			if (Camera->Active.GetValue(Time)) return Camera;
		}
	}
	return nullptr;
}

UTexture* AFICScene::GetPreviewTexture() {
	if (PreviewTexture) {
		return PreviewTexture->GetTexture();
	} else {
		return nullptr;
	}
}

void AFICScene::OnTextureUpdate() {
	OnPreviewUpdate.Broadcast();
}

UE_DISABLE_OPTIMIZATION_SHIP
void AFICScene::UpdatePreview() {
	if (PreviewTexture == nullptr) {
		PreviewTexture = NewObject<UFICProceduralTexture>(this);
		PreviewTexture->OnTextureUpdate.AddDynamic(this, &AFICScene::OnTextureUpdate);
	}
	
	TSharedRef<FSequenceExporterProceduralTexture> TextureExporter = MakeShared<FSequenceExporterProceduralTexture>(PreviewTexture);
	FTextureRHIRef Target;
	UEngine* Engine = GEngine;
	ENQUEUE_RENDER_COMMAND(UpdateScenePreview)([this, &Target, Engine](FRHICommandListImmediate& RHICmdList){
		auto viewportClient = Engine->GameViewport;
		if (!viewportClient) return;
		auto viewport = viewportClient->GetGameViewport();
		if (!viewport) return;
		auto frame = viewport->GetViewportFrame();
		if (!frame) return;
		auto viewport2 = frame->GetViewport();
		if (!viewport2) return;
		FViewportRHIRef Viewport = viewport2->GetViewportRHI();
		if (Viewport) {
			Target = RHIGetViewportBackBuffer(Viewport);
		}
		auto viewport3 = viewportClient->Viewport;
		if (!viewport3) return;
		FTextureRHIRef Texture = viewport3->GetRenderTargetTexture();
		if (Texture) {
			Target = Texture;
		}
	});
	FlushRenderingCommands();
	if (Target) {
		AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(TextureExporter, MakeShared<FFICRenderTarget_Raw>(Target), true);
	}
}
UE_ENABLE_OPTIMIZATION_SHIP

bool AFICScene::IsSceneAlreadyInUse() {
	AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(this);
	AFICEditorSubsystem* EditSubSys = AFICEditorSubsystem::GetFICEditorSubsystem(this);
	FString ProcessKey = GetSceneProcessKey(SceneName);
	return SubSys->GetActiveRuntimeProcessesMap().Contains(ProcessKey) || (EditSubSys->GetActiveEditorContext() && EditSubSys->GetActiveEditorContext()->GetScene() == this);
}

class FFICSceneObjectCollector : public FArchiveUObject {
private:
	TArray<UObject*>& ObjectsToSave;

public:
	FFICSceneObjectCollector(TArray<UObject*>& toFill) : ObjectsToSave(toFill) {
		ArIsSaveGame = true;
		ArIsObjectReferenceCollector = true;
	}

	virtual FArchive& operator<<(UObject*& Object) override {
		if (!Object) return *this;
		if (Object->Implements<UFGSaveInterface>()) {
			if (IFGSaveInterface::Execute_ShouldSave(Object)) {
				ObjectsToSave.AddUnique(Object);
				Object->Serialize(*this);
			}
		}
		return *this;
	}
};

class FFICSceneTOCProxy : public FArchiveProxy {
public:
	UObject* NewOuter = nullptr;
	TMap<FString, UObject*> ObjectMap;

	FFICSceneTOCProxy(FArchive& Inner, UObject* NewOuter) : FArchiveProxy(Inner), NewOuter(NewOuter) {}

	virtual FArchive& operator<<(UObject*& Object) override {
		FString ClassPath;
		FString Name;
		FString Path;
		if (IsSaving() && Object) {
			ClassPath = Object->GetClass()->GetPathName();
			Name = Object->GetName();
			Path = Object->GetPathName();
		}

		InnerArchive << ClassPath;
		InnerArchive << Name;
		InnerArchive << Path;

		if (IsLoading()) {
			UClass* Class = FSoftClassPath(ClassPath).TryLoadClass<UObject>();
			if (Class) {
				Object = NewObject<UObject>(NewOuter, Class, FName(Name));
			}
			ObjectMap.Add(Path, Object);
		}

		return *this;
	}
};

class FFICSceneDataProxy : public FArchiveProxy {
public:
	const TMap<FString, UObject*>& ObjectReplacementMap;

	FFICSceneDataProxy(FArchive& Inner, const TMap<FString, UObject*>& ObjectReplacementMap) : FArchiveProxy(Inner), ObjectReplacementMap(ObjectReplacementMap) {}

	virtual FArchive& operator<<(UObject*& Object) override {
		FString Path;
		if (IsSaving() && Object) {
			Path = Object->GetPathName();
		}
		InnerArchive << Path;
		if (IsLoading() && !Path.IsEmpty()) {
			UObject* const* replacement = ObjectReplacementMap.Find(Path);
			if (replacement) {
				Object = *replacement;
			} else {
				Object = FSoftObjectPath(Path).TryLoad();
			}
		}
		if (Object) {
			Object->Serialize(*this);
		}
		return *this;
	}
};

void AFICScene::SerializeScene(TArray<UObject*>& ObjectsToSave, FArchive& Ar) {
	uint64 numObjects = ObjectsToSave.Num();
	Ar << numObjects;
	ObjectsToSave.SetNumZeroed(numObjects);

	FFICSceneTOCProxy TOC(Ar, this);
	for (UObject*& Object : ObjectsToSave) {
		TOC << Object;
	}

	FFICSceneDataProxy DataProxy(Ar, TOC.ObjectMap);
	for (UObject* Object : ObjectsToSave) {
		DataProxy << Object;
	}
	this->Serialize(DataProxy);
}
UE_DISABLE_OPTIMIZATION_SHIP
void AFICScene::SaveToFile(const FString& Path) {
	TArray<UObject*> ObjectsToSave;
	FFICSceneObjectCollector Collector(ObjectsToSave);
	this->Serialize(Collector);

	TArray<uint8> Data;
	FMemoryWriter Ar(Data);
	Ar.ArIsSaveGame = true;
	Ar.UsingCustomVersion(FSaveCustomVersion::GUID);
	Ar.UsingCustomVersion(FFICSceneVersion::GUID);
	SerializeScene(ObjectsToSave, Ar);
	Ar.Close();

	if (!FFileHelper::SaveArrayToFile(Data, *Path)) {
		FSlateNotificationManager::Get().AddNotification(FNotificationInfo(FText::FromString("Failed to save scene file: " + Path)));
	}
}
UE_ENABLE_OPTIMIZATION_SHIP
void AFICScene::LoadFromFile(const FString& Path) {
	auto subsys = AFICEditorSubsystem::GetFICEditorSubsystem(this);

	subsys->CloseEditor();
	FString name = this->SceneName;

	TArray<uint8> Data;
	if (!FFileHelper::LoadFileToArray(Data, *Path)) {
		FSlateNotificationManager::Get().AddNotification(FNotificationInfo(FText::FromString("Failed to load scene file: " + Path)));
	}

	TArray<UObject*> ObjectsToSave;
	FMemoryReader Ar(Data);
	Ar.ArIsSaveGame = true;
	SerializeScene(ObjectsToSave, Ar);

	this->SceneName = name;

	subsys->OpenEditor(this);
}
