#include "FICSubsystemHolder.h"

void UFICSubsystemHolder::InitSubsystems() {
	Super::InitSubsystems();

	SpawnSubsystem(Subsystem, AFICSubsystem::StaticClass(), "FICSubsystem");
}
