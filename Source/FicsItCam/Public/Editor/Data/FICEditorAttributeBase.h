#pragma once

#include "Data/Attributes/FICKeyframe.h"
#include "Editor/UI/FICSequencerRow.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICKeyframeControl.h"
#include "Widgets/Input/SNumericEntryBox.h"

struct FFICAttribute;

DECLARE_MULTICAST_DELEGATE(FFICAttributeValueChanged)

/**
 * This is the base class of editor attributes which will manage a attribute in the view of
 * the editor. Such a editor attribute will contain a current state which is basically a cache
 * containing any value allowed for the attribute but which is not yet added to the actual animation.
 * When the current frame of the editor changes, this value should get updated with a calculated one
 * from the actual attribute. Otherwise this cached value is used for display, changed and cache used
 * so control interfaces can abstractly add, create, changed keyframes without the need to interact
 * directly with the value type of the attribute.
 */
class FFICEditorAttributeBase : public IFICSequencerRowProvider, public TSharedFromThis<FFICEditorAttributeBase> {
public:
	FFICAttributeValueChanged OnValueChanged;
	
	FLinearColor GraphColor;
	bool bShowInGraph = false;
	
	FFICEditorAttributeBase(FLinearColor GraphColor) : GraphColor(GraphColor) {}
	virtual ~FFICEditorAttributeBase() = default;

	// Begin IFICSequencerRowProvider
	virtual TSharedRef<SFICSequencerRow> CreateRow(SFICSequencer* InSequencer) override;
	virtual TArray<TSharedPtr<FFICSequencerRowMeta>> GetChildRows() override;
	virtual FText GetRowName() { return FText::FromString(TEXT("Unnamed")); }
	// End IFICSequencerRowProvider

	/**
	 * Returns a Unique Type name for this editor attribute kind (kind of a replacement for RTTI)
	 */
	virtual FName GetAttributeType() const;

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
	virtual const FFICAttribute& GetAttributeConst() const = 0;
	virtual FFICAttribute& GetAttribute() { return const_cast<FFICAttribute&>(GetAttributeConst()); }

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
	virtual TMap<FString, TSharedRef<FFICEditorAttributeBase>> GetChildAttributes() { return TMap<FString, TSharedRef<FFICEditorAttributeBase>>(); }

	/**
	 * Returns true if every child attribute and this attribute has a keyframe at the given frame
	 */
	bool AllKeyframesSet(FICFrame Frame) {
		if (!GetKeyframe(Frame).IsValid()) return false;
		for (const TPair<FString, TSharedRef<FFICEditorAttributeBase>>& Attribute : GetChildAttributes()) {
			if (!Attribute.Value->AllKeyframesSet(Frame)) {
				return false;
			}
		}
		return true;
	}

	/**
	 * Creates and returns a slate widget used to change the attributes details in the details panel
	 */
	virtual TSharedRef<SWidget> CreateDetailsWidget(UFICEditorContext* Context) = 0;

	/**
	 * Searches for a child attribute with the given name
	 */
	virtual TSharedRef<FFICEditorAttributeBase> operator[](const FString& Name) {
		return GetChildAttributes()[Name];
	}
	template<typename T = FFICEditorAttributeBase>
	TSharedRef<T> GetRef(const FString& Name) {
		return StaticCastSharedRef<T>((*this)[Name]);
	}
	template<typename T = FFICEditorAttributeBase>
	T& Get(const FString& Name) {
		return *GetRef<T>(Name);
	}
};

template<typename AttribType>
class TFICEditorAttribute : public FFICEditorAttributeBase {
private:
	AttribType& Attribute;
	typename AttribType::ValueType CurrentValue = AttribType::ValueType();

public:
	TFICEditorAttribute(AttribType& inAttribute, FLinearColor GraphColor = FColor::White) : FFICEditorAttributeBase(GraphColor), Attribute(inAttribute) {
		CurrentValue = Attribute.GetValue(0);
	}
	
