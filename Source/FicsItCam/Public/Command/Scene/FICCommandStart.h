#pragma once

#include "Command/FICCommand.h"
#include "Runtime/Process/FICRuntimeProcessPlayScene.h"
#include "FICCommandStart.generated.h"

UCLASS()
class UFICCommandStart : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandStart() {
		bFinal = true;
		CommandName = TEXT("start");
		CommandSyntax = TEXT("/fic start <scene>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		GetSceneKey(Key, *InArgs[0])
		if (SubSys->GetActiveRuntimeProcessesMap().Contains(Key)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' is already playing!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		if (!SubSys->GetRuntimeProcesses().Contains(Key)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Scene player fro Scene '%s' not yet created!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		UFICRuntimeProcessPlayScene* PlayScene = Cast<UFICRuntimeProcessPlayScene>(SubSys->GetRuntimeProcesses()[Key]);
		if (!SubSys->StartRuntimeProcess(PlayScene)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to start playing Scene '%s'!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' started to play!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};