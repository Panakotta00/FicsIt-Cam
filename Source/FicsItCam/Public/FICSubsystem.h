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
class UFICCommand;

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

	UPROPERTY(SaveGame)
	TMap<FString, UFICRuntimeProcess*> RuntimeProcesses;
	UPROPERTY(SaveGame)
	TSet<UFICRuntimeProcess*> ActiveRuntimeProcesses;
	
	UPROPERTY()
	AFICRuntimeProcessorCharacter* RuntimeProcessorCharacter = nullptr;
	UPROPERTY()
	ACharacter* OriginalPlayerCharacter = nullptr;

	TMap<TSubclassOf<UFICCommand>, TMap<FString, UFICCommand*>> Commands;
	
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
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual bool ShouldSave_Implementation() const override;
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface

	const TMap<TSubclassOf<UFICCommand>, TMap<FString, UFICCommand*>>& GetCommands() { return Commands; }
	
	bool CreateRuntimeProcess(FString Key, UFICRuntimeProcess* InProcess, bool bStartAutomatically = false);
	bool RemoveRuntimeProcess(UFICRuntimeProcess* Process);
	bool StartRuntimeProcess(UFICRuntimeProcess* Process);
	bool StopRuntimeProcess(UFICRuntimeProcess* Process);
	const TMap<FString, UFICRuntimeProcess*>& GetRuntimeProcesses() { return RuntimeProcesses; }
	const TSet<UFICRuntimeProcess*>& GetActiveRuntimeProcesses() { return ActiveRuntimeProcesses; }
	TMap<FString, UFICRuntimeProcess*> GetActiveRuntimeProcessesMap() {
		TMap<FString, UFICRuntimeProcess*> Map;
		for (const TPair<FString, UFICRuntimeProcess*>& Process : RuntimeProcesses) {
			if (ActiveRuntimeProcesses.Contains(Process.Value)) Map.Add(Process.Key, Process.Value);
		}
		return Map;
	}

	void CreateRuntimeProcessorCharacter(UFICRuntimeProcess* RuntimeProcess);
	void DestoryRuntimeProcessorCharacter(AFICRuntimeProcessorCharacter* Character);

	AFICRuntimeProcessorCharacter* GetRuntimeProcessorCharacter() { return RuntimeProcessorCharacter; }
	
	void SaveRenderTargetAsJPG(const FString& FilePath, UTextureRenderTarget2D* RenderTarget);

	AFICScene* FindSceneByName(const FString& InSceneName);
	UFICRuntimeProcess* FindRuntimeProcess(const FString& InKey);
	FString FindRuntimeProcessKey(UFICRuntimeProcess* InProcess);
};
