#pragma once

#include "Runtime/FICRuntimeProcessorCharacter.h"
#include "FICRuntimeProcess.generated.h"

UCLASS()
class UFICRuntimeProcess : public UObject {
	GENERATED_BODY()
public:
	virtual void Initialize(AFICRuntimeProcessorCharacter* InCharacter) {}
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {}
	virtual void Shutdown(AFICRuntimeProcessorCharacter* InCharacter) {}
};
