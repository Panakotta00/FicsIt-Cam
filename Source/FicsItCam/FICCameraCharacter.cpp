#include "FICCameraCharacter.h"

#include "Engine/World.h"
#include "FGPlayerController.h"
#include "util/Logging.h"

void AFICCameraCharacter::OnTickWorldStreamTimer() {
	UWorld* world = GetWorld();
	world->UpdateLevelStreamingState(); 
	if(world->IsLevelStreamingRequestPending(world->GetFirstPlayerController())) return;
	AFGCharacterPlayer* Char = Cast<AFGCharacterPlayer>(OriginalCharacter);
	if (Char) Char->CheatToggleGhostFly(false);
	GetWorld()->GetTimerManager().ClearTimer(WorldStreamTimer);
}

AFICCameraCharacter::AFICCameraCharacter() {
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(GetCapsuleComponent());
	
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

	Camera->SetActive(true);
}

void AFICCameraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	PlayerInputComponent->BindKey(EKeys::C, EInputEvent::IE_Pressed, this, &AFICCameraCharacter::StopAnimation);
}

void AFICCameraCharacter::PossessedBy(AController* NewController) {}
void AFICCameraCharacter::UnPossessed() {}

void AFICCameraCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	
	if (Animation) {
		//double Start = FPlatformTime::Seconds();
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
		//SML::Logging::error((FPlatformTime::Seconds() - Start) * 1000);
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
	OriginalCharacter->SetActorHiddenInGame(true);
	Cast<AFGHUD>(Controller->GetHUD())->SetHUDVisibility(false);
	Cast<AFGHUD>(Controller->GetHUD())->SetShowCrossHair(false);
	Controller->PlayerCameraManager->UnlockFOV();
}

void AFICCameraCharacter::StopAnimation() {
	if (Animation) {
		AFGPlayerController* Controller = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
		Controller->Possess(OriginalCharacter);
		OriginalCharacter->SetActorHiddenInGame(false);
		Cast<AFGHUD>(Controller->GetHUD())->SetHUDVisibility(true);
		Cast<AFGHUD>(Controller->GetHUD())->SetShowCrossHair(true);
		Animation = nullptr;
		Progress = 0.0f;
		Cast<AFGCharacterPlayer>(OriginalCharacter)->CheatToggleGhostFly(true);
		GetWorld()->GetTimerManager().SetTimer(WorldStreamTimer, this, &AFICCameraCharacter::OnTickWorldStreamTimer, 0.1f, true);
	}
}
