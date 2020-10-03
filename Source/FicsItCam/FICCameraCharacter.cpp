#include "FICCameraCharacter.h"


#include "FICCamera.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "UI/FICEditorContext.h"

AFICCameraCharacter::AFICCameraCharacter() {
	PrimaryActorTick.bCanEverTick = true;
	bSimGravityDisabled = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(GetCapsuleComponent());

	SetActorEnableCollision(false);
	
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = true;
}

void AFICCameraCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->MaxFlySpeed = bIsSprinting ? MaxFlySpeed * 10 : MaxFlySpeed;
	GetCharacterMovement()->MaxAcceleration = 1000000;
	
	if (EditorContext) {
		FVector Pos = GetActorLocation();
		FRotator RotNew = GetActorRotation();
		FRotator RotOld = FRotator(EditorContext->RotPitch.GetValue(), EditorContext->RotYaw.GetValue(), EditorContext->RotRoll.GetValue());
		while (RotOld.Pitch < -180.0) RotOld.Pitch += 360.0;
		while (RotOld.Pitch > 180.0) RotOld.Pitch -= 360.0;
		while (RotOld.Yaw < -180.0) RotOld.Yaw += 360.0;
		while (RotOld.Yaw > 180.0) RotOld.Yaw -= 360.0;
		while (RotOld.Roll < -180.0) RotOld.Roll += 360.0;
		while (RotOld.Roll > 180.0) RotOld.Roll -= 360.0;
		//RotOld.Roll = 0;
		//RotNew.Roll = 0;
		//FRotator RotDiff = FQuat::FindBetween(RotNew.Vector(), RotOld.Vector()).Rotator();
		FRotator RotDiff = RotNew - RotOld;
		//RotDiff.Roll = 0;
		RotNew = RotOld + RotDiff;
		
		EditorContext->PosX.SetValue(Pos.X);
		EditorContext->PosY.SetValue(Pos.Y);
		EditorContext->PosZ.SetValue(Pos.Z);
		EditorContext->RotPitch.SetValue(RotNew.Pitch);
		EditorContext->RotYaw.SetValue(RotNew.Yaw);
		EditorContext->RotRoll.SetValue(RotNew.Roll);
	}
}

void AFICCameraCharacter::BeginPlay() {
	Super::BeginPlay();

	Camera->SetActive(true);

	UCharacterMovementComponent* Mov = GetCharacterMovement();
	Mov->GravityScale = 0;
	Mov->bUseSeparateBrakingFriction = true;
	Mov->BrakingFriction = 10;
}

void AFICCameraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UInputSettings *Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	Settings->AddAxisMapping(FInputAxisKeyMapping("FlyUp", EKeys::SpaceBar));
	Settings->AddAxisMapping(FInputAxisKeyMapping("FlyUp", EKeys::LeftAlt, -1));
	Settings->AddAxisMapping(FInputAxisKeyMapping("Roll", EKeys::E));
	Settings->AddAxisMapping(FInputAxisKeyMapping("Roll", EKeys::Q, -1));
	Settings->AddAxisMapping(FInputAxisKeyMapping("Zoom", EKeys::MouseWheelAxis));

	PlayerInputComponent->BindAxis("MoveForward", this, &AFICCameraCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFICCameraCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AFICCameraCharacter::RotateYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AFICCameraCharacter::RotatePitch);
	PlayerInputComponent->BindAxis("Roll", this, &AFICCameraCharacter::RotateRoll);
	
	PlayerInputComponent->BindAxis("FlyUp", this, &AFICCameraCharacter::FlyUp);
	
	PlayerInputComponent->BindAction("ToggleSprint", EInputEvent::IE_Pressed, this, &AFICCameraCharacter::SprintPressed);
	PlayerInputComponent->BindAction("ToggleSprint", EInputEvent::IE_Released, this, &AFICCameraCharacter::SprintReleased);

	PlayerInputComponent->BindAction("Jump_Drift", EInputEvent::IE_Pressed, this, &AFICCameraCharacter::JumpPressed);
	PlayerInputComponent->BindAction("Jump_Drift", EInputEvent::IE_Released, this, &AFICCameraCharacter::JumpReleased);

	PlayerInputComponent->BindAxis("Zoom", this, &AFICCameraCharacter::Zoom);
}

void AFICCameraCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
}

void AFICCameraCharacter::UnPossessed() {
	Super::UnPossessed();
}

void AFICCameraCharacter::MoveForward(float Value) {
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	float Multi = FlyMultiplier;
	if (bIsSprinting) Multi *= 5;
	AddMovementInput(Direction, Value * Multi);
}

void AFICCameraCharacter::MoveRight(float Value) {
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	float Multi = FlyMultiplier;
	if (bIsSprinting) Multi *= 5;
	AddMovementInput(Direction, Value * Multi);
}
#pragma optimize("", off)
void AFICCameraCharacter::RotatePitch(float Value) {
	/*FRotator ActorRot = GetActorRotation();
	FVector NewRot = ActorRot.RotateVector(FRotator(Value, 0, 0).Vector());
	FRotator Rot = FQuat::FindBetween(ActorRot.Vector(), NewRot).Rotator();
	AddControllerPitchInput(Rot.Pitch);
	AddControllerYawInput(Rot.Yaw);
	AddControllerRollInput(Rot.Roll);*/
	AddControllerPitchInput(Value);
}

void AFICCameraCharacter::RotateYaw(float Value) {
	/*FRotator ActorRot = GetActorRotation();
	FVector NewRot = ActorRot.RotateVector(FRotator(0, Value, 0).Vector());
	FRotator Rot = FQuat::FindBetween(ActorRot.Vector(), NewRot).Rotator();
	AddControllerPitchInput(Rot.Pitch);
	AddControllerYawInput(Rot.Yaw);
	AddControllerRollInput(Rot.Roll);*/
	AddControllerYawInput(Value);
}

void AFICCameraCharacter::RotateRoll(float Value) {
	/*FRotator ActorRot = GetActorRotation();
	FVector NewRot = ActorRot.RotateVector(FRotator(0, 0, Value).Vector());
	FRotator Rot = FQuat::FindBetween(ActorRot.Vector(), NewRot).Rotator();
	AddControllerPitchInput(Rot.Pitch);
	AddControllerYawInput(Rot.Yaw);
	AddControllerRollInput(Rot.Roll);*/
	AddControllerRollInput(Value);
}
#pragma optimize("", on)

void AFICCameraCharacter::FlyUp(float Value) {
	FVector Direction = GetActorUpVector();
	float Multi = FlyMultiplier;
	if (bIsSprinting) Multi *= 5;
	AddMovementInput(Direction, Value * Multi);
}

void AFICCameraCharacter::SprintPressed() {
	bIsSprinting = true;
}

void AFICCameraCharacter::SprintReleased() {
	bIsSprinting = false;
}

void AFICCameraCharacter::JumpPressed() {
	bIsJumping = true;
}

void AFICCameraCharacter::JumpReleased() {
	bIsJumping = false;
}

void AFICCameraCharacter::Zoom(float Value) {
	APlayerController* Controller = Cast<APlayerController>(GetController());
	if (Controller->IsInputKeyDown(EKeys::LeftControl)) {
		float Delta = Value;
		int64 Range = EditorContext->GetAnimation()->AnimationEnd - EditorContext->GetAnimation()->AnimationStart;
		while (Range > 300) {
			Range /= 10;
			Delta *= 10;
		}
		if (Delta) EditorContext->SetCurrentFrame(EditorContext->GetCurrentFrame() + Delta);
	} else if (Controller->IsInputKeyDown(EKeys::LeftShift)) {
		float Delta = Value * 10;
		if (Delta) EditorContext->SetFlySpeed(EditorContext->GetFlySpeed() + Delta);
	}
}

void AFICCameraCharacter::UpdateValues() {
	if (EditorContext) {
		FVector Pos = FVector(EditorContext->PosX.GetValue(), EditorContext->PosY.GetValue(), EditorContext->PosZ.GetValue());
		SetActorLocation(Pos);
		FRotator Rot = FRotator(EditorContext->RotPitch.GetValue(), EditorContext->RotYaw.GetValue(), EditorContext->RotRoll.GetValue());
		GetController()->SetControlRotation(Rot);
		SetActorRotation(Rot);
		Cast<APlayerController>(GetController())->PlayerCameraManager->UnlockFOV();
		Camera->SetFieldOfView(EditorContext->FOV.GetValue());
	}
}
