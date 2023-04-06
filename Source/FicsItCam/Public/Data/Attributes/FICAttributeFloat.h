#pragma once

#include "FICAttribute.h"
#include "FICAttributeFloat.generated.h"

// TODO: Rename to FFICKeyframeFloat
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

	UPROPERTY(SaveGame)
	float OutTanTime = 0.0f;

	FFICFloatKeyframe() = default;
	FFICFloatKeyframe(float Value, EFICKeyframeType Type = FIC_KF_EASE) : FFICKeyframe(Type), Value(Value) {}

	virtual FICValue GetValue() const override { return Value; }
	virtual void SetValue(FICValue InValue) override { Value = InValue; }
	virtual FFICValueTimeFloat GetInControl() const override { return FFICValueTimeFloat(InTanTime, InTanValue); }
	virtual void SetInControl(const FFICValueTimeFloat& InInControl) override { InTanTime = FMath::Max(InInControl.Frame, 0.f); InTanValue = InInControl.Value; }
	virtual FFICValueTimeFloat GetOutControl() const { return FFICValueTimeFloat(OutTanTime, OutTanValue); }
	virtual void SetOutControl(const FFICValueTimeFloat& InOutControl) override { OutTanTime = FMath::Max(InOutControl.Frame, 0.f); OutTanValue = InOutControl.Value; }
};

// TODO: Rename to FFICAttributeFloat
USTRUCT(BlueprintType)
struct FFICFloatAttribute : public FFICAttribute {
	GENERATED_BODY()

	friend class FFICFloatKeyframeTrampoline;
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
	virtual FName GetAttributeType() const { return FName(TEXT("FloatAttribute")); }
	
	virtual EFICKeyframeType GetAllowedKeyframeTypes() const override;
	virtual TMap<FICFrame, TSharedRef<FFICKeyframe>> GetKeyframes() override;
	virtual TSharedRef<FFICKeyframe> AddKeyframe(FICFrame Time) override;
	virtual void RemoveKeyframe(FICFrame Time) override;
	virtual void MoveKeyframe(FICFrame From, FICFrame To) override;
	virtual void RecalculateKeyframe(FICFrame Time) override;
	virtual FICValue GetFloatValue(FICFrameFloat Time) override;
	virtual bool HasKeyframe(FICFrame Time) const override;

	virtual void Set(TSharedRef<FFICAttribute> InAttrib) override;
	virtual TSharedRef<FFICAttribute> Get() override;

	virtual TSharedRef<FFICEditorAttributeBase> CreateEditorAttribute() override;
	// End FFICAttribute

	virtual FFICFloatKeyframe* GetKeyframe(FICFrame Time) { return Keyframes.Find(Time); }
	
	FFICFloatKeyframe* SetKeyframe(FICFrame Time, FFICFloatKeyframe Keyframe);
	float GetValue(FICFrameFloat Time);
	void SetDefaultValue(float Value) { FallBackValue = Value; }
};

class FFICFloatKeyframeTrampoline : public FFICKeyframe {
private:
	FFICFloatAttribute* Attribute;
	FICFrame Frame;

public:
	FFICFloatKeyframeTrampoline(FFICFloatAttribute* Attribute, FICFrame Frame) : Attribute(Attribute), Frame(Frame) {}

	FFICFloatKeyframe* GetKeyframe() const { if (this) return &Attribute->Keyframes[Frame]; return nullptr; }
	
	virtual FICValue GetValue() const override { return GetKeyframe()->GetValue(); }
	virtual void SetValue(FICValue InValue) override { GetKeyframe()->SetValue(InValue); }
	virtual FFICValueTimeFloat GetInControl() const override {
		return GetKeyframe()->GetInControl();
	}
	virtual void SetInControl(const FFICValueTimeFloat& InInControl) override {
		GetKeyframe()->SetInControl(InInControl);
	}
	virtual FFICValueTimeFloat GetOutControl() const {
		return GetKeyframe()->GetOutControl();
	}
	virtual void SetOutControl(const FFICValueTimeFloat& InOutControl) override {
		GetKeyframe()->SetOutControl(InOutControl);
	}
	virtual EFICKeyframeType GetType() const override { return GetKeyframe()->GetType(); }
	virtual void SetType(EFICKeyframeType InType) override { return GetKeyframe()->SetType(InType); }
};
