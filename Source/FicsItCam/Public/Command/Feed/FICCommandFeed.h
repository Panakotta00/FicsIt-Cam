#pragma once

#include "Command/FICCommand.h"
#include "Runtime/Process/FICRuntimeProcessCameraFeed.h"
#include "FICCommandFeed.generated.h"

#define FeedKey(Key, ArgsNum) \
	FString Key = FString::Printf(TEXT("Feed_%s"), *InArgs[ArgsNum]);

#define CheckFeedFromKey(TimelapseVar, Key) \
	UFICRuntimeProcessCameraFeed* TimelapseVar = Cast<UFICRuntimeProcessCameraFeed>(SubSys->FindRuntimeProcess(Key)); \
	if (!TimelapseVar) { \
		InSender->SendChatMessage(FString::Printf(TEXT("Feed for '%s' does not exist."), *InArgs[0]), FColor::Red); \
		return EExecutionStatus::BAD_ARGUMENTS; \
	}

UCLASS()
class UFICCommandFeed : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandFeed() {
		CommandName = TEXT("feed");
	}
};