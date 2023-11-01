#pragma once

#include "EngineUtils.h"
#include "Command/FICCommand.h"
#include "FICCommandList.generated.h"

UCLASS()
class UFICCommandList : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandList() {
		bFinal = true;
		CommandName = TEXT("list");
		CommandSyntax = TEXT("/fic list");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		InSender->SendChatMessage(FString::Printf(TEXT("List of Scenes:")), FColor::Green);
		for (TActorIterator<AFICScene> Scene(InSender->GetWorld()); Scene; ++Scene) {
			InSender->SendChatMessage(Scene->SceneName, FColor::Green);
		}
		return EExecutionStatus::COMPLETED;
	}
};