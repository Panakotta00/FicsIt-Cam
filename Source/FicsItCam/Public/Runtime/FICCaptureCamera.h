#pragma once

#include "FICCaptureCamera.generated.h"

UCLASS()
class AFICCaptureCamera : public AActor {
	GENERATED_BODY()
public:
	UPROPERTY()
	USceneCaptureComponent2D* CaptureComponent;

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;

	AFICCaptureCamera();
};