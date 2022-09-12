#include "Runtime/FICCaptureCamera.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

AFICCaptureCamera::AFICCaptureCamera() {
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));
	
	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	CaptureComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	RenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("RenderTarget"));
	RenderTarget->InitCustomFormat(100, 100, PF_R8G8B8A8, false);
	RenderTarget->bGPUSharedFlag = true;
	CaptureComponent->TextureTarget = RenderTarget;

	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;

	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	// Kinda performance intense
//	CaptureComponent->DetailMode = DM_MAX;
//	CaptureComponent->LODDistanceFactor = 0.01;
//	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	CaptureComponent->bUseRayTracingIfEnabled = true;
	CaptureComponent->ShowFlags.SetTemporalAA(true);
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
		Camera->PostProcessSettings.VignetteIntensity = 0;
		Camera->PostProcessSettings.DepthOfFieldVignetteSize = 0;
	}
}

void AFICCaptureCamera::CopyCameraData(UCameraComponent* InCamera) {
	SetActorLocation(InCamera->GetComponentLocation());
	SetActorRotation(InCamera->GetComponentRotation());
	
	FMinimalViewInfo ViewInfo;
	InCamera->GetCameraView(0, ViewInfo);
	CaptureComponent->SetCameraView(ViewInfo);
	//FWeightedBlendables Blendables = CaptureComponent->PostProcessSettings.WeightedBlendables;
	CaptureComponent->PostProcessSettings = InCamera->PostProcessSettings;
	//CaptureComponent->PostProcessSettings.WeightedBlendables = Blendables;
	CaptureComponent->PostProcessBlendWeight = InCamera->PostProcessBlendWeight;
	CaptureComponent->MaxViewDistanceOverride = TNumericLimits<float>::Max();
}
