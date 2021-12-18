#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "GameFramework/Actor.h"
#include "FICAnimation.generated.h"

UENUM()
enum EFICKeyframeType {
	FIC_KF_NONE			= 0b0000000,
	FIC_KF_HANDLES		= 0b1000000,
	FIC_KF_EASE			= 0b1000001,
	FIC_KF_EASEINOUT	= 0b1000010,
	FIC_KF_MIRROR		= 0b1000100,
	FIC_KF_CUSTOM		= 0b1001000,
	FIC_KF_LINEAR		= 0b0010000,
	FIC_KF_STEP			= 0b0100000,
	FIC_KF_ALL			= 0b0111111,
};

USTRUCT(BlueprintType)
struct FFICKeyframe {
	GENERATED_BODY()

public:
	FFICKeyframe(EFICKeyframeType Type = FIC_KF_EASE) : KeyframeType(Type) {}
	virtual ~FFICKeyframe() = default;

	/**
	 * Returns the value of the keyframe as float.
	 * Intended to be used for attibute viewers like graph view.
	 */
	virtual float GetValueAsFloat() const { return 0.0f; }
	virtual void GetInControlAsFloat(float& OutFrame, float& OutValue) {}
	virtual void GetOutControlAsFloat(float& OutFrame, float& OutValue) {}
	virtual void SetValueFromFloat(float InValue) {}
	virtual void SetInControlAsFloat(float InFrame, float InValue) {}
	virtual void SetOutControlAsFloat(float InFrame, float InValue) {}
	

	UPROPERTY(SaveGame)
	TEnumAsByte<EFICKeyframeType> KeyframeType = FIC_KF_EASE;
};

struct FFICKeyframeRef {
private:
	FFICKeyframe* Keyframe = nullptr;
	bool bShouldDestroy = false;

public:
	FFICKeyframeRef(FFICKeyframe* Keyframe, bool bShouldDestroy = false) : Keyframe(Keyframe), bShouldDestroy(bShouldDestroy) {}
	~FFICKeyframeRef();

	const FFICKeyframe* operator->() const { return Keyframe; }
	FFICKeyframe* operator->() { return Keyframe; }
	const FFICKeyframe& operator*() const { return *Keyframe; }
	FFICKeyframe& operator*() { return *Keyframe; }
	operator bool() const { return static_cast<bool>(Keyframe); }

	const FFICKeyframe* Get() const { return Keyframe;}
	FFICKeyframe* Get() { return Keyframe;}
};

USTRUCT(BlueprintType)
struct FFICAttribute {
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE(FOnUpdate)
	FOnUpdate OnUpdate;

	virtual ~FFICAttribute() = default;
	virtual TMap<int64, TSharedPtr<FFICKeyframeRef>> GetKeyframes() { return TMap<int64, TSharedPtr<FFICKeyframeRef>>(); }
	virtual EFICKeyframeType GetAllowedKeyframeTypes() const { return FIC_KF_NONE; }
	virtual TSharedPtr<FFICKeyframeRef> AddKeyframe(int64 Time) { return nullptr; }
	virtual void RemoveKeyframe(int64 Time) {}
	virtual void MoveKeyframe(int64 From, int64 To) {}
	virtual void RecalculateKeyframe(int64 Time) {}

	void RecalculateAllKeyframes();
	bool GetNextKeyframe(int64 Time, int64& outTime, TSharedPtr<FFICKeyframeRef>& outKeyframe);
	bool GetPrevKeyframe(int64 Time, int64& outTime, TSharedPtr<FFICKeyframeRef>& outKeyframe);
};

float Interpolate(FVector2D P0, FVector2D P1, FVector2D P2, FVector2D P3, float t);

USTRUCT(BlueprintType)
struct FFICFloatKeyframe : public FFICKeyframe {
	GENERATED_BODY()

	typedef float ValueType;

	UPROPERTY(SaveGame)
	float Value = 0.0f;

	UPROPERTY(SaveGame)
	float InTanValue = 0.0f;

	UPROPERTY(SaveGame)
	float InTanTime = 0.0f;

	UPROPERTY(SaveGame)
	float OutTanValue = 0.0f;

	UPROPERTY(SaveGame)
	float OutTanTime = 0.0f;

	FFICFloatKeyframe() = default;
	FFICFloatKeyframe(float Value, EFICKeyframeType Type = FIC_KF_EASE) : FFICKeyframe(Type), Value(Value) {}

	virtual float GetValueAsFloat() const { return Value; }
	virtual void SetValueFromFloat(float InValue) { Value = InValue; }
	virtual void GetInControlAsFloat(float& OutFrame, float& OutValue) { OutFrame = InTanTime; OutValue = InTanValue; }
	virtual void SetInControlAsFloat(float InFrame, float InValue) override { InTanTime = FMath::Max(InFrame, 0.f); InTanValue = InValue; }
	virtual void GetOutControlAsFloat(float& OutFrame, float& OutValue) { OutFrame = OutTanTime; OutValue = OutTanValue; }
	virtual void SetOutControlAsFloat(float InFrame, float InValue) override { OutTanTime = FMath::Max(InFrame, 0.f); OutTanValue = InValue; }
};

USTRUCT(BlueprintType)
struct FFICFloatAttribute : public FFICAttribute {
	GENERATED_BODY()
public:
	typedef FFICFloatKeyframe KeyframeType;
	typedef float ValueType;
	
private:
	UPROPERTY(SaveGame)
	TMap<int64, FFICFloatKeyframe> Keyframes;

public:
	UPROPERTY(SaveGame)
	float FallBackValue = 0.0f;
	
	// Begin FFICAttribute
	virtual TMap<int64, TSharedPtr<FFICKeyframeRef>> GetKeyframes() override;
	virtual EFICKeyframeType GetAllowedKeyframeTypes() const override;
	virtual TSharedPtr<FFICKeyframeRef> AddKeyframe(int64 Time) override;
	virtual void RemoveKeyframe(int64 Time) override;
	virtual void MoveKeyframe(int64 From, int64 To) override;
	virtual void RecalculateKeyframe(int64 Time) override;
	// End FFICAttribute
	
	FFICFloatKeyframe* SetKeyframe(int64 Time, FFICFloatKeyframe Keyframe);
	float GetValue(float Time);
	void SetDefaultValue(float Value) { FallBackValue = Value; }
};

USTRUCT(BlueprintType)
struct FFICGroupAttribute : public FFICAttribute {
	GENERATED_BODY()

public:
	TMap<FString, TAttribute<FFICAttribute*>> Children;

	// Begin FFICAttribute
	virtual TMap<int64, TSharedPtr<FFICKeyframeRef>> GetKeyframes() override;
	virtual EFICKeyframeType GetAllowedKeyframeTypes() const override;
	virtual TSharedPtr<FFICKeyframeRef> AddKeyframe(int64 Time) override;
	virtual void RemoveKeyframe(int64 Time) override;
	virtual void MoveKeyframe(int64 From, int64 To) override;
	virtual void RecalculateKeyframe(int64 Time) override;
	// End FFICAttribute
};

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
};
