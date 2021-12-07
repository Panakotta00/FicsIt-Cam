#include "FICEditorAttributeBase.h"

void FFICEditorAttributeBase::RemoveKeyframe(int64 Time) {
	FFICAttribute* Attrib = GetAttribute();
	if (!Attrib) return;
	Attrib->RemoveKeyframe(Time);
}

TSharedPtr<FFICKeyframeRef> FFICEditorAttributeBase::GetKeyframe(int64 Time) {
	if (!GetAttributeConst()) return nullptr;
	TMap<int64, TSharedPtr<FFICKeyframeRef>> Keyframes = GetAttribute()->GetKeyframes();
	TSharedPtr<FFICKeyframeRef>* KF = Keyframes.Find(Time);
	if (KF) return *KF;
	return nullptr;
}

bool FFICEditorAttributeBase::IsAnimated() {
	if (!GetAttributeConst()) return nullptr;
	return GetAttribute()->GetKeyframes().Num() > 0;
}

int64 FFICEditorAttributeBase::GetFrame() const {
	return Frame;
}

void FFICEditorAttributeBase::SetFrame(int64 inFrame) {
	Frame = inFrame;
	UpdateValue();
}

void FFICEditorGroupAttribute::SetKeyframe(int64 Time) {
	for (const TPair<FString, TAttribute<FFICEditorAttributeBase*>>& Attrib : Attributes) {
		FFICEditorAttributeBase* Base = Attrib.Value.Get();
		if (Base) Base->SetKeyframe(Time);
	}
}

void FFICEditorGroupAttribute::RemoveKeyframe(int64 Time) {
	for (const TPair<FString, TAttribute<FFICEditorAttributeBase*>>& Attrib : Attributes) {
		FFICEditorAttributeBase* Base = Attrib.Value.Get();
		if (Base) Base->RemoveKeyframe(Time);
	} 
}

bool FFICEditorGroupAttribute::HasChanged(int64 Time) const {
	for (const TPair<FString, TAttribute<FFICEditorAttributeBase*>>& Attrib : Attributes) {
		FFICEditorAttributeBase* Base = Attrib.Value.Get();
		if (Base && Base->HasChanged(Time)) return true;
	}
	return false;
}

const FFICAttribute* FFICEditorGroupAttribute::GetAttributeConst() const {
	return &All;
}

void FFICEditorGroupAttribute::UpdateValue() {
	for (const TPair<FString, TAttribute<FFICEditorAttributeBase*>>& Attrib : Attributes) {
		FFICEditorAttributeBase* Base = Attrib.Value.Get();
		if (Base) Base->UpdateValue();
	}
}

float FFICEditorGroupAttribute::GetValueAsFloat(int64 InFrame) const {
	float Sum = 0.0f;
	for (const TTuple<FString, TAttribute<FFICEditorAttributeBase*>>& Attribute : Attributes) {
		Sum += Attribute.Value.Get()->GetValueAsFloat(InFrame);
	}
	return Sum / Attributes.Num();
}

void FFICEditorGroupAttribute::SetKeyframeFromFloat(int64 InFrame, float InValue, EFICKeyframeType InType, bool bCreate) {
	for (const TTuple<FString, TAttribute<FFICEditorAttributeBase*>>& Attribute : Attributes) {
		if (!bCreate && !Attribute.Value.Get()->GetKeyframe(InFrame)) continue;
		Attribute.Value.Get()->SetKeyframeFromFloat(InFrame, Attribute.Value.Get()->GetValueAsFloat(InFrame), InType);
	}
}

TMap<FString, TAttribute<FFICEditorAttributeBase*>> FFICEditorGroupAttribute::GetChildAttributes() {
	return Attributes;
}
