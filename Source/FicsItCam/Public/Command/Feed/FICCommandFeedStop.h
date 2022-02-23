#pragma once

#include "FICCommandFeed.h"
#include "FICCommandFeedStop.generated.h"

UCLASS()
class UFICCommandFeedStop : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandFeedStop() {
		bFinal = true;
		ParentCommand = UFICCommandFeed::StaticClass();
		CommandName = TEXT("stop");
		CommandSyntax = TEXT("/fic feed stop <camera>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		FeedKey(Key, 0)
		if (!SubSys->GetActiveRuntimeProcessesMap().Contains(Key)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Feed for '%s' is not currenty running!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		CheckFeedFromKey(Feed, Key)
		if (!SubSys->StopRuntimeProcess(Feed)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to stop Feed for '%s'!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Feed for '%s' stopped!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};