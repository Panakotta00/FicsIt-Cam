#include "Runtime/Process/FICRuntimeProcessCameraFeed.h"

#include "FICSubsystem.h"
#include "Command/CommandSender.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/FICCaptureCamera.h"
#include "Widgets/Images/SImage.h"

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

void UFICRuntimeProcessCameraFeed::Start(AFICRuntimeProcessorCharacter* InCharacter) {
	Camera = GetWorld()->SpawnActor<AFICCaptureCamera>();
	CameraArgument.InitalizeCaptureCamera(Camera);

	Brush = FSlateImageBrush(Camera->RenderTarget, FVector2D(Camera->RenderTarget->SizeX, Camera->RenderTarget->SizeY));
	FVector2D Resolution = CameraArgument.GetResolution(this);
	Window = SNew(SWindow)[
		SNew(SImage)
		.Image(&Brush)
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
}

void UFICRuntimeProcessCameraFeed::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	CameraArgument.UpdateCameraSettings(Camera);
	Camera->CopyCameraData(Camera->Camera);
}

void UFICRuntimeProcessCameraFeed::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	SaveWindowSettings();
	if (Window) Window->DestroyWindowImmediately();
	Camera->CaptureComponent->bCaptureEveryFrame = false;
	Camera->Destroy();
	Camera = nullptr;
}
