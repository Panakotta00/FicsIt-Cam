#include "FICEditorCameraCharacter.h"

#include "CineCameraComponent.h"
#include "FGPlayerController.h"
#include "FICCameraCharacter.h"
#include "FICEditorCameraActor.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "Slate/SceneViewport.h"
#include "UI/FICEditorContext.h"

AFICEditorCameraCharacter::AFICEditorCameraCharacter() {
	PrimaryActorTick.bCanEverTick = true;
	bSimGravityDisabled = true;

	SetActorEnableCollision(false);

	bBlockInput = true;
	
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = true;
	
}

void AFICEditorCameraCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (GetController() == GetWorld()->GetFirstPlayerController()) {
		bool bUseCinematic = EditorContext->GetAnimation()->bUseCinematic;
		if (!IsValid(Camera) || Camera->IsA<UCineCameraComponent>() != bUseCinematic) {
			if (Camera) Camera->DestroyComponent();
			if (bUseCinematic) {
				UCineCameraComponent* CineCamera = NewObject<UCineCameraComponent>(this);
				CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::Manual;
				Camera = CineCamera;
			} else {
				Camera = NewObject<UCameraComponent>(this);
			}
		
			Camera->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);

			AController* PController = Controller;
			PController->UnPossess();
			PController->Possess(this);
		
			Camera->SetActive(true);

			UpdateValues();
		}
		Cast<ULocalPlayer>(GetNetOwningPlayer())->Size = FVector2D(0.5,0.5);

		Camera->bConstrainAspectRatio = EditorContext->bForceResolution;
		if (Camera->IsA<UCineCameraComponent>()) {
			UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(Camera);
			CineCamera->Filmback.SensorWidth = EditorContext->GetAnimation()->SensorWidth * EditorContext->SensorWidthAdjust;
			CineCamera->Filmback.SensorHeight = EditorContext->GetAnimation()->SensorHeight * EditorContext->SensorWidthAdjust;
			Camera->SetFieldOfView(EditorContext->FOV.GetValue());
		} else {
			Camera->SetAspectRatio(EditorContext->GetAnimation()->ResolutionHeight / EditorContext->GetAnimation()->ResolutionWidth);
		}
	
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCharacterMovement()->MaxFlySpeed = bIsSprinting ? MaxFlySpeed * 10 : MaxFlySpeed;
		GetCharacterMovement()->MaxAcceleration = 1000000;

		FRotator RotatorFix;
		AController* MyController = GetController();
		if (MyController) {
			RotatorFix = MyController->GetControlRotation();
			RotatorFix.Roll = RollRotationFixValue;
		}
	
		if (EditorContext) {
			if (EditorContext->bMoveCamera) {
				CameraActor->SetActorTransform(GetActorTransform());
			}

			if (CameraActor) {
				FVector Pos = CameraActor->GetActorLocation();
				FRotator RotNew = CameraActor->GetActorRotation();
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

			if (EditorContext->bMoveCamera) RotatorFix.Roll = EditorContext->RotRoll.GetValue();

			// Draw Path
			if (EditorContext->bShowPath) {
				FVector PrevLoc = FVector::ZeroVector;
				FRotator PrevRot = FRotator::ZeroRotator;
				for (int64 Time = EditorContext->GetAnimation()->AnimationStart; Time <= EditorContext->GetAnimation()->AnimationEnd; ++Time) {
					bool bIsKeyframe = EditorContext->Pos.GetKeyframe(Time).IsValid();
					FVector Loc = FVector(EditorContext->PosX.GetValue(Time), EditorContext->PosY.GetValue(Time), EditorContext->PosZ.GetValue(Time));
					if (bIsKeyframe || Loc != PrevLoc) GetWorld()->LineBatcher->DrawLine(Loc, Loc, bIsKeyframe ? FColor::Yellow : FColor::Blue, SDPG_World, 20);
					if (PrevLoc != FVector::ZeroVector) {
						GetWorld()->LineBatcher->DrawLine(PrevLoc, Loc, FColor::Red, SDPG_World, 5);
					}
					PrevLoc = Loc;
				}
			}
		
			UGameplayStatics::SetGlobalTimeDilation(this, EditorContext->GetAnimation()->bBulletTime ? 0.00001 : 1);
			CustomTimeDilation = 1.0f/UGameplayStatics::GetGlobalTimeDilation(this);
		}
	
		if (MyController) GetController()->SetControlRotation(RotatorFix);
	}
}

