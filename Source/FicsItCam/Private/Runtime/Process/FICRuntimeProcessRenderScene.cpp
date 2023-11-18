#include "Runtime/Process/FICRuntimeProcessRenderScene.h"

#include "AudioDevice.h"
#include "CanvasTypes.h"
#include "EngineModule.h"
#include "FICSubsystem.h"
#include "IImageWrapperModule.h"
#include "Algo/Accumulate.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Editor/FICEditorSubsystem.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Exporter/FICFfmpegExporter.h"
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
	//FAudioDeviceManager::Get()->GetActiveAudioDevice().GetAudioDevice()->) // TODO: Audio Capture?
	FAudioThread::StopAudioThread();
	
	FViewportClient* ViewportClient = GetWorld()->GetGameViewport();
	DummyViewport = MakeShared<FFICRendererViewport>(ViewportClient, ExportSettings.Resolution.X, ExportSettings.Resolution.Y);
	
	Exporter = UFICExporterDescriptor::CreateExporter(ExportSettings.Exporter, ExportSettings);
	//Exporter = MakeShared<FSequenceImageExporter>(Path, FIntPoint(Scene->ResolutionWidth, Scene->ResolutionHeight));
	Exporter->Init();

	GEngine->GameViewport->AddViewportWidgetContent(
		SAssignNew(Overlay, SVerticalBox)
		+SVerticalBox::Slot()
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
}

void UFICRuntimeProcessRenderScene::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	if(GetWorld()->IsLevelStreamingRequestPending(GetWorld()->GetFirstPlayerController())) return;

	ETAStatistics.Insert(GetWorld()->DeltaRealTimeSeconds, 0);
	while (ETAStatistics.Num() > 60) ETAStatistics.Pop();

	Progress = (float)FrameProgress / (float)Scene->FPS;
	Super::Tick(InCharacter, DeltaSeconds);

	// Capture Image
	FlushRenderingCommands();
	
	UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
	FCanvas Canvas(DummyViewport.Get(), NULL, ViewportClient->GetWorld(), ViewportClient->GetWorld()->FeatureLevel);
	ViewportClient->Draw(DummyViewport.Get(), &Canvas);
	Canvas.Flush_GameThread();

	// Store Image
	AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(Exporter.ToSharedRef(), DummyViewport.ToSharedRef());
	
	++FrameProgress;
}

void UFICRuntimeProcessRenderScene::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	Super::Stop(InCharacter);

	if (Overlay) GEngine->GameViewport->RemoveViewportWidgetContent(Overlay.ToSharedRef());

	Exporter->Finish();
	
	auto* Settings = GetWorld()->GetWorldSettings();
	Settings->MinUndilatedFrameTime = PrevMinUndilatedFrameTime;
	Settings->MaxUndilatedFrameTime = PrevMaxUndilatedFrameTime;
}

UFICRuntimeProcessRenderScene* UFICRuntimeProcessRenderScene::StartRenderScene(AFICScene* InScene, const FFICExportSettings& Settings) {
	AFICEditorSubsystem* EditSubSys = AFICEditorSubsystem::GetFICEditorSubsystem(InScene);
	if (InScene->IsSceneAlreadyInUse()) return nullptr;
	AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InScene);
	UFICRuntimeProcessRenderScene* Process = NewObject<UFICRuntimeProcessRenderScene>(SubSys);
	Process->Scene = InScene;
	Process->ExportSettings = Settings;
	Process->RenderSettings = static_cast<FFICRenderSettings>(Settings);
	if (SubSys->CreateRuntimeProcess(AFICScene::GetSceneProcessKey(InScene->SceneName), Process, true)) {
		return Process;
	} else {
		return nullptr;
	}
}
