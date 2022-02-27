#include "Runtime/Process/FICRuntimeProcessCameraFeed.h"

#include "FGCharacterPlayer.h"
#include "FGCineCameraComponent.h"
#include "FICSubsystem.h"
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

	Camera->CopyCameraData(Cast<UCameraComponent>(Cast<AFGCharacterPlayer>(GetWorld()->GetFirstPlayerController()->GetCharacter())->GetComponentByClass(UCameraComponent::StaticClass())));
}

void UFICRuntimeProcessCameraFeed::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	Camera->CopyCameraData(Camera->Camera);
	CameraArgument.UpdateCameraSettings(Camera);
	/*if (CameraArgument.CameraReference.IsAnimated()) {
	} else {
//		FSlateApplication::Get().GetGameViewport()->GetViewportInterface().Pin()->GetViewportRenderTargetTexture()-> 
	}*/
}

void UFICRuntimeProcessCameraFeed::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	SaveWindowSettings();
	if (Window) {
		Window->DestroyWindowImmediately();
		Window->SetContent(SNew(SBox));
		Window.Reset();
	}
	Camera->CaptureComponent->bCaptureEveryFrame = false;
	Camera->Destroy();
	Camera = nullptr;
	Brush = FSlateImageBrush("CameraFeed", FVector2D(1,1));
}
