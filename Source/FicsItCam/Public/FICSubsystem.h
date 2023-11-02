#pragma once

#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "IImageWrapper.h"
#include "RHIGPUReadback.h"
#include "Util/SequenceExporter.h"
#include "FICSubsystem.generated.h"

class UFICRuntimeProcess;
class AFICScene;
class AFICTimelapseCamera;
class UFICEditorContext;
class AFICAnimation;
class AFICRuntimeProcessorCharacter;
class UFICCommand;

struct FFICRenderTarget {
	virtual ~FFICRenderTarget() {}

	virtual FTextureRHIRef GetRenderTarget() = 0;
};

struct FFICRenderRequest {
	FRenderCommandFence RenderFence = FRenderCommandFence();
	
	FRHIGPUTextureReadback Readback;

	TSharedRef<FSequenceExporter> Exporter;
	TSharedRef<FFICRenderTarget> RenderTarget;

	FFICRenderRequest(TSharedRef<FFICRenderTarget> RenderTarget, TSharedRef<FSequenceExporter> Exporter, FRHIGPUTextureReadback Readback) : RenderTarget(RenderTarget), Exporter(Exporter), Readback(Readback) {}
};

struct FFICRenderTarget_Raw : public FFICRenderTarget {
	FTextureRHIRef RenderTargetTexture;

	FFICRenderTarget_Raw(FRenderTarget* RenderTarget) : RenderTargetTexture(RenderTarget->GetRenderTargetTexture()) {}
	FFICRenderTarget_Raw(FTextureRHIRef RenderTargetTexture) : RenderTargetTexture(RenderTargetTexture) {}

	virtual FTextureRHIRef GetRenderTarget() override { return RenderTargetTexture; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFICRuntimeProcessDelegate, FString, Key, UFICRuntimeProcess*, RuntimeProcess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFICSceneDelegate, AFICScene*, Scene);

UCLASS()
class AFICSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
private:
	TQueue<TSharedPtr<FFICRenderRequest>> RenderRequestQueue;

	UPROPERTY(SaveGame)
	TMap<FString, UFICRuntimeProcess*> RuntimeProcesses;
	UPROPERTY(SaveGame)
	TSet<UFICRuntimeProcess*> PersistentActiveRuntimeProcesses;
	UPROPERTY()
	TSet<UFICRuntimeProcess*> ActiveRuntimeProcesses;
	UPROPERTY()
	TSet<AFICScene*> Scenes;
	
	UPROPERTY()
	AFICRuntimeProcessorCharacter* RuntimeProcessorCharacter = nullptr;
	UPROPERTY()
	ACharacter* OriginalPlayerCharacter = nullptr;

	TMap<TSubclassOf<UFICCommand>, TMap<FString, UFICCommand*>> Commands;
	
public:
	UPROPERTY(BlueprintAssignable)
	FFICRuntimeProcessDelegate OnRuntimeProcessCreated;
	UPROPERTY(BlueprintAssignable)
	FFICRuntimeProcessDelegate OnRuntimeProcessDeleted;
	UPROPERTY(BlueprintAssignable)
	FFICRuntimeProcessDelegate OnRuntimeProcessStarted;
	UPROPERTY(BlueprintAssignable)
	FFICRuntimeProcessDelegate OnRuntimeProcessStopped;
	UPROPERTY(BlueprintAssignable)
	FFICSceneDelegate OnSceneCreated;
	UPROPERTY(BlueprintAssignable)
	FFICSceneDelegate OnSceneDeleted;
	
	UFUNCTION(BlueprintCallable, Category="FicsIt-Cam", meta=(WorldContext = "WorldContext"))
	static AFICSubsystem* GetFICSubsystem(UObject* WorldContext);
	
	AFICSubsystem();

	// Begin UObject
	virtual void FinishDestroy() override;
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	// End UObject
	
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
	
	UFUNCTION(BlueprintCallable)
	bool CreateRuntimeProcess(FString Key, UFICRuntimeProcess* InProcess, bool bStartAutomatically = false);
	UFUNCTION(BlueprintCallable)
	bool RemoveRuntimeProcess(UFICRuntimeProcess* Process);
	UFUNCTION(BlueprintCallable)
	bool StartRuntimeProcess(UFICRuntimeProcess* Process);
	UFUNCTION(BlueprintCallable)
	bool StopRuntimeProcess(UFICRuntimeProcess* Process);
	UFUNCTION(BlueprintCallable)
	const TMap<FString, UFICRuntimeProcess*>& GetRuntimeProcesses() { return RuntimeProcesses; }
	UFUNCTION(BlueprintCallable)
	const TSet<UFICRuntimeProcess*>& GetActiveRuntimeProcesses() { return ActiveRuntimeProcesses; }
	UFUNCTION(BlueprintCallable)
	TMap<FString, UFICRuntimeProcess*> GetActiveRuntimeProcessesMap() {
		TMap<FString, UFICRuntimeProcess*> Map;
		for (const TPair<FString, UFICRuntimeProcess*>& Process : RuntimeProcesses) {
			if (ActiveRuntimeProcesses.Contains(Process.Value)) Map.Add(Process.Key, Process.Value);
		}
		return Map;
	}
	UFUNCTION(BlueprintCallable)
	bool IsRuntimeProcessActive(UFICRuntimeProcess* Process);
	UFUNCTION(BlueprintCallable)
	UFICRuntimeProcess* FindRuntimeProcess(const FString& InKey);
	UFUNCTION(BlueprintCallable)
	FString FindRuntimeProcessKey(UFICRuntimeProcess* InProcess);

	void CreateRuntimeProcessorCharacter(UFICRuntimeProcess* RuntimeProcess);
	void DestoryRuntimeProcessorCharacter(AFICRuntimeProcessorCharacter* Character);

	AFICRuntimeProcessorCharacter* GetRuntimeProcessorCharacter() { return RuntimeProcessorCharacter; }
	
	void ExportRenderTarget(TSharedRef<FSequenceExporter> Exporter, TSharedRef<FFICRenderTarget> RenderTarget, bool bInstant = false);
	void HandleRenderRequest(TSharedPtr<FFICRenderRequest> InRequest);

	UFUNCTION(BlueprintCallable)
	TSet<AFICScene*> GetScenes() const;
	UFUNCTION(BlueprintCallable)
	AFICScene* FindSceneByName(const FString& InSceneName);
	UFUNCTION(BlueprintCallable)
	AFICScene* CreateScene(const FString& InSceneName);
	UFUNCTION(BlueprintCallable)
	void DeleteScene(AFICScene* Scene);

	UFUNCTION(BlueprintCallable)
	void FilterAndSortRuntimeProcesses(const TArray<UFICRuntimeProcess*>& InRuntimeProcesses, TArray<UFICRuntimeProcess*>& OutActive, TArray<UFICRuntimeProcess*>& OutInactive);

	UFUNCTION(BlueprintCallable)
	void FilterAndSortScenes(const TArray<AFICScene*>& InScenes, TArray<AFICScene*>& OutActive, TArray<AFICScene*>& OutPaused, TArray<AFICScene*>& OutInactive);
};
