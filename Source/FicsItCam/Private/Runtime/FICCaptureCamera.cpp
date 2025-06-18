#include "Runtime/FICCaptureCamera.h"

#include "CineCameraComponent.h"
#include "FGGameViewportClient.h"
#include "FGSettings.h"
#include "Blueprint/GameViewportSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"
#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Settings/FGUserSetting.h"

AFICCaptureCamera::AFICCaptureCamera() {
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));

	StreamingSource = CreateDefaultSubobject<UWorldPartitionStreamingSourceComponent>(TEXT("StreamingSource"));
	
	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	CaptureComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	RenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("RenderTarget"));
	RenderTarget->TargetGamma = 1;
	RenderTarget->InitCustomFormat(100, 100, PF_R8G8B8A8, false);
	RenderTarget->bGPUSharedFlag = true;
	CaptureComponent->TextureTarget = RenderTarget;

	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
	CaptureComponent->bUseRayTracingIfEnabled = true;
	CaptureComponent->bAlwaysPersistRenderingState = true;
	//CaptureComponent->PrimitiveRenderMode;
	//CaptureComponent->CaptureSortPriority;
	//CaptureComponent->DetailMode = EDetailMode::DM_Epic;
	//CaptureComponent->MaxViewDistanceOverride;
	//CaptureComponent->CachedLevelCollection;

	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	//CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	if (GEngine) CaptureComponent->ShowFlags = *GEngine->GameViewport->GetEngineShowFlags();
	
	// Kinda performance intense
	CaptureComponent->LODDistanceFactor = 0.01;
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
}

void AFICCaptureCamera::SetCamera(bool bEnabled, bool bCinematic) {
	if (Camera) {
		Camera->DestroyComponent();
		Camera = nullptr;
	}
	if (bEnabled) {
		if (bCinematic) {
			Camera = NewObject<UCineCameraComponent>(this);
		} else {
			Camera = NewObject<UCameraComponent>(this);
		}
		Camera->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void AFICCaptureCamera::UpdateCaptureWithCameraData(UCameraComponent* InCamera) {
	if (!InCamera) InCamera = Camera;
	
	SetActorLocation(InCamera->GetComponentLocation());
	SetActorRotation(InCamera->GetComponentRotation());
	
	FMinimalViewInfo ViewInfo;
	InCamera->GetCameraView(0, ViewInfo);
	CaptureComponent->SetCameraView(ViewInfo);

	CaptureComponent->PostProcessSettings.AutoExposureSpeedDown = 10000.0f;
	CaptureComponent->PostProcessSettings.AutoExposureSpeedUp = 10000.0f;
	CaptureComponent->PostProcessSettings.bOverride_AutoExposureSpeedDown = true;
	CaptureComponent->PostProcessSettings.bOverride_AutoExposureSpeedUp = true;
	CaptureComponent->PostProcessBlendWeight = 1.0f;
}
