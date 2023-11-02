#pragma once

#include "FICRuntimeProcess.h"
#include "Runtime/FICCameraArgument.h"
#include "Brushes/SlateImageBrush.h"
#include "Runtime/FICCameraReference.h"

#include "FICRuntimeProcessCameraFeed.generated.h"

class FFICFeedView;

UCLASS()
class UFICRuntimeProcessCameraFeed : public UFICRuntimeProcess, public IFGSaveInterface {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame, BlueprintReadWrite)
	FFICCameraArgument CameraArgument;
	
	UPROPERTY(BlueprintReadOnly)
	AFICCaptureCamera* Camera = nullptr;

	UPROPERTY(SaveGame)
	UFICProceduralTexture* PreviewTexture = nullptr;

	FSlateImageBrush Brush = FSlateImageBrush("CameraFeed", FVector2D(1,1));
	TSharedPtr<SWindow> Window;
	TSharedPtr<FFICFeedView> View;

	UPROPERTY(SaveGame)
	FVector2D WindowLocation;
	UPROPERTY(SaveGame)
	FVector2D WindowSize;
	UPROPERTY(SaveGame)
	bool bEverSaved = false;

	void SaveWindowSettings();
	void LoadWindowSettings();
	
public:
	UPROPERTY(BlueprintAssignable)
	FFICTextureUpdateDelegate OnPreviewUpdate;

	// Begin UObject
	virtual void PostInitProperties() override;
	// End UObject
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	// End IFGSaveInterface
	
	// Begin UFICRuntimeProcess
	virtual void Start(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) override;
	virtual void Stop(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual bool IsPersistent() override { return true; }
	// End UFICRuntimeProcess

	UFUNCTION(BlueprintCallable)
	UTexture* GetPreviewTexture();

	UFUNCTION()
	void OnTextureUpdate();
};