#pragma once

#include "FICAttribute.h"
#include "FICAttributeBool.generated.h"

USTRUCT(BlueprintType)
struct FFICKeyframeBool : public FFICKeyframe {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	bool Value = false;

	FFICKeyframeBool() = default;
	FFICKeyframeBool(bool Value) : FFICKeyframe(FIC_KF_STEP), Value(Value) {}

	virtual FICValue GetValue() const override { return Value ? 1.0f : 0.0f; }
	virtual void SetValue(FICValue InValue) override { Value = FMath::IsNearlyZero(InValue); }
	virtual FFICValueTimeFloat GetInControl() override { return FFICValueTimeFloat(0, 0); }
	virtual void SetInControl(const FFICValueTimeFloat& InInControl) override { }
	virtual FFICValueTimeFloat GetOutControl() { return FFICValueTimeFloat(0, 0); }
	virtual void SetOutControl(const FFICValueTimeFloat& InOutControl) override { }
};

USTRUCT(BlueprintType)
struct FFICAttributeBool : public FFICAttribute {
	GENERATED_BODY()

	friend class FFICKeyframeBoolTrampoline;

private:
	UPROPERTY(SaveGame)
	TMap<int64, FFICKeyframeBool> Keyframes;

public:
	UPROPERTY(SaveGame)
	bool FallBackValue = false;
	
	// Begin FFICAttribute
	virtual FName GetAttributeType() { return FName(TEXT("AttributeBool")); }
	
	virtual EFICKeyframeType GetAllowedKeyframeTypes() const override;
	virtual TMap<FICFrame, TSharedRef<FFICKeyframe>> GetKeyframes() override;
	virtual TSharedRef<FFICKeyframe> AddKeyframe(FICFrame Time) override;
	virtual void RemoveKeyframe(FICFrame Time) override;
	virtual void MoveKeyframe(FICFrame From, FICFrame To) override;
	virtual void RecalculateKeyframe(FICFrame Time) override;

	virtual void Set(TSharedRef<FFICAttribute> InAttrib) override;
	virtual TSharedRef<FFICAttribute> Get() override;

	virtual TSharedRef<FFICEditorAttributeBase> CreateEditorAttribute() override;
	// End FFICAttribute

	FFICKeyframeBool* SetKeyframe(FICFrame Time, FFICKeyframeBool Keyframe);
	bool GetValue(FICFrameFloat Time);
	void SetDefaultValue(bool Value) { FallBackValue = Value; }
};

class FFICKeyframeBoolTrampoline : public FFICKeyframe {
private:
	FFICAttributeBool* Attribute;
	FICFrame Frame;

public:
	FFICKeyframeBoolTrampoline(FFICAttributeBool* Attribute, FICFrame Frame) : Attribute(Attribute), Frame(Frame) {}

	FFICKeyframeBool* GetKeyframe() const { if (this) return &Attribute->Keyframes[Frame]; return nullptr; }
	
	virtual FICValue GetValue() const override { return GetKeyframe()->GetValue(); }
	virtual void SetValue(FICValue InValue) override { GetKeyframe()->SetValue(InValue); }
	virtual FFICValueTimeFloat GetInControl() override {
		return GetKeyframe()->GetInControl();
	}
	virtual void SetInControl(const FFICValueTimeFloat& InInControl) override {
		GetKeyframe()->SetInControl(InInControl);
	}
	virtual FFICValueTimeFloat GetOutControl() {
		return GetKeyframe()->GetOutControl();
	}
	virtual void SetOutControl(const FFICValueTimeFloat& InOutControl) override {
		GetKeyframe()->SetOutControl(InOutControl);
	}
	virtual EFICKeyframeType GetType() override { return GetKeyframe()->GetType(); }
	virtual void SetType(EFICKeyframeType InType) override { return GetKeyframe()->SetType(InType); }
};
