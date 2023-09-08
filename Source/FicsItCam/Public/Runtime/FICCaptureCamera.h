#pragma once
#include "CineCameraComponent.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"

#include "FICCaptureCamera.generated.h"

class UCameraComponent;
UCLASS()
class AFICCaptureCamera : public AActor {
	GENERATED_BODY()
public:
	UPROPERTY()
	UWorldPartitionStreamingSourceComponent* StreamingSource;
	
	UPROPERTY(BlueprintReadOnly)
	USceneCaptureComponent2D* CaptureComponent;

	UPROPERTY(BlueprintReadOnly)
	UTextureRenderTarget2D* RenderTarget;

	UPROPERTY(BlueprintReadOnly)
	UCameraComponent* Camera;

	AFICCaptureCamera();
	
	void SetCamera(bool bEnabled, bool bCinematic);
	void UpdateCaptureWithCameraData(UCameraComponent* Camera = nullptr);
};