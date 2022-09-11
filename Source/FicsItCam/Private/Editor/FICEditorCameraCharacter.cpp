// ReSharper disable CppUENonExistentInputAction
// ReSharper disable CppUENonExistentInputAxis

#include "Editor/FICEditorCameraCharacter.h"

#include "CineCameraComponent.h"
#include "FGGameUserSettings.h"
#include "FGPlayerController.h"
#include "FICUtils.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Data/Objects/FICCamera.h"
#include "Editor/Data/FICEditorCameraActor.h"
#include "Editor/FICEditorContext.h"
#include "Editor/FICEditorSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "Slate/SceneViewport.h"

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
		bool bUseCinematic = EditorContext->GetScene()->bUseCinematic && EditorContext->GetLockCameraToView();
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
			bReposses = true;
			PController->UnPossess();
			PController->Possess(this);
			UFGInputLibrary::UpdateInputMappings(Cast<APlayerController>(PController));
			UFGGameUserSettings::GetFGGameUserSettings()->ApplySettings(false);
			bReposses = false;
		
			Camera->SetActive(true);

			UpdateValues();
		}
		Cast<ULocalPlayer>(GetNetOwningPlayer())->Size = FVector2D(0.5,0.5);

		Camera->bConstrainAspectRatio = EditorContext->bForceResolution;
		if (Camera->IsA<UCineCameraComponent>()) {
			UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(Camera);
			CineCamera->Filmback.SensorWidth = EditorContext->GetScene()->SensorDimension.X * EditorContext->SensorWidthAdjust;
			CineCamera->Filmback.SensorHeight = EditorContext->GetScene()->SensorDimension.Y * EditorContext->SensorWidthAdjust;
		} else {
			Camera->SetAspectRatio(EditorContext->GetScene()->ResolutionHeight / EditorContext->GetScene()->ResolutionWidth);
		}
		if (EditorContext->GetCamera() && EditorContext->GetLockCameraToView()) Camera->SetFieldOfView(EditorContext->GetCameraEditor()->Get("Lens Settings").Get<TFICEditorAttribute<FFICFloatAttribute>>("FOV").GetValue());
		else Camera->SetFieldOfView(FOV);
		
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCharacterMovement()->MaxFlySpeed = Cast<APlayerController>(GetController())->PlayerInput->IsShiftPressed() ? MaxFlySpeed * 10 : MaxFlySpeed;
		GetCharacterMovement()->MaxAcceleration = 1000000;

		FRotator RotatorFix;
		AController* MyController = GetController();
		if (MyController) {
			RotatorFix = MyController->GetControlRotation();
			RotatorFix.Roll = RollRotationFixValue;
		}
	
		if (EditorContext) {
			UFICCamera* CameraObject = EditorContext->GetActiveCamera();
			if (EditorContext->GetLockCameraToView() && CameraObject) {
				if (LastCameraSceneObject != CameraObject) {
					LastCameraSceneObject = CameraObject;
					UpdateValues();
				}

				FRotator RotOld = FFICAttributeRotation::FromEditorAttribute(EditorContext->GetCameraEditor()->Get<FFICEditorAttributeGroup>("Rotation"));
				FVector PosOld = FFICAttributePosition::FromEditorAttribute(EditorContext->GetCameraEditor()->Get<FFICEditorAttributeGroup>("Position"));
				FVector PosNew = GetActorLocation();
				FRotator RotNew = GetController()->GetControlRotation();

				// Patch Rotation
				RotNew = UFICUtils::AdditiveRotation(RotOld, RotNew);
				RotNew.Roll = RollRotationFixValue;

				if (bWasChangedDirectly) EditorContext->bInAutoKeyframeSet = true;
				EditorContext->CommitAutoKeyframe(this);
				bChangedByMovement = true;
				FFICAttributePosition::ToEditorAttribute(PosNew, EditorContext->GetCameraEditor()->Get<FFICEditorAttributeGroup>("Position"));
				FFICAttributeRotation::ToEditorAttribute(RotNew, EditorContext->GetCameraEditor()->Get<FFICEditorAttributeGroup>("Rotation"));
				bChangedByMovement = false;
				EditorContext->CommitAutoKeyframe(nullptr);
				if (bWasChangedDirectly) EditorContext->bInAutoKeyframeSet = false;
				
				RotOld = RotNew;
			}

			UGameplayStatics::SetGlobalTimeDilation(this, EditorContext->GetScene()->bBulletTime ? 0.00001 : 1);
			CustomTimeDilation = 1.0f/UGameplayStatics::GetGlobalTimeDilation(this);
		}
	
		if (MyController) GetController()->SetControlRotation(RotatorFix);

		bWasChangedDirectly = false;
	}
}

