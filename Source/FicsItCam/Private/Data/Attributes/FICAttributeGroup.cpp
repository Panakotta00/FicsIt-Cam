#include "FicsItCam/Public/Data/Attributes/FICAttributeGroup.h"

TMap<FICFrame, TSharedRef<FFICKeyframe>> FFICGroupAttribute::GetKeyframes() {
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes;
	for (const TPair<FString, TAttribute<FFICAttribute*>>& Attr : Children) {
		FFICAttribute* Attrib = Attr.Value.Get();
		if (Attrib) for (const TPair<int64, TSharedRef<FFICKeyframe>>& Keyframe : Attrib->GetKeyframes()) {
			FFICKeyframe KF;
			KF.KeyframeType = FIC_KF_CUSTOM;
			Keyframes.Add(Keyframe.Key, MakeShared<FFICKeyframe>(KF));
		}
	}
	return Keyframes;
}

EFICKeyframeType FFICGroupAttribute::GetAllowedKeyframeTypes() const {
	return FIC_KF_NONE;
}

TSharedRef<FFICKeyframe> FFICGroupAttribute::AddKeyframe(FICFrame Time) {
	for (const TPair<FString, TAttribute<FFICAttribute*>>& Attr : Children) {
		FFICAttribute* Attrib = Attr.Value.Get();
		if (Attrib) Attrib->AddKeyframe(Time);
	}
	FFICKeyframe KF;
	KF.KeyframeType = FIC_KF_CUSTOM;
	return MakeShared<FFICKeyframe>(KF);
}

void FFICGroupAttribute::RemoveKeyframe(FICFrame Time) {
	for (const TPair<FString, TAttribute<FFICAttribute*>>& Attr : Children) {
		FFICAttribute* Attrib = Attr.Value.Get();
		if (Attrib) Attrib->RemoveKeyframe(Time);
	}
}

void FFICGroupAttribute::MoveKeyframe(FICFrame From, FICFrame To) {
	for (const TPair<FString, TAttribute<FFICAttribute*>>& Attr : Children) {
		FFICAttribute* Attrib = Attr.Value.Get();
		if (Attrib) Attrib->MoveKeyframe(From, To);
	}
}

void FFICGroupAttribute::RecalculateKeyframe(FICFrame Time) {
	for (const TPair<FString, TAttribute<FFICAttribute*>>& Attr : Children) {
		FFICAttribute* Attrib = Attr.Value.Get();
		if (Attrib) Attrib->RecalculateKeyframe(Time);
	}
}

void FFICGroupAttribute::Set(TSharedRef<FFICAttribute> InAttrib) {
	TSharedRef<FFICGroupAttribute> Attrib = StaticCastSharedRef<FFICGroupAttribute>(InAttrib);
	for (const TPair<FString, TAttribute<FFICAttribute*>>& Attr : Children) {
		TSharedRef<FFICAttribute>* Attribute = Attrib->AttributeCache.Find(Attr.Key);
		if (Attribute) Attr.Value.Get()->Set(*Attribute);
	}
}

TSharedRef<FFICAttribute> FFICGroupAttribute::Get() {
	TSharedRef<FFICGroupAttribute> Attrib = MakeShared<FFICGroupAttribute>();
	for (const TPair<FString, TAttribute<FFICAttribute*>>& Attr : Children) {
		Attrib->AttributeCache.Add(Attr.Key, Attr.Value.Get()->Get());
	}
	return Attrib;
}
