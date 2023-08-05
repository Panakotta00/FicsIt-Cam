#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "FICEditorCameraCharacter.generated.h"

class UFICCamera;
class UCineCameraComponent;
class AFICEditorCameraActor;
class UTransformProxy;
class UFICEditorContext;

UCLASS()
class AFICEditorCameraCharacter : public ACharacter {
	GENERATED_BODY()

private:
	UPROPERTY()
	UCameraComponent* Camera;

	UPROPERTY()
	UFICCamera* LastCameraSceneObject = nullptr;

	float FlyMultiplier = 10000;

	float RollRotationFixValue = 0.0f;
	bool bReposses = false;

	float FOV = 0;

	bool bControlView = true;
	bool bControlViewTemp = false;
	FVector2D LastCursorPos;
	bool bWasChangedDirectly = false;
	bool bChangedByMovement = false;
	FDelegateHandle OnCurrentFrameChangedHandle;
	
	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;

public:
	float MaxFlySpeed = 1000;

	AFICEditorCameraCharacter();
	
	// Begin AActor
	virtual void Tick( float DeltaSeconds ) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor

	// Begin ACharacter
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	// End ACharacter

	UFUNCTION()
	void Move(const FInputActionValue& ActionValue);
	UFUNCTION()
	void Rotate(const FInputActionValue& ActionValue);
	UFUNCTION()
	void ChangeFOV(const FInputActionValue& ActionValue);
	UFUNCTION()
	void ChangeSpeed(const FInputActionValue& ActionValue);
	UFUNCTION()
	void ChangeFrame(const FInputActionValue& ActionValue);
	UFUNCTION()
	void Redo();
	UFUNCTION()
	void Undo();
	UFUNCTION()
	void NextKeyframe();
	UFUNCTION()
	void PrevKeyframe();
	UFUNCTION()
	void ToggleAllKeyframes();
	UFUNCTION()
	void ToggleAutoKeyframe();
	UFUNCTION()
	void ToggleCursor();
	UFUNCTION()
	void ToggleLockCamera();
	UFUNCTION()
	void ToggleShowPath();
	UFUNCTION()
	void Grab();
	
	UFUNCTION()
	void RightMousePress();
	UFUNCTION()
	void RightMouseRelease();

	UFUNCTION()
	void OnLeftMouseDown();
	UFUNCTION()
	void OnLeftMouseUp();

	UFUNCTION()
	void SetEditorContext(UFICEditorContext* InEditorContext);
	
	// Updates Camera Values based on the current value cache in the referenced editor context
	void UpdateValues();

	void SetControlView(bool bInControlView, bool bIsTemporary = false);
	bool IsControlView() { return bControlView; }
	void ControlViewToggle() { SetControlView(!bControlView); }
};
