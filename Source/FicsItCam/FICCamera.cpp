#include "FICCamera.h"

#include "Engine/World.h"
#include "FGPlayerController.h"
#include "util/Logging.h"

AFICCamera::AFICCamera() {
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Camera->SetAutoActivate(true);
	
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFICCamera::BeginPlay() {
	Super::BeginPlay();
}

void AFICCamera::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	
	if (Animation) {
		Progress += DeltaSeconds;
		FVector Pos;
		Pos.X = Animation->PosX.GetValue(Progress);
		Pos.Y = Animation->PosY.GetValue(Progress);
		Pos.Z = Animation->PosZ.GetValue(Progress);
		FRotator Rot;
		Rot.Pitch = Animation->RotPitch.GetValue(Progress);
		Rot.Yaw = Animation->RotYaw.GetValue(Progress);
		Rot.Roll = Animation->RotRoll.GetValue(Progress);
		float FOV = Animation->FOV.GetValue(Progress);
		Camera->SetWorldLocation(Pos);
		Camera->SetWorldRotation(Rot);
		Camera->SetFieldOfView(FOV);

		if (Animation->GetEndOfAnimation() < Progress) {
			StopAnimation();
		}
	}
}

void AFICCamera::StartAnimation(UFICAnimation* inAnimation) {
	StopAnimation();
	Animation = inAnimation;
	if (!Animation) return;
	
	AFGPlayerController* Controller = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
	bPrevCanBeDamaged = Controller->bCanBeDamaged;
	PrevViewTarget = Controller->GetViewTarget();
	CameraManager = Controller->PlayerCameraManager;
	CameraPos = CameraManager->GetCameraLocation();
	CameraRot = CameraManager->GetCameraRotation();
	CameraFOV = CameraManager->GetFOVAngle();
	//Controller->SetViewTarget(this);
	PrevCamera = Cast<UCameraComponent>(Controller->GetCharacter()->GetComponentByClass(UCameraComponent::StaticClass()));
	PrevCamera->SetActive(false);
	Camera = NewObject<UCameraComponent>(Controller->GetCharacter());
	Camera->AttachToComponent(Controller->GetCharacter()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Camera->RegisterComponent();
	Controller->GetCharacter()->SetActorHiddenInGame(true);
	Camera->SetActive(true);
	Cast<AFGCharacterPlayer>(Controller->GetCharacter())->SetCameraMode(ECameraMode::ECM_ThirdPerson);
}

void AFICCamera::StopAnimation() {
	AFGPlayerController* Controller = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
	Controller->GetCharacter()->SetActorHiddenInGame(false);
	if (Camera) {
		Camera->SetActive(false);
		Camera->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		Camera->UnregisterComponent();
		Camera->DestroyComponent();
	}
	if (PrevCamera) PrevCamera->SetActive(true);
	//Controller->SetViewTarget(nullptr);
	//Controller->RerunConstructionScripts();
	//Controller->GetCharacter()->RerunConstructionScripts();
	Cast<AFGCharacterPlayer>(Controller->GetCharacter())->SetCameraMode(ECameraMode::ECM_FirstPerson);
	Animation = nullptr;
	Progress = 0.0f;
}
