#include "Runtime/Process/FICRuntimeProcessRenderScene.h"

#include "AkAudioModule.h"
#include "AudioDevice.h"
#include "CanvasTypes.h"
#include "EngineModule.h"
#include "Rendering/FICDummyViewport.h"
#include "FicsItCamModule.h"
#include "FICSubsystem.h"
#include "IImageWrapperModule.h"
#include "HAL/PlatformFileManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SCanvas.h"
#include "SlateMaterialBrush.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Algo/Accumulate.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Editor/FICEditorSubsystem.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/WorldSettings.h"
#include "Widgets/SViewport.h"
#include "Widgets/Notifications/SProgressBar.h"

UE_DISABLE_OPTIMIZATION_SHIP
static void CaptureCallback(AkAudioBuffer& in_CaptureBuffer, AkOutputDeviceID /*in_idOutput*/, void* in_pCookie) {
	auto self = reinterpret_cast<UFICRuntimeProcessRenderScene*>(in_pCookie);

	if (!self->bStarted || self->bSkipAudio) return;

	size_t uSampleCount = static_cast<size_t>(in_CaptureBuffer.uValidFrames);

	if (!uSampleCount || !in_CaptureBuffer.GetInterleavedData())
		return;

#if PLATFORM_WINDOWS
	StaticCastSharedPtr<FSequenceMP4Exporter>(self->Exporter)->AddAudioFrame((float*)in_CaptureBuffer.GetInterleavedData(), in_CaptureBuffer.uValidFrames);
#endif
}
UE_ENABLE_OPTIMIZATION_SHIP

UFICRuntimeProcessRenderScene::~UFICRuntimeProcessRenderScene() {
	if (bStarted) Stop(nullptr);
}

void UFICRuntimeProcessRenderScene::Start(AFICRuntimeProcessorCharacter* InCharacter) {
	Super::Start(InCharacter);

	WwiseSoundEngineAPI = IWwiseSoundEngineAPI::Get();

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

	AudioSampleRate = WwiseSoundEngineAPI->GetSampleRate();

	// Audio Capture
	{
		//FAudioDeviceManager::Get()->GetActiveAudioDevice().GetAudioDevice()->) // TODO: Audio Capture?
		FAudioThread::StopAudioThread();

		WwiseSoundEngineAPI->RenderAudio(false);

		AKRESULT ret = WwiseSoundEngineAPI->SetOfflineRendering(true);
		if (ret != AK_Success) {
			UE_LOG(LogFicsItCam, Warning, TEXT("Unable to set offline rendering: %i"), ret);
		}

		WwiseSoundEngineAPI->RenderAudio(false);

		// RenderAudio() wont generate more audio samples
		WwiseSoundEngineAPI->SetOfflineRenderingFrameTime(TNumericLimits<AkReal32>::Min());

		// Flush both messages from above
		WwiseSoundEngineAPI->RenderAudio();

		/*AkChannelConfig channelConfig;
		channelConfig.SetStandard(AK_SPEAKER_SETUP_STEREO);

		AkOutputSettings newSettings;
		newSettings.audioDeviceShareset = AK_INVALID_UNIQUE_ID;
		newSettings.idDevice = 0;
		newSettings.ePanningRule = AkPanningRule_Speakers;
		newSettings.channelConfig = channelConfig;

		WwiseSoundEngineAPI->ReplaceOutput(newSettings, 0, &m_defaultOutputDeviceId);*/
		//WwiseSoundEngineAPI->RenderAudio(true);

		ret = WwiseSoundEngineAPI->RegisterCaptureCallback(&CaptureCallback, 0, this);
		if (ret != AK_Success) {
			UE_LOG(LogFicsItCam, Warning, TEXT("Unable to register capture callback: %i"), ret);
		}
	}

	FViewportClient* ViewportClient = GetWorld()->GetGameViewport();
	DummyViewport = MakeShared<FFICRendererViewport>(ViewportClient, Scene->ResolutionWidth, Scene->ResolutionHeight);
	FlushRenderingCommands();
	DummyViewportInterface = MakeShared<FFICDummyViewport>(DummyViewport->GetRenderTarget(), FIntPoint(Scene->ResolutionWidth, Scene->ResolutionHeight));

	// Create Save Path
	FString FSP;
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	if (FSP.IsEmpty()) {
		FSP = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/") TEXT("SaveGames/") TEXT("FicsItCam/"), Scene->SceneName);
	}
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*FSP)) PlatformFile.CreateDirectoryTree(*FSP);
	
	Path = FPaths::Combine(FSP, FDateTime::Now().ToString() + TEXT(".mp4"));

#if PLATFORM_WINDOWS
	Exporter = MakeShared<FSequenceMP4Exporter>(FIntPoint(Scene->ResolutionWidth, Scene->ResolutionHeight), Scene->FPS, Path, AudioSampleRate);
#else
	Exporter = MakeShared<FSequenceImageExporter>(Path, FIntPoint(Scene->ResolutionWidth, Scene->ResolutionHeight));
