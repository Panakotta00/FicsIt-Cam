#pragma once
#include "Attribute.h"
#include "FICAnimation.h"
#include "util/Logging.h"

DECLARE_DELEGATE(FFICAttributeValueChanged)

/**
 * This is the base class of editor attributes which will manage a attribute in the view of
 * the editor. Such a editor attribute will contain a current state which is basically a cache
 * containing any value allowed for the attribute but which is not yet added to the actual animation.
 * When the current frame of the editor changes, this value should get updated with a calculated one
 * from the actual attribute. Otherwise this cached value is used for display, changed and cache used
 * so control interfaces can abstractly add, create, changed keyframes without the need to interact
 * directly with the value type of the attribute.
 */
class FFICEditorAttributeBase {
private:
	int64 Frame = 0;

protected:
	FFICAttributeValueChanged OnValueChanged;
	
public:
	FFICEditorAttributeBase(const FFICAttributeValueChanged& OnValueChanged) : OnValueChanged(OnValueChanged) {}
	virtual ~FFICEditorAttributeBase() = default;
	
	/**
	 * Creates or Sets a/the keyframe at the given frame to the current value.
	 */
	virtual void SetKeyframe(int64 Time) = 0;

	/**
	 * Removes the keyframe at the given frame if it exists.
	 */
	virtual void RemoveKeyframe(int64 Time);

	/**
	 * Returns the keyframe at the given time, otherwise returns nullptr.
	 */
	virtual TSharedPtr<FFICKeyframeRef> GetKeyframe(int64 Time);

	/**
	 * Returns true if the attribute contains any keyframes.
	 */
	virtual bool IsAnimated();

	/**
	 * Returns true if the value of the attribute differs from the current value.
	 */
	virtual bool HasChanged(int64 Time) = 0;

	/**
	 * Returns the attribute.
	 */
	virtual FFICAttribute* GetAttribute() = 0;

	/**
	 * Returns the current Frame
	 */
	virtual int64 GetFrame();

	/**
	 * Sets the current Frame
	 */
	virtual void SetFrame(int64 Frame);

	/**
	 * Sets the current value to the value in the attribute at the current frame
	 */
	virtual void UpdateValue() = 0;
};

template<typename AttribType>
class TFICEditorAttribute : public FFICEditorAttributeBase {
private:
	TAttribute<AttribType*> Attribute;
	typename AttribType::ValueType CurrentValue = AttribType::ValueType();

public:
	TFICEditorAttribute(TAttribute<AttribType*> inAttribute = nullptr, FFICAttributeValueChanged OnValueChanged = FFICAttributeValueChanged()) : FFICEditorAttributeBase(OnValueChanged) {
		Attribute = inAttribute;
		if (Attribute.Get()) CurrentValue = Attribute.Get()->GetValue(0);
	}
	
	// Begin FFICEditorAttributeBase
	virtual void SetKeyframe(int64 Time) override {
		if (!Attribute.Get()) return;
		typename AttribType::KeyframeType* KF = static_cast<typename AttribType::KeyframeType*>(Attribute.Get()->AddKeyframe(Time)->Get());
		if (KF) KF->Value = CurrentValue;
		Attribute.Get()->RecalculateAllKeyframes();
		UpdateValue();
	}
	
	virtual bool HasChanged(int64 Time) override {
		if (!Attribute.Get()) return false;
		typename AttribType::ValueType Value = Attribute.Get()->GetValue(Time);
		return FMath::Abs(Value - CurrentValue) > 0.0001;
	}
	
	virtual FFICAttribute* GetAttribute() override {
		return Attribute.Get();
	}

	virtual void UpdateValue() override {
		if (!Attribute.Get()) return;
		SetValue(Attribute.Get()->GetValue(GetFrame()));
	}
	// End FFICEditorAttributeBase

	TAttribute<FFICAttribute*> GetAttribAttrib() { return Attribute; }

	void SetValue(typename AttribType::ValueType Value) {
		if (CurrentValue == Value) return;
		CurrentValue = Value;
		bool _ = OnValueChanged.ExecuteIfBound();
		if (!Attribute.Get()) return;
		if (!IsAnimated()) Attribute.Get()->SetDefaultValue(CurrentValue);
	}

	typename AttribType::ValueType GetValue() {
		return CurrentValue;
	}
};

class FFICEditorGroupAttribute : public FFICEditorAttributeBase {
private:
	TMap<FString, TAttribute<FFICEditorAttributeBase*>> Attributes;
	FFICGroupAttribute All;

public:
	FFICEditorGroupAttribute(const TMap<FString, TAttribute<FFICEditorAttributeBase*>>& Attributes = TMap<FString, TAttribute<FFICEditorAttributeBase*>>(), FFICAttributeValueChanged OnValueChanged = FFICAttributeValueChanged()) : FFICEditorAttributeBase(OnValueChanged), Attributes(Attributes) {
		for (const TPair<FString, TAttribute<FFICEditorAttributeBase*>>& Attr : Attributes) {
			All.Children.Add(Attr.Key, TAttribute<FFICAttribute*>::Create([Attr]() {
				FFICEditorAttributeBase* Attrib = Attr.Value.Get();
				if (Attrib) return Attrib->GetAttribute();
				return (FFICAttribute*)nullptr;
			}));
		}
	}
	
	// Begin FFICEditorAttributeBase
	virtual void SetKeyframe(int64 Time) override;
	virtual bool HasChanged(int64 Time) override;
	virtual FFICAttribute* GetAttribute() override;
	virtual void UpdateValue() override;
	// End FFICEditorAttributeBase
};