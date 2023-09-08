#pragma once

#include "FICRuntimeProcess.h"
#include "Runtime/FICCameraArgument.h"
#include "Util/FICProceduralTexture.h"
#include "FICRuntimeProcessTimelapseCamera.generated.h"

UCLASS()
class UFICRuntimeProcessTimelapseCamera : public UFICRuntimeProcess, public IFGSaveInterface {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FFICTextureUpdateDelegate OnPreviewUpdate;
	
	UPROPERTY(SaveGame, BlueprintReadWrite)
	FFICCameraArgument CameraArgument;

	UPROPERTY(BlueprintReadOnly)
	AFICCaptureCamera* CaptureCamera = nullptr;

	UPROPERTY(SaveGame, BlueprintReadWrite, meta=(ExposeOnSpawn=true))
	float SecondsPerFrame = 1.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	UFICProceduralTexture* PreviewTexture = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float Time = 0.0f;

	TSharedPtr<FSequenceExporter> Exporter;

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	// End IFGSaveInterface
	
	// Begin UFICRuntimeProcess
	virtual void Start(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) override;
	virtual void Stop(AFICRuntimeProcessorCharacter* InCharacter) override;
	// End UFICRuntimeProcess

	UFUNCTION(BlueprintCallable)
	UTexture* GetPreviewTexture();

	UFUNCTION()
	void OnTextureUpdate();
};
