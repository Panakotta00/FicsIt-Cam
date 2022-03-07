#pragma once

#include "Command/FICCommand.h"
#include "Runtime/Process/FICRuntimeProcessPlayScene.h"
#include "FICCommandStop.generated.h"

UCLASS()
class UFICCommandStop : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandStop() {
		bFinal = true;
		CommandName = TEXT("stop");
		CommandSyntax = TEXT("/fic stop <scene>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		GetSceneKey(Key, *InArgs[0])
		if (!SubSys->GetRuntimeProcesses().Contains(Key)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' has no current player!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		UFICRuntimeProcessPlayScene* PlayScene = Cast<UFICRuntimeProcessPlayScene>(SubSys->GetRuntimeProcesses()[Key]);
		if (!SubSys->RemoveRuntimeProcess(PlayScene)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to remove Scene '%s' player!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' Player removed!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};