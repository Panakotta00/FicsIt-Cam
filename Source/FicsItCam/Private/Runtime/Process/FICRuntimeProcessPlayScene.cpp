#include "Runtime/Process/FICRuntimeProcessPlayScene.h"

#include "CineCameraComponent.h"
#include "FICSubsystem.h"
#include "Runtime/FICCaptureCamera.h"

void UFICRuntimeProcessPlayScene::Initialize(AFICRuntimeProcessorCharacter* InCharacter) {
	InCharacter->SetCamera(true, Scene->bUseCinematic);
	if (Scene->bUseCinematic) {
		UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(InCharacter->Camera);
		CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::Manual;
		CineCamera->Filmback.SensorWidth = Scene->SensorDimension.X;
		CineCamera->Filmback.SensorHeight = Scene->SensorDimension.Y;
	} else {
		InCharacter->Camera->SetAspectRatio(Scene->ResolutionHeight / Scene->ResolutionWidth);
	}

	/*
	if (bDoRender) {
		RenderTarget->InitCustomFormat(Animation->ResolutionWidth, Animation->ResolutionHeight, EPixelFormat::PF_B8G8R8A8, true);
		RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		RenderTarget->TargetGamma = 3.3f;
		RenderTarget->UpdateResourceImmediate();
	}

	if (Animation->bBulletTime) SetTimeDilation(0);
	*/
}

void UFICRuntimeProcessPlayScene::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaTime) {

	//double Start = FPlatformTime::Seconds();
	/*if (bDoRender) {
		if(GetWorld()->IsLevelStreamingRequestPending(GetWorld()->GetFirstPlayerController())) return;
		
		Time = Progress;
		Progress += 1;

		if (!Animation->bBulletTime) SetTimeDilation(1.0f/Animation->FPS/DeltaSeconds);
	} else {
	if (bDoRender) {
		FMinimalViewInfo ViewInfo;
		Camera->GetCameraView(0, ViewInfo);
		CaptureComponent->SetCameraView(ViewInfo);
		FWeightedBlendables Blendables = CaptureComponent->PostProcessSettings.WeightedBlendables;
		CaptureComponent->PostProcessSettings = ViewInfo.PostProcessSettings;
		CaptureComponent->PostProcessSettings.WeightedBlendables = Blendables;
		CaptureComponent->PostProcessBlendWeight = Camera->PostProcessBlendWeight;
		CaptureComponent->MaxViewDistanceOverride = TNumericLimits<float>::Max();
		CaptureComponent->CaptureScene();
		
		FString FSP;
		// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
		if (FSP.IsEmpty()) {
			FSP = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/") TEXT("SaveGames/") TEXT("FicsItCam/"), Animation->Name);
		}

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.DirectoryExists(*FSP)) PlatformFile.CreateDirectoryTree(*FSP);

		FSP = FPaths::Combine(FSP, FString::FromInt((int)Progress) + TEXT(".jpeg"));

		AFICSubsystem::GetFICSubsystem(this)->SaveRenderTargetAsJPG(FSP, RenderTarget);
	if (!Animation->bBulletTime)
		Cast<APlayerController>(GetController())->SetPause(false);
		if (Animation->GetEndOfAnimation() < Progress / Animation->FPS) {
			StopAnimation();
		}
	*/
	
	FICFrameFloat Time = Progress * Scene->FPS;
	UFICCamera* Camera = Scene->GetActiveCamera(Time);
	FVector Pos = Camera->Position.Get(Time);
	FRotator Rot = Camera->Rotation.Get(Time);
	float FOV = Camera->FOV.GetValue(Time);
	float Aperture = Camera->Aperture.GetValue(Time);
	float FocusDistance = Camera->FocusDistance.GetValue(Time);

	InCharacter->SetActorLocation(Pos);
	InCharacter->SetActorRotation(Rot);
	InCharacter->GetController()->SetControlRotation(Rot);
	Cast<APlayerController>(InCharacter->GetController())->PlayerCameraManager->UnlockFOV();
	InCharacter->Camera->SetFieldOfView(FOV);
	UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(Camera);
	if (CineCamera) {
		CineCamera->CurrentAperture = Aperture;
		CineCamera->FocusSettings.ManualFocusDistance = FocusDistance;
	}

	if (Time > Scene->AnimationRange.End) AFICSubsystem::GetFICSubsystem(this)->StopProcess();
	Progress += DeltaTime;
}

void UFICRuntimeProcessPlayScene::Shutdown(AFICRuntimeProcessorCharacter* InCharacter) {
}
