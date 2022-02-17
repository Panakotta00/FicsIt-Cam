#pragma once


#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "IImageWrapper.h"
#include "FICSubsystem.generated.h"

class AFICTimelapseCamera;
class UFICEditorContext;
class AFICAnimation;
class AFICCameraCharacter;
USTRUCT()
struct FFICRenderRequest{
	GENERATED_BODY()

	TArray<FColor> Image;
	FRenderCommandFence RenderFence;
	FString Path;

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget = nullptr;
};

class FFICAsyncImageCompressAndSave : public FNonAbandonableTask{
public:
	FFICAsyncImageCompressAndSave(TSharedPtr<IImageWrapper> Image, FString Path);
	~FFICAsyncImageCompressAndSave();

	// Required by UE4!
	FORCEINLINE TStatId GetStatId() const{
		RETURN_QUICK_DECLARE_CYCLE_STAT(AsyncSaveImageToDiskTask, STATGROUP_ThreadPoolAsyncTasks);
	}

protected:
	TSharedPtr<IImageWrapper> Image;
	FString Path = "";

public:
	void DoWork();
};

UCLASS()
class AFICSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
private:
	UPROPERTY()
	AFICCameraCharacter* Camera = nullptr;

	UPROPERTY()
	AFICAnimation* ActiveAnimation = nullptr;

	TQueue<TSharedPtr<FFICRenderRequest>> RenderRequestQueue;
	
public:
	UPROPERTY(BlueprintReadWrite, SaveGame, Category="FicsIt-Cam")
	TMap<FString, AFICAnimation*> StoredAnimations;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="FicsIt-Cam")
	TMap<FString, AFICTimelapseCamera*> TimelapseCameras;
	
	UPROPERTY()
	TSet<AFICAnimation*> VisibleAnimations;

	AFICSubsystem();
	
	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam", meta=(WorldContext = "WorldContextObject"))
	static AFICSubsystem* GetFICSubsystem(UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam")
	void PlayAnimation(AFICAnimation* Path, bool bDoRendering = false);
	
	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam")
	void StopAnimation();
	
	void AddVisibleAnimation(AFICAnimation* Path);

	void CreateCamera();
	
	void SaveRenderTargetAsJPG(const FString& FilePath, UTextureRenderTarget2D* RenderTarget);
};
