#pragma once

#include "FICAnimation.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"

#include "FICCamera.generated.h"

UCLASS()
class AFICCamera : public AActor {
	GENERATED_BODY()

private:
	UPROPERTY()
	AFICAnimation* Animation = nullptr;

	float Progress = 0.0f;

	UPROPERTY()
	AActor* PrevViewTarget = nullptr;

	bool bPrevCanBeDamaged = false;


	FVector CameraPos;
	FRotator CameraRot;
	float CameraFOV;
	UPROPERTY()
	UCameraComponent* PrevCamera = nullptr;

	UPROPERTY()
	APlayerCameraManager* CameraManager = nullptr;

public:
	UPROPERTY()
	UCameraComponent* Camera = nullptr;

	AFICCamera();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam")
	void StartAnimation(AFICAnimation* inAnimation);

	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam")
	void StopAnimation();
};
