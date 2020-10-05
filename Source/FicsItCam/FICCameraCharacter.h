#pragma once

#include "FICAnimation.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"


#include "FICCameraCharacter.generated.h"

UCLASS()
class AFICCameraCharacter : public ACharacter {
	GENERATED_BODY()

private:
	UPROPERTY()
	AFICAnimation* Animation = nullptr;

	float Progress = 0.0f;

	UPROPERTY()
	ACharacter* OriginalCharacter = nullptr;
	
public:
	UPROPERTY()
	UCameraComponent* Camera = nullptr;

	AFICCameraCharacter();

	// Begin AActor
	virtual void Tick( float DeltaSeconds ) override;
	virtual void BeginPlay() override;
	// End AActor

	// Begin ACharacter
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	// End ACharacter

	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam")
	void StartAnimation(AFICAnimation* inAnimation);

	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam")
	void StopAnimation();
};