void AFICEditorCameraCharacter::BeginPlay() {
	Super::BeginPlay();

	UCharacterMovementComponent* Mov = GetCharacterMovement();
	Mov->GravityScale = 0;
	Mov->bUseSeparateBrakingFriction = true;
	Mov->BrakingFriction = 10;

	CameraActor = GetWorld()->SpawnActor<AFICEditorCameraActor>();
	CameraActor->SetActorTransform(GetActorTransform());
	CameraActor->EditorContext = EditorContext;
}

void AFICEditorCameraCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	if (CameraActor) CameraActor->Destroy();
}

void AFICEditorCameraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	Settings->AddAxisMapping(FInputAxisKeyMapping("FicsItCam.Zoom", EKeys::MouseWheelAxis));
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AFICEditorCameraCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFICEditorCameraCharacter::MoveRight);
	PlayerInputComponent->BindAxis("FicsItCam.MoveUp", this, &AFICEditorCameraCharacter::FlyUp);
	PlayerInputComponent->BindAxis("FicsItCam.MoveRoll", this, &AFICEditorCameraCharacter::RotateRoll);
	PlayerInputComponent->BindAxis("FicsItCam.Zoom", this, &AFICEditorCameraCharacter::Zoom);

	PlayerInputComponent->BindAxis("Turn", this, &AFICEditorCameraCharacter::RotateYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AFICEditorCameraCharacter::RotatePitch);

	PlayerInputComponent->BindAction("FicsItCam.ChangeFOV", IE_Pressed, this, &AFICEditorCameraCharacter::EnterChangeFOV);
	PlayerInputComponent->BindAction("FicsItCam.ChangeFOV", IE_Released, this, &AFICEditorCameraCharacter::LeaveChangeFOV);
	PlayerInputComponent->BindAction("FicsItCam.Sprint", EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::EnterSprint);
	PlayerInputComponent->BindAction("FicsItCam.Sprint", EInputEvent::IE_Released, this, &AFICEditorCameraCharacter::LeaveSprint);
	PlayerInputComponent->BindAction("FicsItCam.ChangeSpeed", EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::EnterChangeSpeed);
	PlayerInputComponent->BindAction("FicsItCam.ChangeSpeed", EInputEvent::IE_Released, this, &AFICEditorCameraCharacter::LeaveChangeSpeed);

	PlayerInputComponent->BindAction(TEXT("FicsItCam.NextKeyframe"), EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::NextKeyframe);
	PlayerInputComponent->BindAction(TEXT("FicsItCam.PrevKeyframe"), EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::PrevKeyframe);
	PlayerInputComponent->BindAction(TEXT("FicsItCam.PrevFrame"), EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::PrevFrame);
	PlayerInputComponent->BindAction(TEXT("FicsItCam.PrevFrame"), EInputEvent::IE_Repeat, this, &AFICEditorCameraCharacter::PrevFrame);
	PlayerInputComponent->BindAction(TEXT("FicsItCam.NextFrame"), EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::NextFrame);
	PlayerInputComponent->BindAction(TEXT("FicsItCam.NextFrame"), EInputEvent::IE_Repeat, this, &AFICEditorCameraCharacter::NextFrame);
	PlayerInputComponent->BindAction(TEXT("FicsItCam.ToggleAllKeyframes"), EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::ChangedKeyframe);
	PlayerInputComponent->BindAction(TEXT("FicsItCam.ToggleAutoKeyframe"), EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::ToggleAutoKeyframe);
	PlayerInputComponent->BindAction(TEXT("FicsItCam.ToggleShowPath"), EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::ToggleShowPath);
	PlayerInputComponent->BindAction(TEXT("FicsItCam.ToggleLockCamera"), EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::ToggleLockCamera);
}

void AFICEditorCameraCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	if (NewController && (EditorContext->IsEditorShown || EditorContext->IsEditorShowing)) NewController->DisableInput(Cast<APlayerController>(NewController));
}

void AFICEditorCameraCharacter::UnPossessed() {
	AController* OldController = Controller;
	Super::UnPossessed();
	UGameplayStatics::SetGlobalTimeDilation(this, 1);
	CustomTimeDilation = 1;
	if (OldController) OldController->EnableInput(Cast<APlayerController>(OldController));
	if (OldController && EditorContext && EditorContext->IsEditorShown && !EditorContext->IsEditorShowing) {
		EditorContext->HideEditor();
	}
}

