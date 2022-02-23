#pragma once

#include "FICCommandFeed.h"
#include "FICCommandFeedStart.generated.h"

UCLASS()
class UFICCommandFeedStart : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandFeedStart() {
		bFinal = true;
		ParentCommand = UFICCommandFeed::StaticClass();
		CommandName = TEXT("start");
		CommandSyntax = TEXT("/fic feed start <camera>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		FeedKey(Key, 0)
		CheckFeedFromKey(Feed, Key)
		if (SubSys->GetActiveRuntimeProcessesMap().Contains(Key)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Feed for '%s' is already started!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		if (!SubSys->StartRuntimeProcess(Feed)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to start Feed for '%s'!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Feed for '%s' started!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};