#include "Editor/FICEditorContext.h"

#include "FGInputLibrary.h"
#include "StereoRenderTargetManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Data/Objects/FICCamera.h"
#include "Data/Objects/FICSceneObject.h"
#include "Engine/GameEngine.h"
#include "Slate/SceneViewport.h"

void UFICEditorContext::ShowEditor() {
	HideEditor();

	IsEditorShowing = true;
	OriginalCharacter = GetWorld()->GetFirstPlayerController()->GetCharacter();
	if (!CameraCharacter) CameraCharacter = GetWorld()->SpawnActor<AFICEditorCameraCharacter>(GetCamera()->Position.Get(0), GetCamera()->Rotation.Get(0));
	CameraCharacter->SetEditorContext(this);
	GetWorld()->GetFirstPlayerController()->Possess(CameraCharacter);
	UFGInputLibrary::UpdateInputMappings(GetWorld()->GetFirstPlayerController());

	GEngine->GameViewport->GetGameViewportWidget()->SetRenderDirectlyToWindow(false);
	GEngine->GameViewport->GetGameLayerManager()->SetSceneViewport(nullptr);
	Cast<UGameEngine>(GEngine)->CleanupGameViewport();
	Cast<UGameEngine>(GEngine)->CreateGameViewport(GEngine->GameViewport);
	
	GameViewport = FSlateApplication::Get().GetGameViewport();
	GameViewportContainer = StaticCastSharedPtr<SVerticalBox>(GameViewport->GetParentWidget());
	GameOverlay = StaticCastSharedPtr<SOverlay>(GameViewportContainer->GetParentWidget());
	//EWindowMode::Type WindowMode = GetWorld()->GetGameViewport()->ViewportFrame->GetViewport()->GetWindowMode();
	//GetWorld()->GetGameViewport()->ViewportFrame->ResizeFrame(1920, 1080, WindowMode);

	check(GameOverlay->RemoveSlot(GameViewportContainer.ToSharedRef()) == true);
	
	EditorWidget = SNew(SFICEditor)
        .Context(this)
        .GameWidget(GameViewportContainer)
		.Viewport(GameViewport);
	
	GameOverlay->AddSlot()[
		EditorWidget.ToSharedRef()
	];

	for (UObject* SceneObject : Scene->GetSceneObjects()) {
		Cast<IFICSceneObject>(SceneObject)->InitEditor(this);
	}

	IsEditorShown = true;
}

void UFICEditorContext::HideEditor() {
	IsEditorShowing = false;

	if (Scene) for (UObject* Object : Scene->GetSceneObjects()) {
		Cast<IFICSceneObject>(Object)->UnloadEditor(this);
	}
	
	if (CameraCharacter) {
		GetWorld()->GetFirstPlayerController()->Possess(OriginalCharacter);
		UFGInputLibrary::UpdateInputMappings(GetWorld()->GetFirstPlayerController());
		CameraCharacter->Destroy();
	}
	if (EditorWidget) {
		GameOverlay->RemoveSlot(EditorWidget.ToSharedRef());
		GameOverlay->AddSlot()[
			GameViewportContainer.ToSharedRef()
		];
		GEngine->GameViewport->GetGameViewportWidget()->SetRenderDirectlyToWindow(true);
		GEngine->GameViewport->GetGameLayerManager()->SetSceneViewport(nullptr);
		Cast<UGameEngine>(GEngine)->CleanupGameViewport();
		Cast<UGameEngine>(GEngine)->CreateGameViewport(GEngine->GameViewport);
		EditorWidget = nullptr;
		APlayerController* Controller = GetWorld()->GetFirstPlayerController();
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(Controller);
	}
	UGameplayStatics::SetGamePaused(this, false);
	IsEditorShown = false;
}

void UFICEditorContext::SetAnimPlayer(EFICAnimPlayerState InAnimPlayerState, float InAnimPlayerFactor) {
	AnimPlayerFactor = InAnimPlayerFactor;
	if (AnimPlayerState != InAnimPlayerState) {
		AnimPlayerState = InAnimPlayerState;
		AnimPlayerDelta = 0;
	}
}

UFICEditorContext::UFICEditorContext() {}

