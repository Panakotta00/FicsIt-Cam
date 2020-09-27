#pragma once

#include "CoreMinimal.h"
#include "FICAnimation.generated.h"

UENUM()
enum EFICKeyframeType {
	FIC_KF_NONE			= 0b000000,
	FIC_KF_EASE			= 0b000001,
	FIC_KF_EASEINOUT	= 0b000010,
	FIC_KF_MIRROR		= 0b000100,
	FIC_KF_CUSTOM		= 0b001000,
	FIC_KF_LINEAR		= 0b010000,
	FIC_KF_STEP			= 0b100000,
	FIC_KF_ALL			= 0b111111,
};

USTRUCT(BlueprintType)
struct FFICKeyframe {
	GENERATED_BODY()

public:
	virtual ~FFICKeyframe() = default;

	UPROPERTY(SaveGame)
	TEnumAsByte<EFICKeyframeType> KeyframeType = FIC_KF_EASE;
};

USTRUCT(BlueprintType)
struct FFICAttribute {
	GENERATED_BODY()

	virtual ~FFICAttribute() = default;
	virtual TMap<int64, FFICKeyframe*> GetKeyframes() { return TMap<int64, FFICKeyframe*>(); }
	virtual EFICKeyframeType GetAllowedKeyframeTypes() const { return FIC_KF_NONE; }
	virtual void AddKeyframe(int64 Time) {}
	virtual void RemoveKeyframe(int64 Time) {}
	virtual void MoveKeyframe(int64 From, int64 To) {}
	virtual void RecalculateKeyframe(int64 Time) {}

	void RecalculateAllKeyframes();
	bool GetNextKeyframe(int64 Time, int64& outTime, FFICKeyframe*& outKeyframe);
	bool GetPrevKeyframe(int64 Time, int64& outTime, FFICKeyframe*& outKeyframe);
};

float Interpolate(FVector2D P0, FVector2D P1, FVector2D P2, FVector2D P3, float t);

USTRUCT(BlueprintType)
struct FFICFloatKeyframe : public FFICKeyframe {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	float Value = 0.0f;

	UPROPERTY(SaveGame)
	float InTanValue = 0.0f;

	UPROPERTY(SaveGame)
	float InTanTime = 0.0f;

	UPROPERTY(SaveGame)
	float OutTanValue = 0.0f;

	UPROPERTY()
	float OutTanTime = 0.0f;

	FFICFloatKeyframe() = default;
	FFICFloatKeyframe(float Value) : Value(Value) {}
};

USTRUCT(BlueprintType)
struct FFICFloatAttribute : public FFICAttribute {
	GENERATED_BODY()
	
private:
	UPROPERTY(SaveGame)
	TMap<int64, FFICFloatKeyframe> Keyframes;

public:
	UPROPERTY(SaveGame)
	float FallBackValue = 0.0f;
	
	// Begin FFICAttribute
	virtual TMap<int64, FFICKeyframe*> GetKeyframes() override;
	virtual EFICKeyframeType GetAllowedKeyframeTypes() const override;
	virtual void AddKeyframe(int64 Time) override;
	virtual void RemoveKeyframe(int64 Time) override;
	virtual void MoveKeyframe(int64 From, int64 To) override;
	virtual void RecalculateKeyframe(int64 Time) override;
	// End FFICAttribute
	
	void SetKeyframe(int64 Time, FFICFloatKeyframe Keyframe);
	float GetValue(float Time);
	
};

UCLASS(BlueprintType)
class UFICAnimation : public UObject {
	GENERATED_BODY()

public:
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

	UPROPERTY()
	FFICFloatAttribute FOV;

	UPROPERTY()
	int64 AnimationStart = 0;

	UPROPERTY()
	int64 AnimationEnd = 300;


	UFICAnimation();
	
	void RecalculateAllKeyframes();

	int64 GetEndOfAnimation();
};
