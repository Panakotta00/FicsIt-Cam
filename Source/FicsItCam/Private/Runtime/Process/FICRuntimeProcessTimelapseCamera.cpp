#include "Runtime/Process/FICRuntimeProcessTimelapseCamera.h"

#include "FGPlayerController.h"
#include "FICSubsystem.h"
#include "Runtime/FICCaptureCamera.h"

void UFICRuntimeProcessTimelapseCamera::Start(AFICRuntimeProcessorCharacter* InCharacter) {
	CaptureCamera = GetWorld()->SpawnActor<AFICCaptureCamera>();
	CameraArgument.InitalizeCaptureCamera(CaptureCamera);
	Time = 0.0f;
	CaptureStart = FDateTime::Now();
	CaptureIncrement = 0;
}

void UFICRuntimeProcessTimelapseCamera::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	Time += DeltaSeconds;
	if (Time < SecondsPerFrame) return;
	Time -= SecondsPerFrame;
	
	AFGPlayerController* Controller = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
	//if (Character) Character->SetThirdPersonMode();

	CameraArgument.UpdateCameraSettings(CaptureCamera);
	CaptureCamera->UpdateCaptureWithCameraData();
	
	CaptureCamera->CaptureComponent->CaptureSceneDeferred();

	FString FSP;
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	if (FSP.IsEmpty()) {
		FSP = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/") TEXT("SaveGames/"));
	}
	FSP = FPaths::Combine(FSP, TEXT("FicsItCam/"), CameraArgument.GetSimpleName());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*FSP)) PlatformFile.CreateDirectoryTree(*FSP);
	FSP = FPaths::Combine(FSP, FString::Printf(TEXT("%s-%i.jpg"), *CaptureStart.ToString(), CaptureIncrement));

	AFICSubsystem::GetFICSubsystem(this)->SaveRenderTargetAsJPG(FSP, MakeShared<FFICRenderTarget_Raw>(CaptureCamera->RenderTarget->GameThread_GetRenderTargetResource()));

	++CaptureIncrement;

	//if (Character) Character->SetFirstPersonMode();
}

void UFICRuntimeProcessTimelapseCamera::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	if (CaptureCamera) CaptureCamera->Destroy();
}
