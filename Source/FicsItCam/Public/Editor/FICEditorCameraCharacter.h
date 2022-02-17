#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "FICEditorCameraCharacter.generated.h"

class UFICCamera;
class UCineCameraComponent;
class AFICEditorCameraActor;
class UTransformProxy;
class ATransformGizmoActor;

class UFICEditorContext;
UCLASS()
class AFICEditorCameraCharacter : public ACharacter {
	GENERATED_BODY()

private:
	UPROPERTY()
	UCameraComponent* Camera;

	UPROPERTY()
	UFICCamera* LastCameraSceneObject = nullptr;

	bool bIsSprinting = false;
	bool bChangeFOV = false;
	bool bChangeSpeed = false;
	float FlyMultiplier = 10000;

	float RollRotationFixValue = 0.0f;
	bool bReposses = false;

	bool bControlView = true;
	bool bControlViewTemp = false;
	FVector2D LastCursorPos;
	
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
    void MoveForward(float Value);
	UFUNCTION()
    void MoveRight(float Value);
	UFUNCTION()
	void RotatePitch(float Value);
	UFUNCTION()
	void RotateYaw(float Value);
	UFUNCTION()
	void RotateRoll(float Value);

	UFUNCTION()
	void FlyUp(float Value);

	UFUNCTION()
	void EnterChangeFOV() {
		bChangeFOV = true;
		bChangeSpeed = false;
	}
	UFUNCTION()
	void LeaveChangeFOV() { bChangeSpeed = bChangeFOV = false; }

	UFUNCTION()
	void EnterSprint() { bIsSprinting = true; }
	UFUNCTION()
	void LeaveSprint() { bIsSprinting = false; }

	UFUNCTION()
	void EnterChangeSpeed() {
		bChangeSpeed = true;
		bChangeFOV = false;
	}
	UFUNCTION()
	void LeaveChangeSpeed() {
		bChangeFOV = bChangeSpeed = false;
	}

	UFUNCTION()
	void NextKeyframe();
	UFUNCTION()
	void PrevKeyframe();

	UFUNCTION()
	void NextFrame();
	UFUNCTION()
	void PrevFrame();

	UFUNCTION()
	void ToggleAutoKeyframe();
	UFUNCTION()
	void ToggleShowPath();
	UFUNCTION()
	void ToggleLockCamera();

	UFUNCTION()
	void RightMousePress();
	UFUNCTION()
	void RightMouseRelease();

	UFUNCTION()
	void Undo();
	UFUNCTION()
	void Redo();

	UFUNCTION()
	void ChangedKeyframe();
	
	UFUNCTION()
	void Zoom(float Value);

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
