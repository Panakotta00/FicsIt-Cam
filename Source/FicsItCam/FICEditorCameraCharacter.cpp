#include "FICEditorCameraCharacter.h"

#include "FICCameraCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "UI/FICEditorContext.h"

#pragma optimize("", off)
AFICEditorCameraCharacter::AFICEditorCameraCharacter() {
	PrimaryActorTick.bCanEverTick = true;
	bSimGravityDisabled = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(GetCapsuleComponent());

	SetActorEnableCollision(false);
	
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = true;
}

void AFICEditorCameraCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->MaxFlySpeed = bIsSprinting ? MaxFlySpeed * 10 : MaxFlySpeed;
	GetCharacterMovement()->MaxAcceleration = 1000000;
	
	if (EditorContext) {
		FVector Pos = GetActorLocation();
		FRotator RotNew = GetActorRotation();
		FRotator RotOld = FRotator(EditorContext->RotPitch.GetValue(), EditorContext->RotYaw.GetValue(), EditorContext->RotRoll.GetValue());
		FRotator RotOldN = RotOld;
		while (RotOldN.Pitch < -180.0) RotOldN.Pitch += 360.0;
		while (RotOldN.Pitch > 180.0) RotOldN.Pitch -= 360.0;
		while (RotOldN.Yaw < -180.0) RotOldN.Yaw += 360.0;
		while (RotOldN.Yaw > 180.0) RotOldN.Yaw -= 360.0;
		while (RotOldN.Roll < -180.0) RotOldN.Roll += 360.0;
		while (RotOldN.Roll > 180.0) RotOldN.Roll -= 360.0;
		FRotator RotDiff = RotNew - RotOldN;
		while (RotDiff.Pitch < -180.0) RotDiff.Pitch += 360.0;
		while (RotDiff.Pitch > 180.0) RotDiff.Pitch -= 360.0;
		while (RotDiff.Yaw < -180.0) RotDiff.Yaw += 360.0;
		while (RotDiff.Yaw > 180.0) RotDiff.Yaw -= 360.0;
		while (RotDiff.Roll < -180.0) RotDiff.Roll += 360.0;
		while (RotDiff.Roll > 180.0) RotDiff.Roll -= 360.0;
		RotNew = RotOld + RotDiff;
		
		EditorContext->PosX.SetValue(Pos.X);
		EditorContext->PosY.SetValue(Pos.Y);
		EditorContext->PosZ.SetValue(Pos.Z);
		EditorContext->RotPitch.SetValue(RotNew.Pitch);
		EditorContext->RotYaw.SetValue(RotNew.Yaw);
		EditorContext->RotRoll.SetValue(RotNew.Roll);
	}
}

void AFICEditorCameraCharacter::BeginPlay() {
	Super::BeginPlay();

	Camera->SetActive(true);

	UCharacterMovementComponent* Mov = GetCharacterMovement();
	Mov->GravityScale = 0;
	Mov->bUseSeparateBrakingFriction = true;
	Mov->BrakingFriction = 10;
}

void AFICEditorCameraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UInputSettings *Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	Settings->AddAxisMapping(FInputAxisKeyMapping("FlyUp", EKeys::SpaceBar));
	Settings->AddAxisMapping(FInputAxisKeyMapping("FlyUp", EKeys::LeftAlt, -1));
	Settings->AddAxisMapping(FInputAxisKeyMapping("Roll", EKeys::E));
	Settings->AddAxisMapping(FInputAxisKeyMapping("Roll", EKeys::Q, -1));
	Settings->AddAxisMapping(FInputAxisKeyMapping("Zoom", EKeys::MouseWheelAxis));

	PlayerInputComponent->BindAxis("MoveForward", this, &AFICEditorCameraCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFICEditorCameraCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AFICEditorCameraCharacter::RotateYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AFICEditorCameraCharacter::RotatePitch);
	PlayerInputComponent->BindAxis("Roll", this, &AFICEditorCameraCharacter::RotateRoll);
	
	PlayerInputComponent->BindAxis("FlyUp", this, &AFICEditorCameraCharacter::FlyUp);
	
	PlayerInputComponent->BindAction("ToggleSprint", EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::SprintPressed);
	PlayerInputComponent->BindAction("ToggleSprint", EInputEvent::IE_Released, this, &AFICEditorCameraCharacter::SprintReleased);

	PlayerInputComponent->BindAction("Jump_Drift", EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::JumpPressed);
	PlayerInputComponent->BindAction("Jump_Drift", EInputEvent::IE_Released, this, &AFICEditorCameraCharacter::JumpReleased);

	PlayerInputComponent->BindAxis("Zoom", this, &AFICEditorCameraCharacter::Zoom);

	PlayerInputComponent->BindKey(EKeys::M, EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::NextKeyframe);
	PlayerInputComponent->BindKey(EKeys::N, EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::PrevKeyframe);
	PlayerInputComponent->BindKey(EKeys::Left, EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::PrevFrame);
	PlayerInputComponent->BindKey(EKeys::Left, EInputEvent::IE_Repeat, this, &AFICEditorCameraCharacter::PrevFrame);
	PlayerInputComponent->BindKey(EKeys::Right, EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::NextFrame);
	PlayerInputComponent->BindKey(EKeys::Right, EInputEvent::IE_Repeat, this, &AFICEditorCameraCharacter::NextFrame);
	PlayerInputComponent->BindKey(EKeys::I, EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::ChangedKeyframe);
}

void AFICEditorCameraCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
}

void AFICEditorCameraCharacter::UnPossessed() {
	Super::UnPossessed();
}

void AFICEditorCameraCharacter::MoveForward(float Value) {
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	float Multi = FlyMultiplier;
	if (bIsSprinting) Multi *= 5;
	AddMovementInput(Direction, Value * Multi);
}

void AFICEditorCameraCharacter::MoveRight(float Value) {
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	float Multi = FlyMultiplier;
	if (bIsSprinting) Multi *= 5;
	AddMovementInput(Direction, Value * Multi);
}
void AFICEditorCameraCharacter::RotatePitch(float Value) {
	/*FRotator ActorRot = GetActorRotation();
	FVector NewRot = ActorRot.RotateVector(FRotator(Value, 0, 0).Vector());
	FRotator Rot = FQuat::FindBetween(ActorRot.Vector(), NewRot).Rotator();
	AddControllerPitchInput(Rot.Pitch);
	AddControllerYawInput(Rot.Yaw);
	AddControllerRollInput(Rot.Roll);*/
	AddControllerPitchInput(Value);
}

void AFICEditorCameraCharacter::RotateYaw(float Value) {
	/*FRotator ActorRot = GetActorRotation();
	FVector NewRot = ActorRot.RotateVector(FRotator(0, Value, 0).Vector());
	FRotator Rot = FQuat::FindBetween(ActorRot.Vector(), NewRot).Rotator();
	AddControllerPitchInput(Rot.Pitch);
	AddControllerYawInput(Rot.Yaw);
	AddControllerRollInput(Rot.Roll);*/
	AddControllerYawInput(Value);
}

void AFICEditorCameraCharacter::RotateRoll(float Value) {
	/*FRotator ActorRot = GetActorRotation();
	FVector NewRot = ActorRot.RotateVector(FRotator(0, 0, Value).Vector());
	FRotator Rot = FQuat::FindBetween(ActorRot.Vector(), NewRot).Rotator();
	AddControllerPitchInput(Rot.Pitch);
	AddControllerYawInput(Rot.Yaw);
	AddControllerRollInput(Rot.Roll);*/
	AddControllerRollInput(Value);
}

void AFICEditorCameraCharacter::FlyUp(float Value) {
	FVector Direction = GetActorUpVector();
	float Multi = FlyMultiplier;
	if (bIsSprinting) Multi *= 5;
	AddMovementInput(Direction, Value * Multi);
}

void AFICEditorCameraCharacter::SprintPressed() {
	bIsSprinting = true;
}

void AFICEditorCameraCharacter::SprintReleased() {
	bIsSprinting = false;
}

void AFICEditorCameraCharacter::JumpPressed() {
	bIsJumping = true;
}

void AFICEditorCameraCharacter::JumpReleased() {
	bIsJumping = false;
}

void AFICEditorCameraCharacter::NextKeyframe() {
	int64 Time;
	TSharedPtr<FFICKeyframeRef> KF;
	if (EditorContext->All.GetAttribute()->GetNextKeyframe(EditorContext->GetCurrentFrame(), Time, KF)) EditorContext->SetCurrentFrame(Time);
}

void AFICEditorCameraCharacter::PrevKeyframe() {
	int64 Time;
	TSharedPtr<FFICKeyframeRef> KF;
	if (EditorContext->All.GetAttribute()->GetPrevKeyframe(EditorContext->GetCurrentFrame(), Time, KF)) EditorContext->SetCurrentFrame(Time);
}

void AFICEditorCameraCharacter::NextFrame() {
	EditorContext->SetCurrentFrame(EditorContext->GetCurrentFrame()+1);
}

void AFICEditorCameraCharacter::PrevFrame() {
	EditorContext->SetCurrentFrame(EditorContext->GetCurrentFrame()-1);
}

void AFICEditorCameraCharacter::ChangedKeyframe() {
	int64 Time = EditorContext->GetCurrentFrame();
	if (EditorContext->PosX.GetKeyframe(Time) && EditorContext->PosY.GetKeyframe(Time) && EditorContext->PosZ.GetKeyframe(Time) && EditorContext->RotPitch.GetKeyframe(Time) && EditorContext->RotYaw.GetKeyframe(Time) && EditorContext->RotRoll.GetKeyframe(Time) && EditorContext->FOV.GetKeyframe(Time) &&
		!EditorContext->PosX.HasChanged(Time) && !EditorContext->PosY.HasChanged(Time) && !EditorContext->PosZ.HasChanged(Time) && !EditorContext->RotPitch.HasChanged(Time) && !EditorContext->RotYaw.HasChanged(Time) && !EditorContext->RotRoll.HasChanged(Time) && !EditorContext->FOV.HasChanged(Time)) {
		EditorContext->All.RemoveKeyframe(Time);
	} else {
		EditorContext->All.SetKeyframe(Time);
	}
}

void AFICEditorCameraCharacter::Zoom(float Value) {
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

void AFICEditorCameraCharacter::UpdateValues() {
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
#pragma optimize("", on)
