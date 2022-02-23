#pragma once

#include "FICRuntimeProcess.h"
#include "Runtime/FICCameraArgument.h"
#include "FICRuntimeProcessTimelapseCamera.generated.h"

UCLASS()
class UFICRuntimeProcessTimelapseCamera : public UFICRuntimeProcess {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FFICCameraArgument CameraArgument;

	UPROPERTY()
	AFICCaptureCamera* CaptureCamera = nullptr;

	UPROPERTY()
	float SecondsPerFrame;

	UPROPERTY()
	float Time = 0.0f;
	UPROPERTY()
	FDateTime CaptureStart;
	UPROPERTY()
	int CaptureIncrement;
	
	// Begin UFICRuntimeProcess
	virtual void Start(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) override;
	virtual void Stop(AFICRuntimeProcessorCharacter* InCharacter) override;
	// End UFICRuntimeProcess
};
