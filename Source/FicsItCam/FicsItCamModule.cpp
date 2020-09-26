#include "FicsItCamModule.h"

#include "FICSubsystemHolder.h"

void FFicsItCamModule::StartupModule() {
	FSubsystemInfoHolder::RegisterSubsystemHolder(UFICSubsystemHolder::StaticClass());
}

IMPLEMENT_GAME_MODULE(FFicsItCamModule, FicsItCam);