void AFICEditorCameraCharacter::MoveForward(float Value) {
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	float Multi = FlyMultiplier;
	if (bIsSprinting) Multi *= 3;
	AddMovementInput(Direction, Value * Multi);
}

void AFICEditorCameraCharacter::MoveRight(float Value) {
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	float Multi = FlyMultiplier;
	if (bIsSprinting) Multi *= 3;
	AddMovementInput(Direction, Value * Multi);
}
void AFICEditorCameraCharacter::RotatePitch(float Value) {
	AddControllerPitchInput(Value);
}

void AFICEditorCameraCharacter::RotateYaw(float Value) {
	AddControllerYawInput(Value);
}

void AFICEditorCameraCharacter::RotateRoll(float Value) {
	AddControllerRollInput(Value);
	RollRotationFixValue += Value;
}

void AFICEditorCameraCharacter::FlyUp(float Value) {
	FVector Direction = GetActorUpVector();
	float Multi = FlyMultiplier;
	if (bIsSprinting) Multi *= 3;
	AddMovementInput(Direction, Value * Multi);
	AddActorLocalRotation(FRotator(0, 0, Value));
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
	int64 Rate = 1;
	if (bIsSprinting) Rate = 10;
	EditorContext->SetCurrentFrame(EditorContext->GetCurrentFrame()+Rate);
}

void AFICEditorCameraCharacter::PrevFrame() {
	int64 Rate = 1;
	if (bIsSprinting) Rate = 10;
	EditorContext->SetCurrentFrame(EditorContext->GetCurrentFrame()-Rate);
}

void AFICEditorCameraCharacter::ToggleAutoKeyframe() {
	EditorContext->bAutoKeyframe = !EditorContext->bAutoKeyframe;
}

void AFICEditorCameraCharacter::ToggleShowPath() {
	EditorContext->bShowPath = !EditorContext->bShowPath;
}

void AFICEditorCameraCharacter::ToggleLockCamera() {
	EditorContext->bMoveCamera = !EditorContext->bMoveCamera;
}

void AFICEditorCameraCharacter::SetEditorContext(UFICEditorContext* InEditorContext) {
	EditorContext = InEditorContext;
	if (CameraActor) CameraActor->EditorContext = EditorContext;
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
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (bChangeFOV) {
		float Delta = Value;
		if (bIsSprinting) Delta *= 2;
		EditorContext->FOV.SetValue(EditorContext->FOV.GetValue() + Delta);
	} else if (bChangeSpeed) {
		float Delta = Value * 100;
		if (bIsSprinting) Delta *= 2;
		if (Delta) EditorContext->SetFlySpeed(EditorContext->GetFlySpeed() + Delta);
	} else {
		float Delta = Value;
		int64 Range = EditorContext->GetAnimation()->AnimationEnd - EditorContext->GetAnimation()->AnimationStart;
		while (Range > 300) {
			Range /= 10;
			Delta *= 10;
		}
		if (bIsSprinting) Delta *= 2;
		if (Delta) EditorContext->SetCurrentFrame(EditorContext->GetCurrentFrame() + Delta);
	}
}

void AFICEditorCameraCharacter::UpdateValues() {
	if (EditorContext) {
		FVector Pos = FVector(EditorContext->PosX.GetValue(), EditorContext->PosY.GetValue(), EditorContext->PosZ.GetValue());
		FRotator Rot = FRotator(EditorContext->RotPitch.GetValue(), EditorContext->RotYaw.GetValue(), EditorContext->RotRoll.GetValue());
		if (EditorContext->bMoveCamera) {
			SetActorLocation(Pos);
			SetActorRotation(Rot);
			if (GetController()) {
				GetController()->SetControlRotation(Rot);
				Cast<APlayerController>(GetController())->PlayerCameraManager->UnlockFOV();
			}
			UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(Camera);
			if (CineCamera) {
				CineCamera->Filmback.SensorWidth = EditorContext->GetAnimation()->SensorWidth * EditorContext->SensorWidthAdjust;
				CineCamera->Filmback.SensorHeight = EditorContext->GetAnimation()->SensorHeight * EditorContext->SensorWidthAdjust;
			}
			Camera->SetFieldOfView(EditorContext->FOV.GetValue());
			if (CineCamera) {
				CineCamera->CurrentAperture = EditorContext->Aperture.GetValue();
				CineCamera->FocusSettings.ManualFocusDistance = EditorContext->FocusDistance.GetValue();
			}
		}
		CameraActor->SetActorTransform(FTransform(Rot, Pos));
		CameraActor->UpdateGizmo();
	}
}
