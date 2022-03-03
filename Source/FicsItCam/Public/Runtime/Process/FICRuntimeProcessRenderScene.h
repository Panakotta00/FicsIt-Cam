#pragma once

#include "FICRuntimeProcessPlayScene.h"
#include "FICRUntimeProcessRenderScene.generated.h"

UCLASS()
class UFICRuntimeProcessRenderScene : public UFICRuntimeProcessPlayScene {
	GENERATED_BODY()
public:
	UPROPERTY()
	AFICCaptureCamera* CaptureCamera = nullptr;

	FICFrame FrameProgress = 0;

	// Begin UFICRuntimeProcess
	virtual void Start(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) override;
	virtual void Stop(AFICRuntimeProcessorCharacter* InCharacter) override;
	// End UFICRuntimeProcess
};