#pragma once
#include "CineCameraComponent.h"

#include "FICCaptureCamera.generated.h"

class UCameraComponent;
UCLASS()
class AFICCaptureCamera : public AActor {
	GENERATED_BODY()
public:
	UPROPERTY()
	USceneCaptureComponent2D* CaptureComponent;

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;

	UPROPERTY()
	UCameraComponent* Camera;

	AFICCaptureCamera();
	
	void SetCamera(bool bEnabled, bool bCinematic);
	void UpdateCaptureWithCameraData(UCameraComponent* Camera = nullptr);
};