#pragma once
#include "FICRuntimeProcess.h"

#include "FICRuntimeProcessPlayScene.generated.h"

class AFICCaptureCamera;

UCLASS()
class UFICRuntimeProcessPlayScene : public UFICRuntimeProcess {
	GENERATED_BODY()
private:
	float Progress = 0.0f;
	
public:
	UPROPERTY()
	AFICScene* Scene = nullptr;
	
	// Begin UFICRuntimeProcess
	virtual void Initialize(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaTime) override;
	virtual void Shutdown(AFICRuntimeProcessorCharacter* InCharacter) override;
	// End UFICRuntimeProcess
};