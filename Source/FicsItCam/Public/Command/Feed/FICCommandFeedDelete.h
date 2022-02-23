#pragma once

#include "FICCommandFeed.h"
#include "FICUtils.h"
#include "Runtime/Process/FICRuntimeProcessCameraFeed.h"
#include "FICCommandFeedDelete.generated.h"

UCLASS()
class UFICCommandFeedDelete : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandFeedDelete() {
		bFinal = true;
		ParentCommand = UFICCommandFeed::StaticClass();
		CommandName = TEXT("delete");
		CommandSyntax = TEXT("/fic feed delete <camera>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		FeedKey(Key, 0)
		CheckFeedFromKey(Feed, Key)
		if (!SubSys->RemoveRuntimeProcess(Feed)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to delete Feed for '%s'!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Feed for '%s' deleted!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};