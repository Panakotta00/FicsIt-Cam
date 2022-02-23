#pragma once

#include "FICCommandTimelapse.h"
#include "FICCommandTimelapseStop.generated.h"

UCLASS()
class UFICCommandTimelapseStop : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandTimelapseStop() {
		bFinal = true;
		ParentCommand = UFICCommandTimelapse::StaticClass();
		CommandName = TEXT("stop");
		CommandSyntax = TEXT("/fic timelapse stop <camera>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		TimelapseKey(Key, 0)
		if (!SubSys->GetActiveRuntimeProcessesMap().Contains(Key)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Timelapse for '%s' is not currenty running!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		CheckTimelapseFromKey(Timelapse, Key)
		if (!SubSys->StopRuntimeProcess(Timelapse)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to stop Timelapse for '%s'!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Timelapse for '%s' stopped!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};