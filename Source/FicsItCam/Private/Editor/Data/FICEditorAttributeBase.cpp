#include "FicsItCam/Public/Editor/Data/FICEditorAttributeBase.h"

#include "Data/Attributes/FICAttribute.h"

FName FFICEditorAttributeBase::GetAttributeType() const {
	return GetAttributeConst().GetAttributeType();
}

void FFICEditorAttributeBase::RemoveKeyframe(int64 Time) {
	GetAttribute().RemoveKeyframe(Time);
}

TSharedPtr<FFICKeyframe> FFICEditorAttributeBase::GetKeyframe(int64 Time) {
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = GetAttribute().GetKeyframes();
	TSharedRef<FFICKeyframe>* KF = Keyframes.Find(Time);
	if (KF) return *KF;
	return nullptr;
}

bool FFICEditorAttributeBase::IsAnimated() {
	return GetAttribute().GetKeyframes().Num() > 0;
}
