#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FICTypes.h"
#include "GameFramework/Actor.h"
#include "Objects/FICCamera.h"
#include "Util/FICProceduralTexture.h"
#include "FICScene.generated.h"

struct FICSITCAM_API FFICSceneVersion {
	enum Type {
		NoVersion,

		InitialVersion,

		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	inline static const FGuid GUID = FGuid( 0x04730e5a, 0x04414b85, 0xaa76f90a, 0x57a43a21 );

	inline static const TCHAR Name[] = TEXT( "RuntimeBuildableInstanceVersion" );
};

UCLASS()
class FICSITCAM_API AFICScene : public AActor, public IFGSaveInterface {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	TArray<UObject*> SceneObjects;

	UPROPERTY(SaveGame)
	UFICProceduralTexture* PreviewTexture;

public:
	UPROPERTY(BlueprintAssignable)
	FFICTextureUpdateDelegate OnPreviewUpdate;
	
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FString SceneName = "Unnamed";
	
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FFICFrameRange AnimationRange = FFICFrameRange(1, 600);
	
	UPROPERTY(SaveGame, BlueprintReadWrite)
	int64 FPS = 60;
	
	UPROPERTY(SaveGame, BlueprintReadWrite)
	int64 ResolutionWidth = 1920;
	UPROPERTY(SaveGame, BlueprintReadWrite)
	int64 ResolutionHeight = 1080;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	bool bBulletTime = false;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	bool bLooping = false;

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FTransform LastCameraTransform;
	UPROPERTY(SaveGame, BlueprintReadOnly)
	float LastCameraFOV = 120;
	UPROPERTY(SaveGame, BlueprintReadOnly)
	UObject* LastSelectedSceneObject = nullptr;
	UPROPERTY(SaveGame, BLueprintReadOnly)
	bool bViewportEverSaved = false;

	// Begin IFGSaveInterface
	//virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override { out_dependentObjects.Append(SceneObjects); }
	virtual bool ShouldSave_Implementation() const override { return true; }
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface
	
	const TArray<UObject*>& GetSceneObjects() {
		return SceneObjects;
	}

	void AddSceneObject(UObject* Object) {
		check(Object->Implements<UFICSceneObject>());
		SceneObjects.Add(Object);
	}

	void RemoveSceneObject(UObject* Object) {
		SceneObjects.Remove(Object);
	}

	void MoveSceneObject(UObject* Object, int Delta);

	UFUNCTION(BlueprintCallable)
	FMinimalViewInfo CalculateView(double Time);
	bool CalculateView(FMinimalViewInfo& InOutViewInfo, FICFrameFloat Time);

	UFICCamera* GetActiveCamera(FICFrameFloat Time);
	
	UFUNCTION(BlueprintCallable)
	UTexture* GetPreviewTexture();

	UFUNCTION()
	void OnTextureUpdate();

	UFUNCTION()
	void UpdatePreview();

	UFUNCTION(BlueprintCallable)
	static FString GetSceneProcessKey(const FString& InSceneName) {
		return FString::Printf(TEXT("Scene_%s"), *InSceneName);
	}

	UFUNCTION(BlueprintCallable)
	bool IsSceneAlreadyInUse();

	void SerializeScene(TArray<UObject*>& ObjectsToSave, FArchive& Ar);
	void SaveToFile(const FString& Path);
	void LoadFromFile(const FString& Path);
};
