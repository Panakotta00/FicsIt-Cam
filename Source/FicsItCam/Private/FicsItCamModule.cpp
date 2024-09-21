#include "FicsItCamModule.h"

#include "CoreDelegates.h"
#include "FGGameMode.h"
#include "FGItemRegrowSubsystem.h"
#include "Editor/UI/FICEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"

DEFINE_LOG_CATEGORY(LogFicsItCam);

void FFicsItCamModule::AddPickup_Override(CallScope<void(*)(AFGItemRegrowSubsystem*, AFGItemPickup*)>& Scope, AFGItemRegrowSubsystem* self, AFGItemPickup* Item) {
	if (!self || !Item || self->mPickups.Contains(Item)) {
		Scope.Cancel();
	}
}

void FGGameMode_Play(CallScope<void(*)(AActor*, bool)>& Scope, AActor* gm, bool) {
}

void FfmpegLogCallback(void* ptr, int level, const char* fmt, va_list varargs) {
	char msg[4096];
	int len = vsnprintf(msg, sizeof(msg), fmt, varargs);
	FUTF8ToTCHAR conv(msg, len);
	UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg Log: %s"), conv.Get());
}

void FFicsItCamModule::StartupModule() {
	FSlateStyleRegistry::UnRegisterSlateStyle(FFICEditorStyles::GetStyleSetName());
	FFICEditorStyles::Initialize();
	
	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
#if !WITH_EDITOR
		SUBSCRIBE_METHOD(AFGItemRegrowSubsystem::AddPickup, &AddPickup_Override);

		SUBSCRIBE_METHOD(AFGGameMode::DispatchBeginPlay, &FGGameMode_Play);
#endif

		AFGGameMode* gm = const_cast<AFGGameMode*>(GetDefault<AFGGameMode>());
		gm->ReplaySpectatorPlayerControllerClass = gm->PlayerControllerClass;
	});

	//av_log_set_level(AV_LOG_DEBUG);
	//av_log_set_callback(FfmpegLogCallback);
}

void FFicsItCamModule::ShutdownModule() {
	FFICEditorStyles::Shutdown();
}

void FFicsItCamModule::PostLoadCallback() {
	FFICEditorStyles::Shutdown();
	FFICEditorStyles::Initialize();
}

IMPLEMENT_GAME_MODULE(FFicsItCamModule, FicsItCam);
