#include "FicsItCamModule.h"

#include "FGGameMode.h"
#include "FGGameViewportClient.h"
#include "FGItemRegrowSubsystem.h"
#include "FGSignificanceManager.h"
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

void UFGSignificanceManager_Update(CallScope<void(*)(UFGSignificanceManager*, TArrayView<const FTransform>)> Scope, UFGSignificanceManager* Manager, const TArrayView<const FTransform>& InViewpoints) {
	TArray<FTransform> Viewpoints;
	Viewpoints.Append(InViewpoints);
	Viewpoints.Add(FTransform(FVector(0,0,-1).Rotation(), FVector(0, 0, 25000)));
	Scope(Manager, Viewpoints);
}

void FFicsItCamModule::StartupModule() {
	FSlateStyleRegistry::UnRegisterSlateStyle(FFICEditorStyles::GetStyleSetName());
	FFICEditorStyles::Initialize();
	
	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
#if !WITH_EDITOR
		SUBSCRIBE_METHOD(AFGItemRegrowSubsystem::AddPickup, &AddPickup_Override);

		SUBSCRIBE_METHOD(AFGGameMode::DispatchBeginPlay, &FGGameMode_Play);

		SUBSCRIBE_METHOD_VIRTUAL(UFGSignificanceManager::Update, GetDefault<UFGSignificanceManager>(), UFGSignificanceManager_Update);
#endif

		AFGGameMode* gm = const_cast<AFGGameMode*>(GetDefault<AFGGameMode>());
		gm->ReplaySpectatorPlayerControllerClass = gm->PlayerControllerClass;
	});

	//av_log_set_level(AV_LOG_DEBUG);
	//av_log_set_callback(FfmpegLogCallback);

	/*FSceneViewExtensionIsActiveFunctor SceneViewExtensionIsActive;
	SceneViewExtensionIsActive.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context) {
		return true;
	};
	SceneViewExtension = FSceneViewExtensions::NewExtension<FFICSceneViewExtension>();
	SceneViewExtension->IsActiveThisFrameFunctions.Add(SceneViewExtensionIsActive);*/
}

void FFicsItCamModule::ShutdownModule() {
	FFICEditorStyles::Shutdown();
}

IMPLEMENT_GAME_MODULE(FFicsItCamModule, FicsItCam);
