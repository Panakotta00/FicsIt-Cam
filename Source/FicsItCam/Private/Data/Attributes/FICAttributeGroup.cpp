#include "Data/Attributes/FICAttributeGroup.h"

#include "Editor/Data/FICEditorAttributeGroup.h"

TMap<FICFrame, TSharedRef<FFICKeyframe>> FFICGroupAttribute::GetKeyframes() {
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes;
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		for (const TPair<int64, TSharedRef<FFICKeyframe>>& Keyframe : Attr.Value->GetKeyframes()) {
			Keyframes.Add(Keyframe.Key, MakeShared<FFICKeyframeGroup>(this, Keyframe.Key));
		}
	}
	return Keyframes;
}

void FFICKeyframeGroup::SetType(EFICKeyframeType Type) {
	for (TPair<FString, FFICAttribute*> Child : Attribute->Children) {
		TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = Child.Value->GetKeyframes();
		TSharedRef<FFICKeyframe>* KF = Keyframes.Find(Frame);
		if (KF) {
			(*KF)->SetType(Type);
		}
	}
}

FFICGroupAttribute::~FFICGroupAttribute() {
	for (const TPair<FString, FFICAttribute*>& Attrib : Children) {
		Attrib.Value->OnUpdate.Remove(UpdateDelegateHandles[Attrib.Key]);
	}
}

EFICKeyframeType FFICGroupAttribute::GetAllowedKeyframeTypes() const {
	return FIC_KF_NONE;
}

TSharedRef<FFICKeyframe> FFICGroupAttribute::AddKeyframe(FICFrame Time) {
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attr.Value->AddKeyframe(Time);
	}
	return MakeShared<FFICKeyframeGroup>(this, Time);
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

bool FFICGroupAttribute::HasKeyframe(FICFrame Time) const {
	for (TTuple<FString, FFICAttribute*> Child : Children) {
		if (Child.Value->HasKeyframe(Time)) return true;
	}
	return false;
}

void FFICGroupAttribute::CopyFrom(TSharedRef<FFICAttribute> InAttrib) {
	TSharedRef<FFICGroupAttribute> Attrib = StaticCastSharedRef<FFICGroupAttribute>(InAttrib);
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		TSharedRef<FFICAttribute>* Attribute = Attrib->AttributeCache.Find(Attr.Key);
		if (Attribute) Attr.Value->CopyFrom(*Attribute);
	}
}

TSharedRef<FFICAttribute> FFICGroupAttribute::CreateCopy() {
	TSharedRef<FFICGroupAttribute> Attrib = MakeShared<FFICGroupAttribute>();
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attrib->AttributeCache.Add(Attr.Key, Attr.Value->CreateCopy());
	}
	return Attrib;
}

TSharedRef<FFICEditorAttributeBase> FFICGroupAttribute::CreateEditorAttribute() {
	return MakeShared<FFICEditorAttributeGroup>(*this);
}

const TMap<FString, FFICAttribute*>& FFICGroupAttribute::GetChildAttributes() const {
	return Children;
}

void FFICGroupAttribute::AddChildAttribute(FString Name, FFICAttribute* Attribute) {
	Children.Add(Name, Attribute);
	UpdateDelegateHandles.Add(Name, Attribute->OnUpdate.AddLambda([this]() {
		OnUpdateBroadcast();
	}));
}

void FFICGroupAttribute::RemoveChildAttribute(FString Name) {
	Children[Name]->OnUpdate.Remove(UpdateDelegateHandles[Name]);
	Children.Remove(Name);
	UpdateDelegateHandles.Remove(Name);
}
