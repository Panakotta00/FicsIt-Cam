#pragma once

#include "command/ChatCommandLibrary.h"
#include "FICCommand.generated.h"

UCLASS()
class AFICCommand : public AChatCommandInstance {
	GENERATED_BODY()
	
public:
	AFICCommand();

	// Begin AChatCommandInstance
	virtual EExecutionStatus ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) override;
	// End AChatCommandInstance
};
