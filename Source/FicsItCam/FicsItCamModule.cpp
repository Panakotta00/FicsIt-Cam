#include "FicsItCamModule.h"

#include "Patching/NativeHookManager.h"
#include "Renderer/Private/SceneRendering.h"
#include "Renderer/Private/ShadowRendering.h"

DEFINE_LOG_CATEGORY(LogFicsItCam);

void FFicsItCamModule::StartupModule() {
}

IMPLEMENT_GAME_MODULE(FFicsItCamModule, FicsItCam);
