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
#include "Editor/FICEditorContext.h"
#include "Editor/FICEditorSubsystem.h"
#include "Editor/Data/FICEditorAttributeBase.h"
#include "Editor/Data/FICEditorAttributeGroupDynamic.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "Input/FGEnhancedInputComponent.h"

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
			//UFGInputLibrary::UpdateInputMappings(Cast<APlayerController>(PController));
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
	
	UFGEnhancedInputComponent* EnhancedInputComponent = Cast<UFGEnhancedInputComponent>(PlayerInputComponent);
	const UFGInputSettings* Settings = UFGInputSettings::Get();

	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.Movement"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::Move);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.Rotation"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::Rotate);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.Redo"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::Redo);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.Undo"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::Undo);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.Frame"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::ChangeFrame);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.NextKeyframe"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::NextKeyframe);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.PrevKeyframe"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::PrevKeyframe);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.ToggleAllKeyframes"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::ToggleAllKeyframes);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.ToggleAutoKeyframe"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::ToggleAutoKeyframe);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.ToggleCursor"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::ToggleCursor);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.ToggleLockCamera"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::ToggleLockCamera);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.ToggleShowPath"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::ToggleShowPath);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.Grab"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::Grab);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.FOV"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::ChangeFOV);
	EnhancedInputComponent->BindAction(Settings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.FIC.Editor.Speed"))), ETriggerEvent::Triggered, this, &AFICEditorCameraCharacter::ChangeSpeed);
	
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, EInputEvent::IE_Pressed, this, &AFICEditorCameraCharacter::RightMousePress);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, EInputEvent::IE_Released, this, &AFICEditorCameraCharacter::RightMouseRelease);
	
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AFICEditorCameraCharacter::OnLeftMouseDown);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AFICEditorCameraCharacter::OnLeftMouseUp);
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

void AFICEditorCameraCharacter::Move(const FInputActionValue& ActionValue) {
	if (!IsControlView()) return;

	float Multi = FlyMultiplier;
	// if (Cast<APlayerController>(GetController())->PlayerInput->IsShiftPressed()) Multi *= 3; TODO: Check if SpeedUp
	AddMovementInput(Controller->GetCharacter()->GetTransform().TransformVector(ActionValue.Get<FVector>()).GetSafeNormal(), ActionValue.GetMagnitude() * Multi);
}

void AFICEditorCameraCharacter::Rotate(const FInputActionValue& ActionValue) {
	if (!IsControlView()) return;

	FVector Value = ActionValue.Get<FVector>();
	AddControllerPitchInput(Value.X);
	AddControllerYawInput(Value.Z);
	AddControllerRollInput(Value.Y);
}

void AFICEditorCameraCharacter::ChangeFOV(const FInputActionValue& ActionValue) {
	float Delta = ActionValue.GetMagnitude();
	// if (bIsSprinting) Delta *= 2; TODO: Check if SpeedUp
	if (EditorContext->GetLockCameraToView()) {
		TFICEditorAttribute<FFICFloatAttribute>& FOV_Attr = EditorContext->GetCameraEditor()->Get("Lens Settings").Get<TFICEditorAttribute<FFICFloatAttribute>>("FOV");
		EditorContext->CommitAutoKeyframe(this);
		FOV_Attr.SetValue(FOV_Attr.GetValue() + Delta);
		EditorContext->CommitAutoKeyframe(nullptr);
	} else {
		FOV += Delta;
	}
}

void AFICEditorCameraCharacter::ChangeSpeed(const FInputActionValue& ActionValue) {
	float Delta = ActionValue.GetMagnitude() * 100;
	// if (bIsSprinting) Delta *= 2; TODO: Check if SpeedUp
	//if (Delta) EditorContext->SetFlySpeed(EditorContext->GetFlySpeed() + Delta);
	// TODO: Fly Speed!
}

void AFICEditorCameraCharacter::ChangeFrame(const FInputActionValue& ActionValue) {
	float Delta = ActionValue.GetMagnitude();
	// if (bIsSprinting) Delta *= 2; TODO: Check if SpeedUp
	if (Delta) EditorContext->SetCurrentFrame(EditorContext->GetCurrentFrame() + Delta);
}

void AFICEditorCameraCharacter::Redo() {
	TSharedPtr<FFICChange> Change = EditorContext->ChangeList.PushChange();
	if (Change) Change->RedoChange();
}

void AFICEditorCameraCharacter::Undo() {
	TSharedPtr<FFICChange> Change = EditorContext->ChangeList.PopChange();
	if (Change) Change->UndoChange();
}

void AFICEditorCameraCharacter::NextKeyframe() {
	int64 Time;
	TSharedPtr<FFICKeyframe> KF = EditorContext->GetAllAttributes()->GetAttribute().GetNextKeyframe(EditorContext->GetCurrentFrame(), Time);
	if (KF) EditorContext->SetCurrentFrame(Time);
}

void AFICEditorCameraCharacter::PrevKeyframe() {
	int64 Time;
	TSharedPtr<FFICKeyframe> KF = EditorContext->GetAllAttributes()->GetAttribute().GetPrevKeyframe(EditorContext->GetCurrentFrame(), Time);
	if (KF) EditorContext->SetCurrentFrame(Time);
}

void AFICEditorCameraCharacter::ToggleAllKeyframes() {
	auto Change = MakeShared<FFICChange_Group>();
	Change->PushChange(MakeShared<FFICChange_ActiveFrame>(EditorContext, TNumericLimits<int64>::Min(), EditorContext->GetCurrentFrame()));
	BEGIN_ATTRIB_CHANGE(EditorContext->GetAllAttributes()->GetAttribute())
	EditorContext->ToggleCurrentKeyframes();
	END_ATTRIB_CHANGE(Change)
	EditorContext->ChangeList.PushChange(Change);
}

void AFICEditorCameraCharacter::ToggleAutoKeyframe() {
	EditorContext->SetLockCameraToView(!EditorContext->GetLockCameraToView());
}

void AFICEditorCameraCharacter::ToggleCursor() {
	EditorContext->GetPlayerCharacter()->ControlViewToggle();
}

void AFICEditorCameraCharacter::ToggleLockCamera() {
	EditorContext->SetLockCameraToView(!EditorContext->GetLockCameraToView());
}

void AFICEditorCameraCharacter::ToggleShowPath() {
	EditorContext->bShowPath = !EditorContext->bShowPath;
}

void AFICEditorCameraCharacter::Grab() {
	UInteractiveToolManager* ToolManager = AFICEditorSubsystem::GetFICEditorSubsystem(EditorContext)->ToolsContext->ToolManager;
	ToolManager->SelectActiveToolType(EToolSide::Mouse, "Grab");
	ToolManager->ActivateTool(EToolSide::Mouse);
}

void AFICEditorCameraCharacter::RightMousePress() {
	SetControlView(true, true);
}

void AFICEditorCameraCharacter::RightMouseRelease() {
	if (bControlViewTemp) {
		SetControlView(false);
	}
}

void AFICEditorCameraCharacter::OnLeftMouseDown() {
	AFICEditorSubsystem::GetFICEditorSubsystem(GetWorld())->OnLeftMouseDown(); // TODO: Register Input Handler in EditorSubsystem Instead
}

void AFICEditorCameraCharacter::OnLeftMouseUp() {
	AFICEditorSubsystem::GetFICEditorSubsystem(GetWorld())->OnLeftMouseUp(); // TODO: Register Input Handler in EditorSubsystem Instead
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
