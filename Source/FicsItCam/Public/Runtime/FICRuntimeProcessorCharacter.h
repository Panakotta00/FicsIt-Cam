#pragma once

#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Data/FICAnimation.h"
#include "GameFramework/Character.h"

#include "FICRuntimeProcessorCharacter.generated.h"

class UFICRuntimeProcess;
class UCineCameraComponent;

UCLASS()
class AFICRuntimeProcessorCharacter : public ACharacter {
	GENERATED_BODY()

private:
	UPROPERTY()
	UFICRuntimeProcess* RuntimeProcess = nullptr;

	bool bIsRepossesing = false;

public:
	UPROPERTY()
	UCameraComponent* Camera = nullptr;

	AFICRuntimeProcessorCharacter();

	// Begin AActor
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void BeginPlay() override;
	// End AActor

	// Begin ACharacter
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	// End ACharacter

	void Initialize(UFICRuntimeProcess* RuntimeProcess);
	void Shutdown();

	void SetTimeDilation(float InTimeDilation);

	void SetCamera(bool bEnabled, bool bCinematic);
};
