#pragma once

#include "CoreMinimal.h"
#include "Data/FICTypes.h"
#include "FICKeyframe.generated.h"

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
	virtual FICValue GetValue() const { checkf(false, TEXT("Not Implemented!")); return FICValue(); }
	virtual FFICValueTimeFloat GetInControl() { checkf(false, TEXT("Not Implemented!")); return FFICValueTimeFloat(); }
	virtual FFICValueTimeFloat GetOutControl() { checkf(false, TEXT("Not Implemented!")); return FFICValueTimeFloat(); }
	virtual EFICKeyframeType GetType() { return KeyframeType; }
	virtual void SetValue(FICValue InValue) { checkf(false, TEXT("Not Implemented!")); }
	virtual void SetInControl(const FFICValueTimeFloat& InInControl) { checkf(false, TEXT("Not Implemented!")); }
	virtual void SetOutControl(const FFICValueTimeFloat& InOutControl) { checkf(false, TEXT("Not Implemented!")); }
	virtual void SetType(EFICKeyframeType Type) { KeyframeType = Type; }

	UPROPERTY(SaveGame)
	TEnumAsByte<EFICKeyframeType> KeyframeType = FIC_KF_EASE;
};