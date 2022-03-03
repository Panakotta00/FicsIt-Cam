#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FICTypes.h"
#include "FICUtils.h"
#include "Objects/FICCamera.h"
#include "FICScene.generated.h"

UCLASS()
class FICSITCAM_API AFICScene : public AActor, public IFGSaveInterface {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	TArray<UObject*> SceneObjects;

public:
	UPROPERTY(SaveGame)
	FString SceneName = "Unnamed";
	
	UPROPERTY(SaveGame)
	FFICFrameRange AnimationRange = FFICFrameRange(1, 300);
	
	UPROPERTY(SaveGame)
	int64 FPS = 30;
	
	UPROPERTY(SaveGame)
	int64 ResolutionWidth = 1920;
	UPROPERTY(SaveGame)
	int64 ResolutionHeight = 1080;

	UPROPERTY(SaveGame)
	FVector2D SensorDimension = FVector2D(23.76, 13.365);

	UPROPERTY(SaveGame)
	bool bUseCinematic = true;

	UPROPERTY(SaveGame)
	bool bBulletTime = false;

	UPROPERTY(SaveGame)
	FTransform LastCameraTransform;
	UPROPERTY(SaveGame)
	UObject* LastSelectedSceneObject = nullptr;
	UPROPERTY(SaveGame)
	bool bViewportEverSaved = false;

	// Begin IFGSaveInterface
	//virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override { out_dependentObjects.Append(SceneObjects); }
	virtual bool ShouldSave_Implementation() const override { return true; }
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface
	
	TArray<UObject*> GetSceneObjects() {
		return SceneObjects;
	}

	void AddSceneObject(UObject* Object) {
		check(Object->Implements<UFICSceneObject>());
		SceneObjects.Add(Object);
	}

	void RemoveSceneObject(UObject* Object) {
		SceneObjects.Remove(Object);
	}

	void MoveSceneObject(UObject* Object, int Delta) {
		int Index = SceneObjects.Find(Object);
		SceneObjects.RemoveAt(Index);
		SceneObjects.Insert(Object, UFICUtils::Modulo(Index + Delta, SceneObjects.Num()+1));
	}

	UFICCamera* GetActiveCamera(FICFrameFloat Time);
};
