#include "Runtime/Process/FICRuntimeProcessCameraFeed.h"

#include "FGCharacterPlayer.h"
#include "FGCineCameraComponent.h"
#include "FICSubsystem.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/FICCaptureCamera.h"
#include "Slate/SlateTextures.h"
#include "Widgets/SViewport.h"

void UFICRuntimeProcessCameraFeed::SaveWindowSettings() {
	WindowLocation = Window->GetPositionInScreen();
	WindowSize = Window->GetSizeInScreen();
	bEverSaved = true;
}

void UFICRuntimeProcessCameraFeed::LoadWindowSettings() {
	if (!bEverSaved) return;
	Window->MoveWindowTo(WindowLocation);
	Window->Resize(WindowSize);
}

void UFICRuntimeProcessCameraFeed::PostInitProperties() {
	Super::PostInitProperties();

	if (PreviewTexture) {
		PreviewTexture->OnTextureUpdate.AddDynamic(this, &UFICRuntimeProcessCameraFeed::OnTextureUpdate);
	}
}

void UFICRuntimeProcessCameraFeed::Start(AFICRuntimeProcessorCharacter* InCharacter) {
	Camera = GetWorld()->SpawnActor<AFICCaptureCamera>();
	CameraArgument.InitalizeCaptureCamera(Camera);

	FlushRenderingCommands();

	FIntPoint Size = FIntPoint(Camera->RenderTarget->SizeX, Camera->RenderTarget->SizeY);
	FVector2D Resolution = CameraArgument.GetResolution(this);

	FString ExportPath = CameraArgument.CameraReference.GetData();
	if (!ExportPath.IsEmpty()) {
#if PLATFORM_WINDOWS
		Exporter = MakeShared<FSequenceMP4Exporter>(Size, 60, ExportPath, 0);
#endif
	} else {
		View = MakeShared<FFICDummyViewport>(Camera->RenderTarget->GameThread_GetRenderTargetResource()->GetRenderTargetTexture(), Size);

		Window = SNew(SWindow)[
			SNew(SViewport)
			.ViewportInterface(View.ToSharedRef())
			.ForceVolatile(true)
			.IsEnabled(true)
			.RenderOpacity(1.0)
			.EnableGammaCorrection(true)
		]
		.Title(FText::FromString(CameraArgument.GetName()))
		.ClientSize(Resolution)
		.UseOSWindowBorder(true)
		.ShouldPreserveAspectRatio(true)
		.MinWidth(100*Resolution.X/Resolution.Y)
		.MinHeight(100);
		Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>&) {
			AFICSubsystem::GetFICSubsystem(this)->StopRuntimeProcess(this);
		}));
		FSlateApplication::Get().AddWindow(Window.ToSharedRef());

		LoadWindowSettings();
	}

	Camera->CaptureComponent->bCaptureEveryFrame = true;

	Camera->UpdateCaptureWithCameraData(Cast<UCameraComponent>(Cast<AFGCharacterPlayer>(GetWorld()->GetFirstPlayerController()->GetCharacter())->GetComponentByClass(UCameraComponent::StaticClass())));

	OnPreviewUpdate.Broadcast();

	Progress = 0.0;

	//Exporter = MakeShared<FSequenceMP4Exporter>(Size, 5, "C:\\Users\\Yannic\\Desktop\\Test.mp4");
	//Exporter = MakeShared<FSequenceImageExporter>(TEXT("C:\\Users\\Yannic\\Desktop\\Test"), Size);
	if (Exporter) if (!Exporter->Init()) Exporter.Reset();
}

void UFICRuntimeProcessCameraFeed::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	CameraArgument.UpdateCameraSettings(Camera);
	Camera->UpdateCaptureWithCameraData();

	if (false) {
		Camera->CaptureComponent->bCaptureEveryFrame = false;
		Camera->CaptureComponent->TextureTarget = nullptr;

		FMinimalViewInfo ViewInfo;
		Camera->Camera->GetCameraView(0, ViewInfo);
		Renderer.Render(Camera->RenderTarget->GameThread_GetRenderTargetResource(), ViewInfo, GetWorld());
	}

	if (Exporter) {
		AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(Exporter.ToSharedRef(), MakeShared<FFICRenderTarget_Raw>(Camera->RenderTarget->GameThread_GetRenderTargetResource()), false, Progress);
		Progress += DeltaSeconds;
	}
}

void UFICRuntimeProcessCameraFeed::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	if (Exporter) Exporter->Finish();

	if (PreviewTexture == nullptr) {
		PreviewTexture = NewObject<UFICProceduralTexture>(this);
		PreviewTexture->OnTextureUpdate.AddDynamic(this, &UFICRuntimeProcessCameraFeed::OnTextureUpdate);
	}

	TSharedRef<FSequenceExporterProceduralTexture> TextureExporter = MakeShared<FSequenceExporterProceduralTexture>(PreviewTexture);
	AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(TextureExporter, MakeShared<FFICRenderTarget_Raw>(Camera->RenderTarget->GameThread_GetRenderTargetResource()));
	
	if (Window) {
		SaveWindowSettings();
		Window->SetContent(SNew(SBox));
		Window->SetOnWindowClosed(FOnWindowClosed());
		Window->RequestDestroyWindow();
		Window.Reset();
	}
	if (View) {
		View.Reset();
	}
	Camera->CaptureComponent->bCaptureEveryFrame = false;
	Camera->Destroy();
	Camera = nullptr;
}

UTexture* UFICRuntimeProcessCameraFeed::GetPreviewTexture() {
	if (Camera) {
		return Camera->RenderTarget;
	} else if (PreviewTexture) {
		return PreviewTexture->GetTexture();
	} else {
		return nullptr;
	}
}

void UFICRuntimeProcessCameraFeed::OnTextureUpdate() {
	OnPreviewUpdate.Broadcast();
}