void UFICEditorContext::Tick(float DeltaTime) {
	if (Scene) {
		AnimPlayerDelta += DeltaTime * AnimPlayerFactor;
		int32 FrameDelta = FMath::Floor(AnimPlayerDelta * Scene->FPS);
		AnimPlayerDelta -= (float)FrameDelta / (float)Scene->FPS;
		switch (AnimPlayerState) {
		case FIC_PLAY_FORWARDS:
			SetCurrentFrame(CurrentFrame + FrameDelta);
			break;
		case FIC_PLAY_BACKWARDS:
			SetCurrentFrame(CurrentFrame - FrameDelta);
			break;
		default: ;
		}
	}
}

bool UFICEditorContext::IsTickable() const {
	return !WITH_EDITOR;
}

void UFICEditorContext::LoadSceneObject(UObject* SceneObject) {
	TSharedRef<FFICEditorAttributeBase> Attribute = Cast<IFICSceneObject>(SceneObject)->GetRootAttribute().CreateEditorAttribute();
	AllAttributes->AddAttribute(FString::FromInt(SceneObject->GetUniqueID()), Attribute);
	EditorAttributes.Add(SceneObject, Attribute);
	Attribute->OnValueChanged.AddLambda([this, Attribute, SceneObject]() {
		Cast<IFICSceneObject>(SceneObject)->EditorUpdate(this, Attribute);
	});
	DataAttributeOnUpdateDelegateHandles.Add(SceneObject, Attribute->GetAttribute().OnUpdate.AddLambda([this, Attribute]() {
		Attribute->UpdateValue(GetCurrentFrame());
	}));
}

void UFICEditorContext::UnloadSceneObject(UObject* SceneObject) {
	AllAttributes->RemoveAttribute(FString::FromInt(SceneObject->GetUniqueID()));
	EditorAttributes[SceneObject]->GetAttribute().OnUpdate.Remove(DataAttributeOnUpdateDelegateHandles[SceneObject]);
	EditorAttributes.Remove(SceneObject);
	DataAttributeOnUpdateDelegateHandles.Remove(SceneObject);
}

void UFICEditorContext::AddSceneObject(UObject* SceneObject) {
	Scene->AddSceneObject(SceneObject);
	LoadSceneObject(SceneObject);
	Cast<IFICSceneObject>(SceneObject)->InitEditor(this);
	OnSceneObjectsChanged.Broadcast();
}

void UFICEditorContext::RemoveSceneObject(UObject* SceneObject) {
	
	//Cast<IFICSceneObject>(SceneObject)->UnloadEditor();
	//OnSceneObjectsChanged.Broadcast();
}

void UFICEditorContext::SetScene(AFICScene* InScene) {
	Scene = InScene;
	SetCurrentFrame(Scene->AnimationRange.Begin);
	AllAttributes = MakeShared<FFICEditorAttributeGroupDynamic>();
	for (UObject* SceneObject : Scene->GetSceneObjects()) {
		LoadSceneObject(SceneObject);
	}
}

AFICScene* UFICEditorContext::GetScene() const {
	return Scene;
}

UFICCamera* UFICEditorContext::GetCamera() {
	for (UObject* Object : Scene->GetSceneObjects()) {
		if (Object->IsA<UFICCamera>()) return Cast<UFICCamera>(Object);
	}
	return nullptr;
}

TSharedPtr<FFICEditorAttributeBase> UFICEditorContext::GetCameraEditor() {
	for (TTuple<UObject*, TSharedPtr<FFICEditorAttributeBase>> Attribute : EditorAttributes) {
		if (Attribute.Key->IsA<UFICCamera>()) return Attribute.Value;
	}
	return nullptr;
}

void UFICEditorContext::SetCurrentFrame(FICFrame inFrame) {
	CurrentFrame = inFrame;

	for (TTuple<UObject*, TSharedPtr<FFICEditorAttributeBase>> Attribute : EditorAttributes) {
		Attribute.Value->UpdateValue(inFrame);
	}
	
	UpdateCharacterValues();
}

int64 UFICEditorContext::GetCurrentFrame() const {
	return CurrentFrame;
}

void UFICEditorContext::SetActiveRange(const FFICFrameRange& InActiveRange) {
	ActiveRange = InActiveRange;
}

FFICFrameRange UFICEditorContext::GetActiveRange() {
	return ActiveRange;
}

void UFICEditorContext::SetFlySpeed(float Speed) {
	CameraCharacter->MaxFlySpeed = FMath::Clamp(Speed, 0.0f, 10000.0f);
}

float UFICEditorContext::GetFlySpeed() {
	return CameraCharacter->MaxFlySpeed;
}

void UFICEditorContext::UpdateCharacterValues() {
	if (CameraCharacter) {
		CameraCharacter->UpdateValues();
	}
}

