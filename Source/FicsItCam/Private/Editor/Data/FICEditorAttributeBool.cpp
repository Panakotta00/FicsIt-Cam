#include "Editor/Data/FICEditorAttributeBool.h"

#include "Data/Attributes/FICAttributeBool.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICKeyframeControl.h"

FFICEditorAttributeBool::FFICEditorAttributeBool(FFICAttributeBool* InAttribute, FLinearColor GraphColor) : FFICEditorAttributeBase(GraphColor), Attribute(InAttribute) {}

void FFICEditorAttributeBool::SetKeyframe(FICFrame Time) {
	Attribute->SetKeyframe(Time, FFICKeyframeBool(GetActiveValue()));
}

void FFICEditorAttributeBool::RemoveKeyframe(FICFrame Time) {
	Attribute->RemoveKeyframe(Time);
}

bool FFICEditorAttributeBool::HasChanged(FICFrame Time) const {
	return Attribute->GetValue(Time) != GetActiveValue();
}

const FFICAttribute& FFICEditorAttributeBool::GetAttributeConst() const {
	return *Attribute;
}

void FFICEditorAttributeBool::UpdateValue(FICFrame Time) {
	SetActiveValue(Attribute->GetValue(Time));
}

float FFICEditorAttributeBool::GetValue(FICFrame InFrame) const {
	return Attribute->GetValue(InFrame) ? 1.0 : 0.0;
}

void FFICEditorAttributeBool::SetKeyframe(FFICValueTime InValueFrame, EFICKeyframeType InType, bool bCreate) {
	if (!bCreate && !Attribute->GetKeyframes().Contains(InValueFrame.Frame)) return;
	FFICKeyframeBool Keyframe;
	Keyframe.SetValue(InValueFrame.Value);
	Attribute->SetKeyframe(InValueFrame.Frame, Keyframe);
}

TMap<FString, TSharedRef<FFICEditorAttributeBase>> FFICEditorAttributeBool::GetChildAttributes() {
	return {};
}

TSharedRef<SWidget> FFICEditorAttributeBool::CreateDetailsWidget(UFICEditorContext* Context) {
	return SNew(SHorizontalBox)
	+SHorizontalBox::Slot()[
		SNew(SCheckBox)
		.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
			SetActiveValue(State == ECheckBoxState::Checked);
		})
		.IsChecked_Lambda([this]() {
			return GetActiveValue() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		})
	]
	+SHorizontalBox::Slot()[
		SNew(SFICKeyframeControl, Context, SharedThis(this))
		.Frame_Lambda([Context]() {
			return Context->GetCurrentFrame();
		})
	];
}

void FFICEditorAttributeBool::SetActiveValue(bool InValue) {
	ActiveValue = InValue;
	OnValueChanged.Broadcast();
}

bool FFICEditorAttributeBool::GetActiveValue() const {
	return ActiveValue;
}
