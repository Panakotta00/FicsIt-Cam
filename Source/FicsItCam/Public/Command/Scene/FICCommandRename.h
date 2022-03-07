#pragma once

#include "Command/FICCommand.h"
#include "FICCommandRename.generated.h"

UCLASS()
class UFICCommandRename : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandRename() {
		bFinal = true;
		CommandName = TEXT("rename");
		CommandSyntax = TEXT("/fic rename <old scene> <new scene>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(2)
		TryGetSceneFromArg(OldScene, 0)
		if (!UFICUtils::IsValidFICObjectName(InArgs[1])) {
			InSender->SendChatMessage(FString::Printf(TEXT("'%s' is no valid scene name!"), *InArgs[1]));
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		if (AFICSubsystem::GetFICSubsystem(InSender)->FindSceneByName(InArgs[1])) {
			InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' already exists!"), *InArgs[1]));
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		OldScene->SceneName = InArgs[1];
		InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' renamed to '%s'."), *InArgs[0], *InArgs[1]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};