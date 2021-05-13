#include "FicsItCamModule.h"

#include "Patching/NativeHookManager.h"

DEFINE_LOG_CATEGORY(LogFicsItCam);

void FFicsItCamModule::StartupModule() {
	UE_LOG(LogFicsItCam, Warning, TEXT("Oh noe..."))
}

IMPLEMENT_GAME_MODULE(FFicsItCamModule, FicsItCam);
