#include "Runtime/Process/FICRuntimeProcessTimelapseCamera.h"

#include "FGPlayerController.h"
#include "FICSubsystem.h"
#include "Runtime/FICCaptureCamera.h"

void UFICRuntimeProcessTimelapseCamera::Start(AFICRuntimeProcessorCharacter* InCharacter) {
	CaptureCamera = GetWorld()->SpawnActor<AFICCaptureCamera>();
	CameraArgument.InitalizeCaptureCamera(CaptureCamera);
	Time = 0.0f;
	
	FString FSP;
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	if (FSP.IsEmpty()) {
		FSP = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/") TEXT("SaveGames/"));
	}
	FSP = FPaths::Combine(FSP, TEXT("FicsItCam/"), CameraArgument.GetSimpleName());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*FSP)) PlatformFile.CreateDirectoryTree(*FSP);

	Exporter = MakeShared<FSequenceImageExporter>(FSP, FIntPoint(CaptureCamera->RenderTarget->SizeX, CaptureCamera->RenderTarget->SizeY));
	Exporter->Init();
}

void UFICRuntimeProcessTimelapseCamera::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	Time += DeltaSeconds;
	if (Time < SecondsPerFrame) return;
	Time -= SecondsPerFrame;
	
	AFGPlayerController* Controller = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
	//AFGCharacterPlayer* Character = Cast<AFGCharacterPlayer>(Controller->GetCharacter());
	//if (Character) Character->SetThirdPersonMode();
	// TODO: Fix character and creature animation in capture

	CameraArgument.UpdateCameraSettings(CaptureCamera);
	CaptureCamera->UpdateCaptureWithCameraData();
	
	CaptureCamera->CaptureComponent->CaptureScene();
	
	AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(Exporter.ToSharedRef(), MakeShared<FFICRenderTarget_Raw>(CaptureCamera->RenderTarget->GameThread_GetRenderTargetResource()));

	//if (Character) Character->SetFirstPersonMode();
}

void UFICRuntimeProcessTimelapseCamera::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	Exporter->Finish();
	if (CaptureCamera) CaptureCamera->Destroy();
}
