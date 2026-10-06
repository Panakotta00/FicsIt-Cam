#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraTypes.h"
#include "GameFramework/Actor.h"
#include "FICCaptureCamera.generated.h"

class UCameraComponent;

UCLASS()
class AFICCaptureCamera : public AActor {
	GENERATED_BODY()
public:
	UPROPERTY()
	class UWorldPartitionStreamingSourceComponent* StreamingSource;
	
	UPROPERTY(BlueprintReadOnly)
	class USceneCaptureComponent2D* CaptureComponent;

	UPROPERTY(BlueprintReadOnly)
	class UTextureRenderTarget2D* RenderTarget;

	UPROPERTY(BlueprintReadOnly)
	UCameraComponent* Camera;

	AFICCaptureCamera();
	
	void SetCamera(bool bEnabled = true);
	void UpdateCaptureWithCameraData(UCameraComponent* Camera = nullptr);
	void UpdateCaptureWithViewInfo(const FMinimalViewInfo& InViewInfo);
};