#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "Attributes/FICAttributeFloat.h"
#include "FICAnimation.generated.h"

UCLASS(BlueprintType)
class AFICAnimation : public AActor, public IFGSaveInterface {
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	FString Name;
	
	UPROPERTY(SaveGame)
	FFICFloatAttribute PosX;
	UPROPERTY(SaveGame)
	FFICFloatAttribute PosY;
	UPROPERTY(SaveGame)
	FFICFloatAttribute PosZ;

	UPROPERTY(SaveGame)
	FFICFloatAttribute RotPitch;
	UPROPERTY(SaveGame)
	FFICFloatAttribute RotYaw;
	UPROPERTY(SaveGame)
	FFICFloatAttribute RotRoll;

	UPROPERTY(SaveGame)
	FFICFloatAttribute FOV;
	UPROPERTY(SaveGame)
	FFICFloatAttribute Aperture;
	UPROPERTY(SaveGame)
	FFICFloatAttribute FocusDistance;

	UPROPERTY(SaveGame)
	int64 AnimationStart = 0;

	UPROPERTY(SaveGame)
	int64 AnimationEnd = 300;

	UPROPERTY(SaveGame)
	int64 FPS = 30;
	
	UPROPERTY(SaveGame)
	int64 ResolutionWidth = 1920;
	UPROPERTY(SaveGame)
	int64 ResolutionHeight = 1080;

	UPROPERTY(SaveGame)
	float SensorWidth = 23.76;
	UPROPERTY(SaveGame)
	float SensorHeight = 13.365;

	UPROPERTY(SaveGame)
	bool bUseCinematic = true;

	UPROPERTY(SaveGame)
	bool bBulletTime = false;

	AFICAnimation();

	// Begin AActor
	virtual void OnConstruction(const FTransform& Transform) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	// End IFGSaveInterface
	
	void RecalculateAllKeyframes();

	float GetEndOfAnimation();
	float GetStartOfAnimation();

	FFICFrameRange GetAnimationRange() {
		return FFICFrameRange(AnimationStart, AnimationEnd);
	}
};
