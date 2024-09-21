#include "Data/FICScene.h"

#include "FICSubsystem.h"
#include "FICUtils.h"
#include "Editor/FICEditorSubsystem.h"
#include "Engine/GameViewportClient.h"
#include "Slate/SceneViewport.h"

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

void AFICScene::UpdatePreview() {
	if (PreviewTexture == nullptr) {
		PreviewTexture = NewObject<UFICProceduralTexture>(this);
		PreviewTexture->OnTextureUpdate.AddDynamic(this, &AFICScene::OnTextureUpdate);
	}
	
	TSharedRef<FSequenceExporterProceduralTexture> TextureExporter = MakeShared<FSequenceExporterProceduralTexture>(PreviewTexture);
	FTextureRHIRef Target;
	UEngine* Engine = GEngine;
	ENQUEUE_RENDER_COMMAND(UpdateScenePreview)([this, &Target, Engine](FRHICommandListImmediate& RHICmdList){
		FViewportRHIRef Viewport = Engine->GameViewport->GetGameViewport()->GetViewportFrame()->GetViewport()->GetViewportRHI();
		if (Viewport) {
			Target = RHIGetViewportBackBuffer(Viewport);
		}
		FTextureRHIRef Texture = Engine->GameViewport->Viewport->GetRenderTargetTexture();
		if (Texture) {
			Target = Texture;
		}
	});
	FlushRenderingCommands();
	if (Target) {
		AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(TextureExporter, MakeShared<FFICRenderTarget_Raw>(Target), true);
	}
}

bool AFICScene::IsSceneAlreadyInUse() {
	AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(this);
	AFICEditorSubsystem* EditSubSys = AFICEditorSubsystem::GetFICEditorSubsystem(this);
	FString ProcessKey = GetSceneProcessKey(SceneName);
	return SubSys->GetActiveRuntimeProcessesMap().Contains(ProcessKey) || (EditSubSys->GetActiveEditorContext() && EditSubSys->GetActiveEditorContext()->GetScene() == this);
}
