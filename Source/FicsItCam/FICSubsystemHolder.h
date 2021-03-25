#pragma once

#include "FICSubsystem.h"
#include "Subsystem/ModSubsystemHolder.h"
#include "FICSubsystemHolder.generated.h"

UCLASS()
class UFICSubsystemHolder : public UModSubsystemHolder {
	GENERATED_BODY()
	
public:
	UPROPERTY()
	AFICSubsystem* Subsystem;

	// Begin USMLSubsystemHolder
	virtual void InitSubsystems() override;
	// End USMLSubsystemHolder
};
