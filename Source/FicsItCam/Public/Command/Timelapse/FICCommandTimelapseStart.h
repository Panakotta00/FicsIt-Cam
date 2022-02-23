#pragma once

#include "FICCommandTimelapse.h"
#include "FICCommandTimelapseStart.generated.h"

UCLASS()
class UFICCommandTimelapseStart : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandTimelapseStart() {
		bFinal = true;
		ParentCommand = UFICCommandTimelapse::StaticClass();
		CommandName = TEXT("start");
		CommandSyntax = TEXT("/fic timelapse start <camera>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		TimelapseKey(Key, 0)
		CheckTimelapseFromKey(Timelapse, Key)
		if (SubSys->GetActiveRuntimeProcessesMap().Contains(Key)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Timelapse for '%s' is already started!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		if (!SubSys->StartRuntimeProcess(Timelapse)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to start Timelapse for '%s'!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Timelapse for '%s' started!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};