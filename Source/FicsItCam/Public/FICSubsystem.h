#pragma once


#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "IImageWrapper.h"
#include "FICSubsystem.generated.h"

class UFICRuntimeProcess;
class AFICScene;
class AFICTimelapseCamera;
class UFICEditorContext;
class AFICAnimation;
class AFICRuntimeProcessorCharacter;
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
	TQueue<TSharedPtr<FFICRenderRequest>> RenderRequestQueue;

	UPROPERTY()
	UFICRuntimeProcess* ActiveRuntimeProcess = nullptr;
	
	UPROPERTY()
	AFICRuntimeProcessorCharacter* RuntimeProcessorCharacter = nullptr;
	UPROPERTY()
	ACharacter* OriginalPlayerCharacter = nullptr;
	
public:
	UPROPERTY(BlueprintReadWrite, SaveGame, Category="FicsIt-Cam")
	TMap<FString, AFICTimelapseCamera*> TimelapseCameras;
	
	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam", meta=(WorldContext = "WorldContextObject"))
	static AFICSubsystem* GetFICSubsystem(UObject* WorldContext);
	
	AFICSubsystem();
	
	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
	
	void StartProcess(UFICRuntimeProcess* InProcess);
	void StopProcess();
	UFICRuntimeProcess* GetActiveProcess() { return ActiveRuntimeProcess; }

	AFICRuntimeProcessorCharacter* GetRuntimeProcessorCharacter() { return RuntimeProcessorCharacter; }
	
	void SaveRenderTargetAsJPG(const FString& FilePath, UTextureRenderTarget2D* RenderTarget);

	AFICScene* FindSceneByName(const FString& InSceneName);
};
