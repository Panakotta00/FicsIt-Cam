#include "FICEditorContext.h"

#include "StereoRenderTargetManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/GameEngine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Slate/SceneViewport.h"
#include "FicsItCam/FicsItCamModule.h"

void UFICEditorContext::ShowEditor() {
	HideEditor();

	OriginalCharacter = GetWorld()->GetFirstPlayerController()->GetCharacter();
	if (!CameraCharacter) CameraCharacter = GetWorld()->SpawnActor<AFICEditorCameraCharacter>(FVector(PosX.GetValue(), PosY.GetValue(), PosZ.GetValue()), FRotator(RotPitch.GetValue(), RotYaw.GetValue(), RotRoll.GetValue()));
	CameraCharacter->SetEditorContext(this);
	GetWorld()->GetFirstPlayerController()->Possess(CameraCharacter);

	GEngine->GameViewport->GetGameViewportWidget()->SetRenderDirectlyToWindow(false);
	GEngine->GameViewport->GetGameLayerManager()->SetSceneViewport(nullptr);
	Cast<UGameEngine>(GEngine)->CleanupGameViewport();
	Cast<UGameEngine>(GEngine)->CreateGameViewport(GEngine->GameViewport);
	
	GameViewport = FSlateApplication::Get().GetGameViewport();
	GameViewportContainer = StaticCastSharedPtr<SVerticalBox>(GameViewport->GetParentWidget());
	GameOverlay = StaticCastSharedPtr<SOverlay>(GameViewportContainer->GetParentWidget());

	check(GameOverlay->RemoveSlot(GameViewportContainer.ToSharedRef()) == true);
	
	EditorWidget = SNew(SFICEditor)
        .Context(this)
        .GameWidget(GameViewportContainer);
	
	GameOverlay->AddSlot()[
		EditorWidget.ToSharedRef()
	];
}

void UFICEditorContext::HideEditor() {
	if (CameraCharacter) {
		GetWorld()->GetFirstPlayerController()->Possess(OriginalCharacter);
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
}

UFICEditorContext::UFICEditorContext() :
	PosX(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->PosX : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues)),
	PosY(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->PosY : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues)),
	PosZ(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->PosZ : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues)),
	RotPitch(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->RotPitch : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues)),
	RotYaw(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->RotYaw : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues)),
	RotRoll(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->RotRoll : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues)),
	FOV(TAttribute<FFICFloatAttribute*>::Create([this](){ return Animation ? &Animation->FOV : nullptr; }), FFICAttributeValueChanged::CreateUObject(this, &UFICEditorContext::UpdateCharacterValues)),
	All({{"X", &PosX }, {"Y", &PosY}, {"Z", &PosZ}, {"Pitch", &RotPitch}, {"Yaw", &RotYaw}, {"Roll", &RotRoll}, {"FOV", &FOV}}),
	Pos({{"X", &PosX }, {"Y", &PosY}, {"Z", &PosZ}}) {}

void UFICEditorContext::SetAnimation(AFICAnimation* Anim) {
	Animation = Anim;
	SetCurrentFrame(Animation->AnimationStart);
}

AFICAnimation* UFICEditorContext::GetAnimation() const {
	return Animation;
}

void UFICEditorContext::SetCurrentFrame(int64 inFrame) {
	CurrentFrame = inFrame;

	PosX.SetFrame(CurrentFrame);
	PosY.SetFrame(CurrentFrame);
	PosZ.SetFrame(CurrentFrame);
	RotPitch.SetFrame(CurrentFrame);
	RotYaw.SetFrame(CurrentFrame);
	RotRoll.SetFrame(CurrentFrame);
	FOV.SetFrame(CurrentFrame);
	
	UpdateCharacterValues();
}

int64 UFICEditorContext::GetCurrentFrame() const {
	return CurrentFrame;
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

