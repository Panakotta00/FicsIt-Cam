#pragma once

#include "FICAnimation.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"


#include "FICCameraCharacter.generated.h"

class UCineCameraComponent;
UCLASS()
class AFICCameraCharacter : public ACharacter {
	GENERATED_BODY()

private:
	UPROPERTY()
	AFICAnimation* Animation = nullptr;

	float Progress = 0.0f;

	UPROPERTY()
	ACharacter* OriginalCharacter = nullptr;

	bool bStartFinished = false;
	
	UPROPERTY()
	FTimerHandle WorldStreamTimer;
	
	UFUNCTION()
    void OnTickWorldStreamTimer();
	
public:
	UPROPERTY()
	UCameraComponent* Camera = nullptr;

	UPROPERTY()
	USceneCaptureComponent2D* CaptureComponent = nullptr;

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget = nullptr;

	bool bDoRender = false;

	AFICCameraCharacter();

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

	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam")
	void StartAnimation(AFICAnimation* inAnimation, bool bDoRender);

	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam")
	void StopAnimation();

	void SetTimeDilation(float InTimeDialation);
};
