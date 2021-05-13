#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "FICEditorCameraCharacter.generated.h"

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
	AFICEditorCameraActor* CameraActor = nullptr;

	bool bIsSprinting = false;
	bool bChangeFOV = false;
	bool bChangeSpeed = false;
	float FlyMultiplier = 10000;

	float RollRotationFixValue = 0.0f;
	
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
	void SetEditorContext(UFICEditorContext* InEditorContext);

	UFUNCTION()
	void ChangedKeyframe();

	UFUNCTION()
	void Zoom(float Value);

	// Updates Camera Values based on the current value cache in the referenced editor context
	void UpdateValues();
};
