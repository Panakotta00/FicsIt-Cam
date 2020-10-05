#include "FICEditorAttributeBase.h"

void FFICEditorAttributeBase::RemoveKeyframe(int64 Time) {
	FFICAttribute* Attrib = GetAttribute();
	if (!Attrib) return;
	Attrib->RemoveKeyframe(Time);
}

TSharedPtr<FFICKeyframeRef> FFICEditorAttributeBase::GetKeyframe(int64 Time) {
	if (!GetAttribute()) return nullptr;
	TMap<int64, TSharedPtr<FFICKeyframeRef>> Keyframes = GetAttribute()->GetKeyframes();
	TSharedPtr<FFICKeyframeRef>* KF = Keyframes.Find(Time);
	if (KF) return *KF;
	return nullptr;
}

bool FFICEditorAttributeBase::IsAnimated() {
	if (!GetAttribute()) return nullptr;
	return GetAttribute()->GetKeyframes().Num() > 0;
}

int64 FFICEditorAttributeBase::GetFrame() {
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

bool FFICEditorGroupAttribute::HasChanged(int64 Time) {
	for (const TPair<FString, TAttribute<FFICEditorAttributeBase*>>& Attrib : Attributes) {
		FFICEditorAttributeBase* Base = Attrib.Value.Get();
		if (Base && Base->HasChanged(Time)) return true;
	}
	return false;
}

FFICAttribute* FFICEditorGroupAttribute::GetAttribute() {
	return &All;
}

void FFICEditorGroupAttribute::UpdateValue() {
	for (const TPair<FString, TAttribute<FFICEditorAttributeBase*>>& Attrib : Attributes) {
		FFICEditorAttributeBase* Base = Attrib.Value.Get();
		if (Base) Base->UpdateValue();
	}
}