void AFICEditorCameraCharacter::BeginPlay() {
	Super::BeginPlay();

	UCharacterMovementComponent* Mov = GetCharacterMovement();
	Mov->GravityScale = 0;
	Mov->bUseSeparateBrakingFriction = true;
	Mov->BrakingFriction = 1000000;
}

void AFICEditorCameraCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
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

	PlayerInputComponent->BindKey(EKeys::RightMouseButton, EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::RightMousePress);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, EInputEvent::IE_Released, this, &AFICEditorCameraCharacter::RightMouseRelease);
	
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AFICEditorCameraCharacter::OnLeftMouseDown);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AFICEditorCameraCharacter::OnLeftMouseUp);

	TArray<FInputActionKeyMapping>& Mappings = Cast<APlayerController>(GetController())->PlayerInput->ActionMappings;
	TMap<FName, FInputActionKeyMapping>& KeyMappings = AFICEditorSubsystem::GetFICEditorSubsystem(this)->KeyMappings;
	KeyMappings.Empty();
	for (const FInputActionKeyMapping& KeyMapping : Mappings) {
		KeyMappings.Add(KeyMapping.ActionName, KeyMapping);
	}
	Mappings.Empty();
}

void AFICEditorCameraCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);

	if (bReposses) return;
	if (NewController) NewController->DisableInput(Cast<APlayerController>(NewController));
}

void AFICEditorCameraCharacter::UnPossessed() {
	AController* OldController = GetController();

	Super::UnPossessed();
	
	if (bReposses) return;
	
	UGameplayStatics::SetGlobalTimeDilation(this, 1);
	CustomTimeDilation = 1;

	if (OldController) {
		// Make sure if not able to recover, game is interactable
		OldController->EnableInput(Cast<APlayerController>(OldController));

		// Force Close the editor
		AFICEditorSubsystem* SubSys = AFICEditorSubsystem::GetFICEditorSubsystem(this);
		if (SubSys->GetEditorPlayerCharacter() == this) SubSys->CloseEditor();
		// TODO: Try to do recover
	}
}

void AFICEditorCameraCharacter::MoveForward(float Value) {
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	float Multi = FlyMultiplier;
	if (Cast<APlayerController>(GetController())->PlayerInput->IsShiftPressed()) Multi *= 3;
	AddMovementInput(Direction, Value * Multi);
}

void AFICEditorCameraCharacter::MoveRight(float Value) {
	if (!IsControlView()) return;
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	float Multi = FlyMultiplier;
	if (Cast<APlayerController>(GetController())->PlayerInput->IsShiftPressed()) Multi *= 3;
	AddMovementInput(Direction, Value * Multi);
}
void AFICEditorCameraCharacter::RotatePitch(float Value) {
	if (!IsControlView()) return;
	AddControllerPitchInput(Value);
}

void AFICEditorCameraCharacter::RotateYaw(float Value) {
	if (!IsControlView()) return;
	AddControllerYawInput(Value);
}

void AFICEditorCameraCharacter::RotateRoll(float Value) {
	if (!IsControlView()) return;
	AddControllerRollInput(Value);
	RollRotationFixValue += Value;
}

void AFICEditorCameraCharacter::FlyUp(float Value) {
	if (!IsControlView()) return;
	FVector Direction = GetActorUpVector();
	float Multi = FlyMultiplier;
	if (Cast<APlayerController>(GetController())->PlayerInput->IsShiftPressed()) Multi *= 3;
	AddMovementInput(Direction, Value * Multi);
	AddActorLocalRotation(FRotator(0, 0, Value));
}

void AFICEditorCameraCharacter::RightMousePress() {
	SetControlView(true, true);
}

void AFICEditorCameraCharacter::RightMouseRelease() {
	if (bControlViewTemp) {
		SetControlView(false);
	}
}

void AFICEditorCameraCharacter::Zoom(float Value) {
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	bool bIsSprinting = PlayerController->PlayerInput->IsShiftPressed();
	if (bIsSprinting && PlayerController->PlayerInput->IsCtrlPressed()) {
		float Delta = Value;
		if (bIsSprinting) Delta *= 2;
		if (EditorContext->GetLockCameraToView()) {
			TFICEditorAttribute<FFICFloatAttribute>& FOV_Attr = EditorContext->GetCameraEditor()->Get("Lens Settings").Get<TFICEditorAttribute<FFICFloatAttribute>>("FOV");
			EditorContext->CommitAutoKeyframe(this);
			FOV_Attr.SetValue(FOV_Attr.GetValue() + Delta);
			EditorContext->CommitAutoKeyframe(nullptr);
		} else {
			FOV += Delta;
		}
	} else if (PlayerController->PlayerInput->IsCtrlPressed()) {
		float Delta = Value * 100;
		if (bIsSprinting) Delta *= 2;
		//if (Delta) EditorContext->SetFlySpeed(EditorContext->GetFlySpeed() + Delta);
		// TODO: Fly Speed!
	} else {
		float Delta = Value;
		int64 Range = EditorContext->GetScene()->AnimationRange.Length();
		while (Range > 300) {
			Range /= 10;
			Delta *= 10;
		}
		if (bIsSprinting) Delta *= 2;
		if (Delta) EditorContext->SetCurrentFrame(EditorContext->GetCurrentFrame() + Delta);
	}
}

