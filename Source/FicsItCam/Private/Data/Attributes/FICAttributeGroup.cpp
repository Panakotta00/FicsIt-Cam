#include "FicsItCam/Public/Data/Attributes/FICAttributeGroup.h"

#include "Editor/Data/FICEditorAttributeGroup.h"

TMap<FICFrame, TSharedRef<FFICKeyframe>> FFICGroupAttribute::GetKeyframes() {
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes;
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		for (const TPair<int64, TSharedRef<FFICKeyframe>>& Keyframe : Attr.Value->GetKeyframes()) {
			FFICKeyframe KF;
			KF.KeyframeType = FIC_KF_CUSTOM;
			Keyframes.Add(Keyframe.Key, MakeShared<FFICKeyframe>(KF));
		}
	}
	return Keyframes;
}

FFICGroupAttribute::~FFICGroupAttribute() {
	// TODO: Maybe remove Update Delegates from Children
}

EFICKeyframeType FFICGroupAttribute::GetAllowedKeyframeTypes() const {
	return FIC_KF_NONE;
}

TSharedRef<FFICKeyframe> FFICGroupAttribute::AddKeyframe(FICFrame Time) {
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attr.Value->AddKeyframe(Time);
	}
	FFICKeyframe KF;
	KF.KeyframeType = FIC_KF_CUSTOM;
	return MakeShared<FFICKeyframe>(KF);
}

void FFICGroupAttribute::RemoveKeyframe(FICFrame Time) {
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attr.Value->RemoveKeyframe(Time);
	}
}

void FFICGroupAttribute::MoveKeyframe(FICFrame From, FICFrame To) {
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attr.Value->MoveKeyframe(From, To);
	}
}

void FFICGroupAttribute::RecalculateKeyframe(FICFrame Time) {
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attr.Value->RecalculateKeyframe(Time);
	}
}

void FFICGroupAttribute::Set(TSharedRef<FFICAttribute> InAttrib) {
	TSharedRef<FFICGroupAttribute> Attrib = StaticCastSharedRef<FFICGroupAttribute>(InAttrib);
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		TSharedRef<FFICAttribute>* Attribute = Attrib->AttributeCache.Find(Attr.Key);
		if (Attribute) Attr.Value->Set(*Attribute);
	}
}

TSharedRef<FFICAttribute> FFICGroupAttribute::Get() {
	TSharedRef<FFICGroupAttribute> Attrib = MakeShared<FFICGroupAttribute>();
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attrib->AttributeCache.Add(Attr.Key, Attr.Value->Get());
	}
	return Attrib;
}

TSharedRef<FFICEditorAttributeBase> FFICGroupAttribute::CreateEditorAttribute() {
	return MakeShared<FFICEditorAttributeGroup>(*this);
}

void FFICGroupAttribute::AddChildAttribute(FString Name, FFICAttribute* Attribute) {
	Children.Add(Name, Attribute);
	// TODO: Maybe add Update Delegate to Attribute
}

void FFICGroupAttribute::RemoveChildAttribute(FString Name) {
	Children.Remove(Name);
	// TODO: Maybe add Update Delegate to Attribute
}
