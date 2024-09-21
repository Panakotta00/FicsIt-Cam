#include "Runtime/Process/FICRuntimeProcessCameraFeed.h"

#include "FGCharacterPlayer.h"
#include "FGCineCameraComponent.h"
#include "FICSubsystem.h"
#include "SBox.h"
#include "SlateApplication.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/FICCaptureCamera.h"
#include "Slate/SceneViewport.h"
#include "Slate/SlateTextures.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SViewport.h"

class FFICFeedView : public ISlateViewport
{
public:
	FFICFeedView(FTexture2DRHIRef RenderTargetTexture, FIntPoint InSize)
		: RenderTarget( new FSlateRenderTargetRHI(RenderTargetTexture, InSize.X, InSize.Y))
		, Size(InSize) {
		BeginInitResource(RenderTarget);
	}

	~FFICFeedView() {
		ReleaseResourceAndFlush(RenderTarget);
		delete RenderTarget;
	}

	// Begin ISlateViewport
	virtual FIntPoint GetSize() const override {
		return Size;
	}

	virtual FSlateShaderResource* GetViewportRenderTargetTexture() const override {
		return RenderTarget;
	}

	virtual bool RequiresVsync() const override {
		return false;
	}
	// End ISlateViewport

private:
	FSlateRenderTargetRHI* RenderTarget;
	FIntPoint Size;
};

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
	View = MakeShared<FFICFeedView>(Camera->RenderTarget->GameThread_GetRenderTargetResource()->GetRenderTargetTexture(), Size);

	Window = SNew(SWindow)[
		SNew(SViewport)
		.ViewportInterface(View.ToSharedRef())
		.ForceVolatile(true)
		.IsEnabled(true)
		.RenderOpacity(1.0)
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

	Camera->CaptureComponent->bCaptureEveryFrame = true;

	Camera->UpdateCaptureWithCameraData(Cast<UCameraComponent>(Cast<AFGCharacterPlayer>(GetWorld()->GetFirstPlayerController()->GetCharacter())->GetComponentByClass(UCameraComponent::StaticClass())));

	OnPreviewUpdate.Broadcast();
}

void UFICRuntimeProcessCameraFeed::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	CameraArgument.UpdateCameraSettings(Camera);
	Camera->UpdateCaptureWithCameraData();
	/*if (CameraArgument.CameraReference.IsAnimated()) {
	} else {
//		FSlateApplication::Get().GetGameViewport()->GetViewportInterface().Pin()->GetViewportRenderTargetTexture()-> 
	}*/
}

void UFICRuntimeProcessCameraFeed::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	if (PreviewTexture == nullptr) {
		PreviewTexture = NewObject<UFICProceduralTexture>(this);
		PreviewTexture->OnTextureUpdate.AddDynamic(this, &UFICRuntimeProcessCameraFeed::OnTextureUpdate);
	}

	TSharedRef<FSequenceExporterProceduralTexture> TextureExporter = MakeShared<FSequenceExporterProceduralTexture>(PreviewTexture);
	AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(TextureExporter, MakeShared<FFICRenderTarget_Raw>(Camera->RenderTarget->GameThread_GetRenderTargetResource()));
	
	SaveWindowSettings();
	if (Window) {
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
