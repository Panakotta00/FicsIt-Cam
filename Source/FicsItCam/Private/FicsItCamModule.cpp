#include "FicsItCamModule.h"

#include "FGGameMode.h"
#include "FGItemRegrowSubsystem.h"
#include "Renderer/Private/SceneRendering.h"
#include "Renderer/Private/ShadowRendering.h"

DEFINE_LOG_CATEGORY(LogFicsItCam);

#pragma optimize("", off)
void FFicsItCamModule::AddPickup_Override(CallScope<void(*)(AFGItemRegrowSubsystem*, AFGItemPickup*)>& Scope, AFGItemRegrowSubsystem* self, AFGItemPickup* Item) {
	if (!self || !Item || self->mPickups.Contains(Item)) {
		Scope.Cancel();
	}
}

void FGGameMode_Play(CallScope<void(*)(AActor*, bool)>& Scope, AActor* gm, bool) {
}

void FFicsItCamModule::StartupModule() {
	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
		SUBSCRIBE_METHOD(AFGItemRegrowSubsystem::AddPickup, &AddPickup_Override);

		SUBSCRIBE_METHOD(AFGGameMode::DispatchBeginPlay, &FGGameMode_Play)

		AFGGameMode* gm = const_cast<AFGGameMode*>(GetDefault<AFGGameMode>());
		gm->ReplaySpectatorPlayerControllerClass = gm->PlayerControllerClass;
	});
}

IMPLEMENT_GAME_MODULE(FFicsItCamModule, FicsItCam);

#pragma optimize("", on)
