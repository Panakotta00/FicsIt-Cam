#include "FicsItCamModule.h"

#include "FICSubsystemHolder.h"
#include "Patching/NativeHookManager.h"
#include "Slate/SceneViewport.h"

DEFINE_LOG_CATEGORY(LogFicsItCam);

void FFicsItCamModule::StartupModule() {
	UE_LOG(LogFicsItCam, Warning, TEXT("Oh noe..."))
}

IMPLEMENT_GAME_MODULE(FFicsItCamModule, FicsItCam);
