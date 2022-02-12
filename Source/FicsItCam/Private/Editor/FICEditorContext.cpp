#include "Editor/FICEditorContext.h"

#include "FGInputLibrary.h"
#include "StereoRenderTargetManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/GameEngine.h"
#include "Slate/SceneViewport.h"

void UFICEditorContext::ShowEditor() {
	HideEditor();

	IsEditorShowing = true;
	OriginalCharacter = GetWorld()->GetFirstPlayerController()->GetCharacter();
	if (!CameraCharacter) CameraCharacter = GetWorld()->SpawnActor<AFICEditorCameraCharacter>(FVector(PosX.GetValue(), PosY.GetValue(), PosZ.GetValue()), FRotator(RotPitch.GetValue(), RotYaw.GetValue(), RotRoll.GetValue()));
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

	IsEditorShown = true;
}

void UFICEditorContext::HideEditor() {
	IsEditorShowing = false;
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

UFICEditorContext::UFICEditorContext() :
	PosX(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->PosX : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues), FColor::Red),
	PosY(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->PosY : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues), FColor::Green),
	PosZ(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->PosZ : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues), FColor::Blue),
	RotPitch(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->RotPitch : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues), FColor::Red),
	RotYaw(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->RotYaw : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues), FColor::Green),
	RotRoll(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->RotRoll : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues), FColor::Blue),
	FOV(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->FOV : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues), FColor::Yellow),
	Aperture(TAttribute<FFICFloatAttribute*>::Create([this]() { return Animation ? &Animation->Aperture : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues), FColor::Orange),
	FocusDistance(TAttribute<FFICFloatAttribute*>::Create([this]() { return Animation ? &Animation->FocusDistance : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues), FColor::Purple),
	Pos({{"X", &PosX }, {"Y", &PosY}, {"Z", &PosZ}}),
	Rot({{"Pitch", &RotPitch}, {"Yaw", &RotYaw}, {"Roll", &RotRoll}}),
	All({{"Pos", &Pos}, {"Rot", &Rot}, {"FOV", &FOV}, {"Aperture", &Aperture}, {"Focus Distance", &FocusDistance}}) {
	PosX.bShowInGraph = true;
	PosY.bShowInGraph = true;
	PosZ.bShowInGraph = true;
}

void UFICEditorContext::Tick(float DeltaTime) {
	if (Animation) {
		AnimPlayerDelta += DeltaTime * AnimPlayerFactor;
		int32 FrameDelta = FMath::Floor(AnimPlayerDelta * Animation->FPS);
		AnimPlayerDelta -= (float)FrameDelta / (float)Animation->FPS;
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

void UFICEditorContext::SetAnimation(AFICAnimation* Anim) {
	Animation = Anim;
	SetCurrentFrame(Animation->AnimationStart);
}

AFICAnimation* UFICEditorContext::GetAnimation() const {
	return Animation;
}

void UFICEditorContext::SetCurrentFrame(int64 inFrame) {
	CurrentFrame = inFrame;

	PosX.UpdateValue(CurrentFrame);
	PosY.UpdateValue(CurrentFrame);
	PosZ.UpdateValue(CurrentFrame);
	RotPitch.UpdateValue(CurrentFrame);
	RotYaw.UpdateValue(CurrentFrame);
	RotRoll.UpdateValue(CurrentFrame);
	FOV.UpdateValue(CurrentFrame);
	Aperture.UpdateValue(CurrentFrame);
	FocusDistance.UpdateValue(CurrentFrame);
	
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

