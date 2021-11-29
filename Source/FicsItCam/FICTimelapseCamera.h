// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "GameFramework/Actor.h"
#include "FICTimelapseCamera.generated.h"

UCLASS()
class FICSITCAM_API AFICTimelapseCamera : public AActor, public IFGSaveInterface {
	GENERATED_BODY()
public:
	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;

	UPROPERTY()
	USceneCaptureComponent2D* CaptureComponent;
	
	/**
	 * The frequency of the screen shots taken in seconds per frame.
	 */
	UPROPERTY(SaveGame)
	float Frequency = 1;

	UPROPERTY()
	FTimerHandle CaptureTimer;
	UPROPERTY()
	FDateTime CaptureStart;
	UPROPERTY()
	int CaptureIncrement;

	AFICTimelapseCamera();

protected:
	// Begin AActor
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor
	
	void CaptureTick();

public:
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	// End IFGSaveInterface
	
	void StartTimelapse();
	void StopTimelapse();
};