	// Begin FFICEditorAttributeBase
	virtual void SetKeyframe(FICFrame Time) override {
		Attribute.LockUpdateEvent();
		Attribute.AddKeyframe(Time);
		typename AttribType::KeyframeType& KF = *Attribute.GetKeyframe(Time);
		KF.Value = CurrentValue;
		Attribute.RecalculateAllKeyframes();
		Attribute.UnlockUpdateEvent();
	}
	
	virtual bool HasChanged(FICFrame Time) const override {
		typename AttribType::ValueType Value = Attribute.GetValue(Time);
		return !FMath::IsNearlyZero(Value - CurrentValue, 0.001f);
	}
	
	virtual const FFICAttribute& GetAttributeConst() const override {
		return Attribute;
	}

	virtual void UpdateValue(FICFrame Time) override {
		SetValue(Attribute.GetValue(Time));
	}

	virtual FICValue GetValue(FICFrame InFrame) const override {
		return Attribute.GetValue(InFrame);
	}

	virtual void SetKeyframe(FFICValueTime InValueFrame, EFICKeyframeType InType = FIC_KF_EASE, bool bCreate = true) override {
		typename AttribType::KeyframeType* Keyframe = Attribute.GetKeyframe(InValueFrame.Frame);
		if (!bCreate && !Keyframe) return;
		typename AttribType::KeyframeType NewKeyframe;
		if (Keyframe) {
			NewKeyframe = *Keyframe;
			NewKeyframe.Value = InValueFrame.Value;
			NewKeyframe.KeyframeType = InType;
		} else {
			NewKeyframe = AttribType::KeyframeType(InValueFrame.Value, InType);
		}
		Attribute.SetKeyframe(InValueFrame.Frame, NewKeyframe);
	}

	virtual TSharedRef<SWidget> CreateDetailsWidget(UFICEditorContext* Context) override {
		return SNew(SHorizontalBox)
		+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
			SNew(SNumericEntryBox<float>)
			.Value_Lambda([this]() {
				return GetValue();
			})
			.SupportDynamicSliderMaxValue(true)
			.SupportDynamicSliderMinValue(true)
			.SliderExponent(1)
			.Delta(1)
			.MinValue(TOptional<typename AttribType::ValueType>())
			.MaxValue(TOptional<typename AttribType::ValueType>())
			.MinSliderValue(TOptional<float>())
			.MaxSliderValue(TOptional<float>())
			.LinearDeltaSensitivity(10)
			.AllowSpin(true)
			.OnValueChanged_Lambda([this, Context](typename AttribType::ValueType Val) {
				Context->CommitAutoKeyframe((uint8*)this);
				SetValue(Val);
				Context->CommitAutoKeyframe(nullptr);
			})
			.OnValueCommitted_Lambda([this, Context](typename AttribType::ValueType Val, auto) {
				Context->CommitAutoKeyframe((uint8*)this);
				SetValue(Val);
				Context->CommitAutoKeyframe(nullptr);
			})
			.TypeInterface(MakeShared<TDefaultNumericTypeInterface<typename AttribType::ValueType>>())
		]
		+SHorizontalBox::Slot().Padding(5).AutoWidth()[
			SNew(SFICKeyframeControl, Context, SharedThis(this))
			.Frame_Lambda([this, Context]() {
				return Context->GetCurrentFrame();
			})
		];
	}
	// End FFICEditorAttributeBase

	void SetValue(typename AttribType::ValueType Value) {
		if (CurrentValue == Value) return;
		CurrentValue = Value;
		OnValueChanged.Broadcast();
		if (!IsAnimated()) Attribute.SetDefaultValue(CurrentValue);
	}

	typename AttribType::ValueType GetValue() {
		return CurrentValue;
	}

	typename AttribType::ValueType GetValue(FICFrame Time) {
		return Attribute.GetValue(Time);
	}
};
