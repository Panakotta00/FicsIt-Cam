#include "FicsItCamModule.h"

#include <libavutil/log.h>

#include "FGGameMode.h"
#include "FGItemRegrowSubsystem.h"
#include "Editor/UI/FICEditorStyle.h"
#include "Renderer/Private/SceneRendering.h"
#include "Renderer/Private/ShadowRendering.h"
#include "Styling/SlateStyleRegistry.h"

DEFINE_LOG_CATEGORY(LogFicsItCam);

UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_Movement, "Input.FIC.Editor.Movement");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_Rotation, "Input.FIC.Editor.Rotation");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_FOV, "Input.FIC.Editor.FOV");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_Frame, "Input.FIC.Editor.Frame");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_Grab, "Input.FIC.Editor.Grab");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_NextKeyframe, "Input.FIC.Editor.NextKeyframe");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_PrevKeyframe, "Input.FIC.Editor.PrevKeyframe");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_Redo, "Input.FIC.Editor.Redo");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_Undo, "Input.FIC.Editor.Undo");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_Speed, "Input.FIC.Editor.Speed");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_SpeedUp, "Input.FIC.Editor.SpeedUp");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_ToggleAllKeyframes, "Input.FIC.Editor.ToggleAllKeyframes");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_ToggleAutoKeyframes, "Input.FIC.Editor.ToggleAutoKeyframes");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_ToggleCursor, "Input.FIC.Editor.ToggleCursor");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_ToggleLockCamera, "Input.FIC.Editor.ToggleLockCamera");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Editor_ToggleShowPath, "Input.FIC.Editor.ToggleShowPath");
UE_DEFINE_GAMEPLAY_TAG(Input_FIC_Playback_StopAnimation, "Input.FIC.Playback.StopAnimation");

#pragma optimize("", off)
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

		SUBSCRIBE_METHOD(AFGGameMode::DispatchBeginPlay, &FGGameMode_Play)
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

IMPLEMENT_GAME_MODULE(FFicsItCamModule, FicsItCam);

#pragma optimize("", on)
