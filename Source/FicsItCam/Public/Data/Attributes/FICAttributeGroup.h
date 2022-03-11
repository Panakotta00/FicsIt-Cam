#pragma once

#include "FICAttribute.h"
#include "FICAttributeGroup.generated.h"

USTRUCT()
struct FFICKeyframeGroup : public FFICKeyframe {
	GENERATED_BODY()
private:
	struct FFICGroupAttribute* Attribute = nullptr;
	FICFrame Frame;

public:
	FFICKeyframeGroup(struct FFICGroupAttribute* InAttribute = nullptr, FICFrame InFrame = 0) : Attribute(InAttribute), Frame(InFrame) {}
	
	virtual FICValue GetValue() const { return FICValue(); }
	virtual FFICValueTimeFloat GetInControl() { return FFICValueTimeFloat(); }
	virtual FFICValueTimeFloat GetOutControl() { return FFICValueTimeFloat(); }
	virtual EFICKeyframeType GetType() { return EFICKeyframeType::FIC_KF_CUSTOM; }
	virtual void SetValue(FICValue InValue) { }
	virtual void SetInControl(const FFICValueTimeFloat& InInControl) { }
	virtual void SetOutControl(const FFICValueTimeFloat& InOutControl) { }
	virtual void SetType(EFICKeyframeType Type) override;
};

USTRUCT(BlueprintType)
struct FFICGroupAttribute : public FFICAttribute {
	GENERATED_BODY()
	friend class FFICEditorAttributeGroup;
	friend struct FFICKeyframeGroup;
public:
	inline static const FName TypeName = FName(TEXT("GroupAttribute"));
	
protected:
	TMap<FString, TSharedRef<FFICAttribute>> AttributeCache;
	TMap<FString, FFICAttribute*> Children;
	TMap<FString, FDelegateHandle> UpdateDelegateHandles;

public:
	virtual ~FFICGroupAttribute() override;
	
	// Begin FFICAttribute
	virtual FName GetAttributeType() const { return TypeName; }
	
	virtual EFICKeyframeType GetAllowedKeyframeTypes() const override;
	virtual TMap<FICFrame, TSharedRef<FFICKeyframe>> GetKeyframes() override;
	virtual TSharedRef<FFICKeyframe> AddKeyframe(FICFrame Time) override;
	virtual void RemoveKeyframe(FICFrame Time) override;
	virtual void MoveKeyframe(FICFrame From, FICFrame To) override;
	virtual void RecalculateKeyframe(FICFrame Time) override;
	virtual FICValue GetFloatValue(FICFrameFloat Time) override { return 0.0f; }

	virtual void Set(TSharedRef<FFICAttribute> InAttrib) override;
	virtual TSharedRef<FFICAttribute> Get() override;

	virtual TSharedRef<FFICEditorAttributeBase> CreateEditorAttribute() override;
	// End FFICAttribute

	void AddChildAttribute(FString Name, FFICAttribute* Attribute);
	void RemoveChildAttribute(FString Name);
};