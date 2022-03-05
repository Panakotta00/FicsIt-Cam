#include "FicsItCamModule.h"

#include "FGGameMode.h"
#include "FGItemRegrowSubsystem.h"
#include "Editor/UI/FICEditorStyle.h"
#include "Renderer/Private/SceneRendering.h"
#include "Renderer/Private/ShadowRendering.h"
#include "Styling/SlateStyleRegistry.h"

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
	FSlateStyleRegistry::UnRegisterSlateStyle(FFICEditorStyles::GetStyleSetName());
	FFICEditorStyles::Initialize();
	
	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
#if !WITH_EDITOR
		SUBSCRIBE_METHOD(AFGItemRegrowSubsystem::AddPickup, &AddPickup_Override);

		SUBSCRIBE_METHOD(AFGGameMode::DispatchBeginPlay, &FGGameMode_Play)
#endif

		AFGGameMode* gm = const_cast<AFGGameMode*>(GetDefault<AFGGameMode>());
		gm->ReplaySpectatorPlayerControllerClass = gm->PlayerControllerClass;
	});
}

void FFicsItCamModule::ShutdownModule() {
	FFICEditorStyles::Shutdown();
}

IMPLEMENT_GAME_MODULE(FFicsItCamModule, FicsItCam);

#pragma optimize("", on)
