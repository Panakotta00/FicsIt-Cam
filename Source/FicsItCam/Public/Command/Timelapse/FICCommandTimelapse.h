#pragma once

#include "Command/FICCommand.h"
#include "Runtime/Process/FICRuntimeProcessTimelapseCamera.h"
#include "FICCommandTimelapse.generated.h"

#define TimelapseKey(Key, ArgsNum) \
	FString Key = FString::Printf(TEXT("Timelapse_%s"), *InArgs[ArgsNum]);

#define CheckTimelapseFromKey(TimelapseVar, Key) \
	UFICRuntimeProcessTimelapseCamera* TimelapseVar = Cast<UFICRuntimeProcessTimelapseCamera>(SubSys->FindRuntimeProcess(Key)); \
	if (!TimelapseVar) { \
        InSender->SendChatMessage(FString::Printf(TEXT("Timelapse for '%s' does not exist."), *InArgs[0]), FColor::Red); \
        return EExecutionStatus::BAD_ARGUMENTS; \
    }

UCLASS()
class UFICCommandTimelapse : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandTimelapse() {
		CommandName = TEXT("timelapse");
	}
};