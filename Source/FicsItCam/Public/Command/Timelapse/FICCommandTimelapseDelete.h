#pragma once

#include "FICCommandTimelapse.h"
#include "FICCommandTimelapseDelete.generated.h"

UCLASS()
class UFICCommandTimelapseDelete : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandTimelapseDelete() {
		bFinal = true;
		ParentCommand = UFICCommandTimelapse::StaticClass();
		CommandName = TEXT("delete");
		CommandSyntax = TEXT("/fic timelapse delete <camera>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		TimelapseKey(Key, 0)
		CheckTimelapseFromKey(Timelapse, Key)
		if (!SubSys->RemoveRuntimeProcess(Timelapse)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to delete Timelapse for '%s'!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Timelapse for '%s' deleted!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};