#endif
	Exporter->Init();

	GEngine->GameViewport->AddViewportWidgetContent(
		SAssignNew(Overlay, SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(1.0)
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black)
			.Padding(0.f)
			.Content()[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::Both)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Content()[
					SNew(SViewport)
					.ViewportInterface(DummyViewportInterface)
					.ForceVolatile(true)
					.IsEnabled(true)
					.RenderOpacity(1.0)
					.EnableGammaCorrection(false)
					.ViewportSize(FVector2D(Scene->ResolutionWidth, Scene->ResolutionHeight))
				]
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Fill)[
			SNew(SOverlay)
			+SOverlay::Slot()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)[
				SNew(SProgressBar)
				.Percent_Lambda([this]() {
					return (float)(FrameProgress - Scene->AnimationRange.Begin) / (float)Scene->AnimationRange.Length();
				})
			]
			+SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)[
				SNew(STextBlock)
				.Text_Lambda([this]() {
					int64 CurrentFrame = FrameProgress - Scene->AnimationRange.Begin;
					int64 FrameCount = Scene->AnimationRange.Length();
					float Percent = (float)CurrentFrame / (float)FrameCount;
					float ETASec = (float)(FrameCount - CurrentFrame) * (Algo::Accumulate(ETAStatistics, 0.0f)/ETAStatistics.Num());
					FString ETA = UFGBlueprintFunctionLibrary::SecondsToTimeString(ETASec);
					return FText::FromString(FString::Printf(TEXT("%.1f%% [%lld/%lld] - ETA: %s"), Percent*100, CurrentFrame, FrameCount, *ETA));
				})
			]
		]
	);

	bSkipAudio = true;
	bStarted = true;

	GEngine->GameViewport->Viewport->SetGameRenderingEnabled(false);
}

void UFICRuntimeProcessRenderScene::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	if (!bStarted) return;

	// TODO: Evaluate if something similar / an replacement could be used
	/*if(GetWorld()->IsLevelStreamingRequestPending(GetWorld()->GetFirstPlayerController())) {
		UE_LOG(LogFicsItCam, Warning, TEXT("Level streaming is pending, skipping frame"));
	}*/

	ETAStatistics.Insert(GetWorld()->DeltaRealTimeSeconds, 0);
	while (ETAStatistics.Num() > 60) ETAStatistics.Pop();

	Progress = (float)FrameProgress / (float)Scene->FPS;
	Super::Tick(InCharacter, DeltaSeconds);

	// Capture Image
	FlushRenderingCommands();
	
	UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
	FCanvas Canvas(DummyViewport.Get(), NULL, ViewportClient->GetWorld(), ViewportClient->GetWorld()->GetFeatureLevel(), FCanvas::CDM_DeferDrawing, ViewportClient->ShouldDPIScaleSceneCanvas() ? ViewportClient->GetDPIScale() : 1.0f);
	ViewportClient->Draw(DummyViewport.Get(), &Canvas);
	Canvas.Flush_GameThread();

	// Store Image
	AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(Exporter.ToSharedRef(), DummyViewport.ToSharedRef());

	FlushRenderingCommands();

	bSkipAudio = false;

	WwiseSoundEngineAPI->SetOfflineRenderingFrameTime(1.0 / Scene->FPS);
	WwiseSoundEngineAPI->RenderAudio();
	WwiseSoundEngineAPI->SetOfflineRenderingFrameTime(TNumericLimits<AkReal32>::Min()); // TODO: Some proper restriction would be better (also above in Start)

	++FrameProgress;
}

void UFICRuntimeProcessRenderScene::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	Super::Stop(InCharacter);

	if (!bStarted) return;

	AFICSubsystem::GetFICSubsystem(this)->WaitForAllExports();

	bStarted = false;

	GEngine->GameViewport->Viewport->SetGameRenderingEnabled(true);

	WwiseSoundEngineAPI->UnregisterCaptureCallback(&CaptureCallback, 0, this);
	WwiseSoundEngineAPI->SetOfflineRendering(false);
	WwiseSoundEngineAPI->RenderAudio(false);
	WwiseSoundEngineAPI->RenderAudio();

	if (Overlay) GEngine->GameViewport->RemoveViewportWidgetContent(Overlay.ToSharedRef());

	DummyViewportInterface.Reset();
	Exporter->Finish();

	FAudioThread::StartAudioThread();
	
	auto* Settings = GetWorld()->GetWorldSettings();
	Settings->MinUndilatedFrameTime = PrevMinUndilatedFrameTime;
	Settings->MaxUndilatedFrameTime = PrevMaxUndilatedFrameTime;
}

UFICRuntimeProcessRenderScene* UFICRuntimeProcessRenderScene::StartRenderScene(AFICScene* InScene) {
	AFICEditorSubsystem* EditSubSys = AFICEditorSubsystem::GetFICEditorSubsystem(InScene);
	if (InScene->IsSceneAlreadyInUse()) return nullptr;
	AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InScene);
	UFICRuntimeProcessRenderScene* Process = NewObject<UFICRuntimeProcessRenderScene>(SubSys);
	Process->Scene = InScene;
	if (SubSys->CreateRuntimeProcess(AFICScene::GetSceneProcessKey(InScene->SceneName), Process, true)) {
		return Process;
	} else {
		return nullptr;
	}
}
