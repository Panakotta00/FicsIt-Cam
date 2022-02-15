#pragma once

#include "CoreMinimal.h"
#include "FICTypes.h"
#include "Objects/FICCamera.h"
#include "FICScene.generated.h"

UCLASS()
class FICSITCAM_API AFICScene : public AActor {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	TArray<UObject*> SceneObjects;

public:
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
	
	TArray<UObject*> GetSceneObjects() {
		return SceneObjects;
	}

	void AddSceneObject(UObject* Object) {
		check(Object->Implements<UFICSceneObject>());
		SceneObjects.Add(Object);
		// TODO: Tell Editor Context that Scene Objects have changed and cause a reload of details panel etc.
	}
};
