#pragma once

#include "FICRuntimeProcess.h"
#include "Data/FICActiveSceneObjectManager.h"
#include "FICRuntimeProcessPlayScene.generated.h"

class AFICCaptureCamera;

UCLASS()
class UFICRuntimeProcessPlayScene : public UFICRuntimeProcess {
	GENERATED_BODY()
protected:
	FICFrameFloat Progress = 0.0f;
	FFICActiveSceneObjectManager ActiveSceneObjectManager;
	
public:
	UPROPERTY(BlueprintReadOnly)
	AFICScene* Scene = nullptr;

	UPROPERTY(BlueprintReadOnly)
	bool bBackground = false;
	
	// Begin UFICRuntimeProcess
	virtual bool NeedsRuntimeProcessCharacter() { return !bBackground; }
	virtual void Initialize() override;
	virtual void Start(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) override;
	virtual void Stop(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Shutdown() override;
	// End UFICRuntimeProcess
	
	FICFrameFloat GetProgress() { return Progress; }

	UFUNCTION(BlueprintCallable)
	static UFICRuntimeProcessPlayScene* StartPlayScene(AFICScene* InScene, bool bInBackground);
};
