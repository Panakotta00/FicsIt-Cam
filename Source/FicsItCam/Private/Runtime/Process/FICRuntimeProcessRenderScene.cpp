#include "Runtime/Process/FICRuntimeProcessRenderScene.h"

#include "FICSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/FICCaptureCamera.h"

void UFICRuntimeProcessRenderScene::Initialize(AFICRuntimeProcessorCharacter* InCharacter) {
	Super::Initialize(InCharacter);

	CaptureCamera = GetWorld()->SpawnActor<AFICCaptureCamera>();
	CaptureCamera->RenderTarget->InitCustomFormat(Scene->ResolutionWidth, Scene->ResolutionHeight, EPixelFormat::PF_B8G8R8A8, true);
	CaptureCamera->RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	CaptureCamera->RenderTarget->TargetGamma = 3.3f;
	CaptureCamera->RenderTarget->UpdateResourceImmediate();
	
	if (Scene->bBulletTime) InCharacter->SetTimeDilation(0);

	FrameProgress = Scene->AnimationRange.Begin;
}

void UFICRuntimeProcessRenderScene::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	double Start = FPlatformTime::Seconds();
	if(GetWorld()->IsLevelStreamingRequestPending(GetWorld()->GetFirstPlayerController())) return;

	Progress = (float)FrameProgress / (float)Scene->FPS;
	Super::Tick(InCharacter, DeltaSeconds);
	if (!CaptureCamera) return;

	// If no Bullet Time adjust game time dilation to fit render speed
	if (!Scene->bBulletTime) InCharacter->SetTimeDilation(1.0f/Scene->FPS/DeltaSeconds);
	
	CaptureCamera->CopyCameraData(InCharacter->Camera);

	CaptureCamera->CaptureComponent->CaptureScene();

	// Save Scene in Image
	FString FSP;
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	if (FSP.IsEmpty()) {
		FSP = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/") TEXT("SaveGames/") TEXT("FicsItCam/"), Scene->SceneName);
	}
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*FSP)) PlatformFile.CreateDirectoryTree(*FSP);
	FSP = FPaths::Combine(FSP, FString::FromInt(FrameProgress) + TEXT(".jpeg"));
	AFICSubsystem::GetFICSubsystem(this)->SaveRenderTargetAsJPG(FSP, CaptureCamera->RenderTarget);
	
	++FrameProgress;
}

void UFICRuntimeProcessRenderScene::Shutdown(AFICRuntimeProcessorCharacter* InCharacter) {
	Super::Shutdown(InCharacter);

	CaptureCamera->Destroy();
	CaptureCamera = nullptr;
}
