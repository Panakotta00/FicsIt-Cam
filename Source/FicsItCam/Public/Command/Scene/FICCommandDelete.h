#pragma once

#include "FGGameUserSettings.h"
#include "FICUtils.h"
#include "Command/FICCommand.h"
#include "FICCommandDelete.generated.h"

UCLASS()
class UFICCommandDelete : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandDelete() {
		bFinal = true;
		CommandName = TEXT("delete");
		CommandSyntax = TEXT("/fic delete <scene>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		TryGetSceneFromArg(Scene, 0)
		Scene->Destroy();
		InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' deleted."), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};