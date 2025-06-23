#pragma once

#include "Command/ChatCommandInstance.h"
#include "Runtime/FICRuntimeProcessorCharacter.h"
#include "FICRuntimeProcess.generated.h"

UCLASS(BlueprintType)
class UFICRuntimeProcess : public UObject {
	GENERATED_BODY()
public:
	virtual bool NeedsRuntimeProcessCharacter() { return false; }
	virtual void Initialize() {}
	virtual void Start(AFICRuntimeProcessorCharacter* InCharacter) {}
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {}
	virtual bool IsPersistent() { return false; }
	virtual bool PreSaveProcess() { return false; }
	virtual void PostLoadProcess() {}
	virtual void Stop(AFICRuntimeProcessorCharacter* InCharacter) {}
	virtual void Shutdown() {}
};
