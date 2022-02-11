#include "FicsItCam/Public/Editor/FICEditorAttributeBase.h"

void FFICEditorAttributeBase::RemoveKeyframe(int64 Time) {
	FFICAttribute* Attrib = GetAttribute();
	if (!Attrib) return;
	Attrib->RemoveKeyframe(Time);
}

TSharedPtr<FFICKeyframe> FFICEditorAttributeBase::GetKeyframe(int64 Time) {
	if (!GetAttributeConst()) return nullptr;
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = GetAttribute()->GetKeyframes();
	TSharedRef<FFICKeyframe>* KF = Keyframes.Find(Time);
	if (KF) return *KF;
	return nullptr;
}

bool FFICEditorAttributeBase::IsAnimated() {
	if (!GetAttributeConst()) return nullptr;
	return GetAttribute()->GetKeyframes().Num() > 0;
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

void FFICEditorGroupAttribute::UpdateValue(FICFrame Time) {
	for (const TPair<FString, TAttribute<FFICEditorAttributeBase*>>& Attrib : Attributes) {
		FFICEditorAttributeBase* Base = Attrib.Value.Get();
		if (Base) Base->UpdateValue(Time);
	}
}

float FFICEditorGroupAttribute::GetValue(FICFrame InFrame) const {
	float Sum = 0.0f;
	for (const TTuple<FString, TAttribute<FFICEditorAttributeBase*>>& Attribute : Attributes) {
		Sum += Attribute.Value.Get()->GetValue(InFrame);
	}
	return Sum / Attributes.Num();
}

void FFICEditorGroupAttribute::SetKeyframe(FFICValueTime InValueFrame, EFICKeyframeType InType, bool bCreate) {
	for (const TTuple<FString, TAttribute<FFICEditorAttributeBase*>>& Attribute : Attributes) {
		if (!bCreate && !Attribute.Value.Get()->GetKeyframe(InValueFrame.Frame)) continue;
		Attribute.Value.Get()->SetKeyframe(InValueFrame, InType);
	}
}

TMap<FString, TAttribute<FFICEditorAttributeBase*>> FFICEditorGroupAttribute::GetChildAttributes() {
	return Attributes;
}
