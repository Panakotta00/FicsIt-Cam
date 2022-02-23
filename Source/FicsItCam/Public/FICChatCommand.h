#pragma once

#include "command/ChatCommandLibrary.h"
#include "FICChatCommand.generated.h"

UCLASS()
class AFICChatCommand : public AChatCommandInstance {
	GENERATED_BODY()
	
public:
	AFICChatCommand();

	// Begin AChatCommandInstance
	virtual EExecutionStatus ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) override;
	// End AChatCommandInstance
};
