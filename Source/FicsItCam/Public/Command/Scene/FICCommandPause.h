#pragma once

#include "Command/FICCommand.h"
#include "Runtime/Process/FICRuntimeProcessPlayScene.h"
#include "FICCommandPause.generated.h"

UCLASS()
class UFICCommandPause : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandPause() {
		bFinal = true;
		CommandName = TEXT("pause");
		CommandSyntax = TEXT("/fic pause <scene>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		GetSceneKey(Key, *InArgs[0])
		if (!SubSys->GetActiveRuntimeProcessesMap().Contains(Key)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' is not currenty playing!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		UFICRuntimeProcessPlayScene* PlayScene = Cast<UFICRuntimeProcessPlayScene>(SubSys->GetActiveRuntimeProcessesMap()[Key]);
		if (!SubSys->StopRuntimeProcess(PlayScene)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to stop Scene '%s' from playing!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' stopped from playing!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};