void AFICEditorCameraCharacter::OnLeftMouseDown() {
	AFICEditorSubsystem::GetFICEditorSubsystem(GetWorld())->OnLeftMouseDown();
}

void AFICEditorCameraCharacter::OnLeftMouseUp() {
	AFICEditorSubsystem::GetFICEditorSubsystem(GetWorld())->OnLeftMouseUp();
}

void AFICEditorCameraCharacter::SetEditorContext(UFICEditorContext* InEditorContext) {
	if (OnCurrentFrameChangedHandle.IsValid()) {
		EditorContext->OnCurrentFrameChanged.Remove(OnCurrentFrameChangedHandle);
		OnCurrentFrameChangedHandle = FDelegateHandle();
	}
	EditorContext = InEditorContext;
	if (EditorContext) {
		OnCurrentFrameChangedHandle = EditorContext->OnCurrentFrameChanged.AddLambda([this]() {
			bWasChangedDirectly = true;
		});

		FOV = EditorContext->GetScene()->LastCameraFOV;
	}
}

void AFICEditorCameraCharacter::UpdateValues() {
	if (EditorContext && EditorContext->GetCamera() && !bChangedByMovement) {
		FVector Pos = FFICAttributePosition::FromEditorAttribute(EditorContext->GetCameraEditor()->Get<FFICEditorAttributeGroup>("Position"));
		FRotator Rot = FFICAttributeRotation::FromEditorAttribute(EditorContext->GetCameraEditor()->Get<FFICEditorAttributeGroup>("Rotation"));
		if (EditorContext->GetLockCameraToView()) {
			SetActorLocation(Pos);
			SetActorRotation(Rot);
			if (GetController()) {
				GetController()->SetControlRotation(Rot);
				RollRotationFixValue = Rot.Roll;
				Cast<APlayerController>(GetController())->PlayerCameraManager->UnlockFOV();
			}
			UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(Camera);
			if (CineCamera) {
				CineCamera->Filmback.SensorWidth = EditorContext->GetScene()->SensorDimension.X * EditorContext->SensorWidthAdjust;
				CineCamera->Filmback.SensorHeight = EditorContext->GetScene()->SensorDimension.Y * EditorContext->SensorWidthAdjust;
			}
			Camera->SetFieldOfView(EditorContext->GetCameraEditor()->Get("Lens Settings").Get<TFICEditorAttribute<FFICFloatAttribute>>("FOV").GetValue());
			if (CineCamera) {
				CineCamera->CurrentAperture = EditorContext->GetCameraEditor()->Get("Lens Settings").Get<TFICEditorAttribute<FFICFloatAttribute>>("Aperture").GetValue();
				CineCamera->FocusSettings.ManualFocusDistance = EditorContext->GetCameraEditor()->Get("Lens Settings").Get<TFICEditorAttribute<FFICFloatAttribute>>("Focus Distance").GetValue();
			}
		}
	}
}

void AFICEditorCameraCharacter::SetControlView(bool bInControlView, bool bIsTemporary) {
	bControlViewTemp = bIsTemporary;

	APlayerController* Player = Cast<APlayerController>(Controller);
	if (!Player) return;
	if (bInControlView) {
		if (bInControlView == bControlView) return;
		LastCursorPos = FSlateApplication::Get().GetCursorPos();
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		Player->SetInputMode(InputMode);
		Player->SetShowMouseCursor(false);
	} else {
		if (bControlView) FSlateApplication::Get().SetCursorPos(LastCursorPos);

		UGameViewportClient* GameViewportClient = GetWorld()->GetGameViewport();
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player->Player);
		if (GameViewportClient && LocalPlayer) {
			GameViewportClient->SetMouseLockMode(EMouseLockMode::DoNotLock);
			GameViewportClient->SetIgnoreInput(false);
			GameViewportClient->SetHideCursorDuringCapture(false);
			GameViewportClient->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
			FReply& SlateOperations = LocalPlayer->GetSlateOperations();
			SlateOperations.ReleaseMouseCapture();
			SlateOperations.ReleaseMouseLock();
		}
		Player->SetShowMouseCursor(true);
		FSlateApplication::Get().SetAllUserFocus(AFICEditorSubsystem::GetFICEditorSubsystem(EditorContext)->GetEditorWidget());
	}
	
	bControlView = bInControlView;
}
