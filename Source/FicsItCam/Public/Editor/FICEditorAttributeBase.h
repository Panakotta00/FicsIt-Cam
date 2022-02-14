#pragma once

#include "FicsItCam/Public/Data/Attributes/FICAttribute.h"
#include "FicsItCam/Public/Data/Attributes/FICAttributeGroup.h"

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
protected:
	FFICAttributeValueChanged OnValueChanged;
	
public:
	FLinearColor GraphColor;
	bool bShowInGraph = false;
	
	FFICEditorAttributeBase(const FFICAttributeValueChanged& OnValueChanged, FLinearColor GraphColor) : OnValueChanged(OnValueChanged), GraphColor(GraphColor) {}
	virtual ~FFICEditorAttributeBase() = default;
	
	/**
	 * Creates or Sets a/the keyframe at the given frame to the current value.
	 */
	virtual void SetKeyframe(FICFrame Time) = 0;

	/**
	 * Removes the keyframe at the given frame if it exists.
	 */
	virtual void RemoveKeyframe(FICFrame Time);

	/**
	 * Returns the keyframe at the given time, otherwise returns nullptr.
	 */
	virtual TSharedPtr<FFICKeyframe> GetKeyframe(FICFrame Time);

	/**
	 * Returns true if the attribute contains any keyframes.
	 */
	virtual bool IsAnimated();

	/**
	 * Returns true if the value of the attribute differs from the current value.
	 */
	virtual bool HasChanged(FICFrame Time) const = 0;

	/**
	 * Returns the attribute.
	 */
	virtual const FFICAttribute* GetAttributeConst() const = 0;
	virtual FFICAttribute* GetAttribute() { return const_cast<FFICAttribute*>(GetAttributeConst()); }

	/**
	 * Sets the current value to the value in the attribute at the given frame
	 */
	virtual void UpdateValue(FICFrame Time) = 0;

	/**
	 * Returns the value at a given frame as float,
	 * intended to be used for unified attribute views, like graph view.
	 */
	virtual FICValue GetValue(FICFrame InFrame) const = 0;

	/**
	 * Set a keyframe at the given frame from the given float value,
	 * intended to be used for unified attribute views that can edit the attribute, like graph view.
	 */
	virtual void SetKeyframe(FFICValueTime InValueFrame, EFICKeyframeType InType = FIC_KF_EASE, bool bCreate = true) = 0;

	/**
	 * Returns a map of child attributes and names
	 */
	virtual TMap<FString, TAttribute<FFICEditorAttributeBase*>> GetChildAttributes() { return TMap<FString, TAttribute<FFICEditorAttributeBase*>>(); };
};

template<typename AttribType>
class TFICEditorAttribute : public FFICEditorAttributeBase {
private:
	TAttribute<AttribType*> Attribute;
	typename AttribType::ValueType CurrentValue = AttribType::ValueType();

public:
	TFICEditorAttribute(TAttribute<AttribType*> inAttribute = nullptr, FFICAttributeValueChanged OnValueChanged = FFICAttributeValueChanged(), FLinearColor GraphColor = FColor::White) : FFICEditorAttributeBase(OnValueChanged, GraphColor) {
		Attribute = inAttribute;
		if (Attribute.Get()) {
			CurrentValue = Attribute.Get()->GetValue(0);
		}
	}
	
	// Begin FFICEditorAttributeBase
	virtual void SetKeyframe(int64 Time) override {
		if (!Attribute.Get()) return;
		Attribute.Get()->AddKeyframe(Time);
		typename AttribType::KeyframeType& KF = *Attribute.Get()->GetKeyframe(Time);
		KF.Value = CurrentValue;
		Attribute.Get()->RecalculateAllKeyframes();
	}
	
	virtual bool HasChanged(int64 Time) const override {
		if (!Attribute.Get()) return false;
		typename AttribType::ValueType Value = Attribute.Get()->GetValue(Time);
		return FMath::Abs(Value - CurrentValue) > 0.0001;
	}
	
	virtual const FFICAttribute* GetAttributeConst() const override {
		return Attribute.Get();
	}

	virtual void UpdateValue(FICFrame Time) override {
		if (!Attribute.Get()) return;
		SetValue(Attribute.Get()->GetValue(Time));
	}

	virtual float GetValue(int64 InFrame) const override {
		return Attribute.Get()->GetValue(InFrame);
	}

	virtual void SetKeyframe(FFICValueTime InValueFrame, EFICKeyframeType InType = FIC_KF_EASE, bool bCreate = true) override {
		typename AttribType::KeyframeType* Keyframe = Attribute.Get()->GetKeyframe(InValueFrame.Frame);
		if (!bCreate && !Keyframe) return;
		typename AttribType::KeyframeType NewKeyframe;
		if (Keyframe) {
			NewKeyframe = *Keyframe;
			NewKeyframe.Value = InValueFrame.Value;
			NewKeyframe.KeyframeType = InType;
		} else {
			NewKeyframe = AttribType::KeyframeType(InValueFrame.Value, InType);
		}
		Attribute.Get()->SetKeyframe(InValueFrame.Frame, NewKeyframe);
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

	typename AttribType::ValueType GetValue(int64 Time) {
		return Attribute.Get()->GetValue(Time);
	}
};

class FFICEditorGroupAttribute : public FFICEditorAttributeBase {
private:
	TMap<FString, TAttribute<FFICEditorAttributeBase*>> Attributes;
	FFICGroupAttribute All;

public:
	FFICEditorGroupAttribute(const TMap<FString, TAttribute<FFICEditorAttributeBase*>>& InAttributes = TMap<FString, TAttribute<FFICEditorAttributeBase*>>(), FFICAttributeValueChanged OnValueChanged = FFICAttributeValueChanged(), FLinearColor GraphColor = FColor::White) : FFICEditorAttributeBase(OnValueChanged, GraphColor), Attributes(InAttributes) {
		for (const TPair<FString, TAttribute<FFICEditorAttributeBase*>>& Attr : Attributes) {
			All.Children.Add(Attr.Key, TAttribute<FFICAttribute*>::Create([Attr]() {
				FFICEditorAttributeBase* Attrib = Attr.Value.Get();
				if (Attrib) return Attrib->GetAttribute();
				return (FFICAttribute*)nullptr;
			}));
		}

		All.OnUpdate.AddLambda([this]() {
			for (TTuple<FString, TAttribute<FFICEditorAttributeBase*>> Attribute : Attributes) {
				Attribute.Value.Get()->GetAttribute()->OnUpdate.Broadcast();
			}
		});
	}
	
	// Begin FFICEditorAttributeBase
	virtual void SetKeyframe(FICFrame Time) override;
	virtual void RemoveKeyframe(FICFrame Time) override;
	virtual bool HasChanged(FICFrame Time) const override;
	virtual const FFICAttribute* GetAttributeConst() const override;
	virtual void UpdateValue(FICFrame Time) override;
	virtual float GetValue(FICFrame InFrame) const override;
	virtual void SetKeyframe(FFICValueTime InValueFrame, EFICKeyframeType InType = FIC_KF_EASE, bool bCreate = true) override;
	virtual TMap<FString, TAttribute<FFICEditorAttributeBase*>> GetChildAttributes() override;
	// End FFICEditorAttributeBase
};