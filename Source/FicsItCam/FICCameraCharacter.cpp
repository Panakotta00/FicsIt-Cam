#include "FICCameraCharacter.h"

#include "Engine/World.h"
#include "FGPlayerController.h"
#include "util/Logging.h"

AFICCameraCharacter::AFICCameraCharacter() {
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Camera->SetAutoActivate(true);
	
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	bSimGravityDisabled = true;
	
	SetActorEnableCollision(false);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AFICCameraCharacter::BeginPlay() {
	Super::BeginPlay();
}

void AFICCameraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	PlayerInputComponent->BindKey(EKeys::Escape, EInputEvent::IE_Pressed, this, &AFICCameraCharacter::StopAnimation);
}

void AFICCameraCharacter::PossessedBy(AController* NewController) {}
void AFICCameraCharacter::UnPossessed() {}

void AFICCameraCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	
	if (Animation) {
		Progress += DeltaSeconds;
		float Time = Progress * Animation->FPS;
		FVector Pos;
		Pos.X = Animation->PosX.GetValue(Time);
		Pos.Y = Animation->PosY.GetValue(Time);
		Pos.Z = Animation->PosZ.GetValue(Time);
		FRotator Rot;
		Rot.Pitch = Animation->RotPitch.GetValue(Time);
		Rot.Yaw = Animation->RotYaw.GetValue(Time);
		Rot.Roll = Animation->RotRoll.GetValue(Time);
		float FOV = Animation->FOV.GetValue(Time);
		SetActorLocation(Pos);
		SetActorRotation(Rot);
		GetController()->SetControlRotation(Rot);
		Camera->SetFieldOfView(FOV);

		if (Animation->GetEndOfAnimation() < Progress) {
			StopAnimation();
		}
	}
}

void AFICCameraCharacter::StartAnimation(AFICAnimation* inAnimation) {
	StopAnimation();
	Animation = inAnimation;
	if (!Animation) return;
	Progress = Animation->GetStartOfAnimation();
	
	AFGPlayerController* Controller = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
	OriginalCharacter = Controller->GetCharacter();
	Controller->Possess(this);
	Cast<AFGHUD>(Controller->GetHUD())->SetHUDVisibility(false);
	Cast<AFGHUD>(Controller->GetHUD())->SetShowCrossHair(false);
}

void AFICCameraCharacter::StopAnimation() {
	AFGPlayerController* Controller = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
	Controller->GetCharacter()->SetActorHiddenInGame(false);
	Controller->Possess(OriginalCharacter);
	Cast<AFGHUD>(Controller->GetHUD())->SetHUDVisibility(true);
	Cast<AFGHUD>(Controller->GetHUD())->SetShowCrossHair(true);
	Animation = nullptr;
	Progress = 0.0f;
}
