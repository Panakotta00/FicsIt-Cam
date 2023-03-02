#include "Runtime/Process/FICRuntimeProcessRenderScene.h"

#include "AudioDevice.h"
#include "EngineModule.h"
#include "FICSubsystem.h"
#include "IImageWrapperModule.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GTE/Mathematics/Logger.h"
#include "Runtime/FICCaptureCamera.h"
#include "Slate/SceneViewport.h"
#include "Widgets/SViewport.h"

void UFICRuntimeProcessRenderScene::Start(AFICRuntimeProcessorCharacter* InCharacter) {
	Super::Start(InCharacter);

	auto* Settings = GetWorld()->GetWorldSettings();
	PrevMinUndilatedFrameTime = Settings->MinUndilatedFrameTime;
	PrevMaxUndilatedFrameTime = Settings->MaxUndilatedFrameTime;
	if (Scene->bBulletTime) {
		Settings->MinUndilatedFrameTime = 0;
		Settings->MaxUndilatedFrameTime = 0;
	} else {
		Settings->MinUndilatedFrameTime = 1.0/(double)Scene->FPS;
		Settings->MaxUndilatedFrameTime = Settings->MinUndilatedFrameTime;
	}
	FrameProgress = Scene->AnimationRange.Begin;
	//FAudioDeviceManager::Get()->GetActiveAudioDevice().GetAudioDevice()->)
	FAudioThread::StopAudioThread();
	
	FViewportClient* ViewportClient = GetWorld()->GetGameViewport();
	DummyViewport = MakeShared<FFICRendererViewport>(ViewportClient, Scene->ResolutionWidth, Scene->ResolutionHeight);

	// Create Save Path
	FString FSP;
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	if (FSP.IsEmpty()) {
		FSP = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/") TEXT("SaveGames/") TEXT("FicsItCam/"), Scene->SceneName);
	}
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*FSP)) PlatformFile.CreateDirectoryTree(*FSP);
	
	Path = FPaths::Combine(FSP, FDateTime::Now().ToString() + TEXT(".mp4"));
	
	Exporter = MakeShared<FSequenceMP4Exporter>(FIntPoint(Scene->ResolutionWidth, Scene->ResolutionHeight), Scene->FPS, Path);
	Exporter->Init();
}

void UFICRuntimeProcessRenderScene::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	if(GetWorld()->IsLevelStreamingRequestPending(GetWorld()->GetFirstPlayerController())) return;

	Progress = (float)FrameProgress / (float)Scene->FPS;
	Super::Tick(InCharacter, DeltaSeconds);

	// Capture Image
	FlushRenderingCommands();

	//DummyViewport->EnqueueBeginRenderFrame(false);
	UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
	FCanvas Canvas(DummyViewport.Get(), NULL, ViewportClient->GetWorld(), ViewportClient->GetWorld()->FeatureLevel);
	ViewportClient->Draw(DummyViewport.Get(), &Canvas);
	Canvas.Flush_GameThread();
	//FIntPoint RestoreSize(ViewportClient->Viewport->GetSizeXY().X, ViewportClient->Viewport->GetSizeXY().Y);
	//ENQUEUE_RENDER_COMMAND(EndDrawingCommand)([RestoreSize, this](FRHICommandListImmediate& RHICmdList) {
		//DummyViewport->EndRenderFrame(RHICmdList, false, false);
		//GetRendererModule().SceneRenderTargetsSetBufferSize(RestoreSize.X, RestoreSize.Y);
	//});

	// Store Image
	AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(Exporter.ToSharedRef(), DummyViewport.ToSharedRef());
	
	++FrameProgress;
}

void UFICRuntimeProcessRenderScene::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	Super::Stop(InCharacter);

	Exporter->Finish();
	
	auto* Settings = GetWorld()->GetWorldSettings();
	Settings->MinUndilatedFrameTime = PrevMinUndilatedFrameTime;
	Settings->MaxUndilatedFrameTime = PrevMaxUndilatedFrameTime;
}
