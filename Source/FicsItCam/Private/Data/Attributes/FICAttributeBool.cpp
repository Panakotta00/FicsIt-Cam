#include "Data/Attributes/FICAttributeBool.h"

#include "Editor/Data/FICEditorAttributeBool.h"

EFICKeyframeType FFICAttributeBool::GetAllowedKeyframeTypes() const {
	return FIC_KF_STEP;
}

TMap<FICFrame, TSharedRef<FFICKeyframe>> FFICAttributeBool::GetKeyframes() {
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Map;
	for (TTuple<int64, FFICKeyframeBool> Keyframe : Keyframes) {
		Map.Add(Keyframe.Key, MakeShared<FFICKeyframeBoolTrampoline>(this, Keyframe.Key));
	}
	return Map;
}

TSharedRef<FFICKeyframe> FFICAttributeBool::AddKeyframe(FICFrame Time) {
	if (!Keyframes.Contains(Time)) {
		SetKeyframe(Time, FFICKeyframeBool(GetValue(Time)));
	}
	return MakeShared<FFICKeyframeBoolTrampoline>(this, Time);
}

void FFICAttributeBool::RemoveKeyframe(FICFrame Time) {
	Keyframes.Remove(Time);
	OnUpdate.Broadcast();
}

void FFICAttributeBool::MoveKeyframe(FICFrame From, FICFrame To) {
	if (From == To) return;
	FFICKeyframeBool* FromKeyframe = Keyframes.Find(From);
	if (!FromKeyframe) return;
	SetKeyframe(To, *FromKeyframe);
	RemoveKeyframe(From);
}

void FFICAttributeBool::RecalculateKeyframe(FICFrame Time) {
	for (TTuple<int64, FFICKeyframeBool>& keyframe : Keyframes) {
		keyframe.Value.KeyframeType = FIC_KF_STEP;
	}
	OnUpdate.Broadcast();
}

FICValue FFICAttributeBool::GetFloatValue(FICFrameFloat Time) {
	return GetValue(Time) ? 1.0f : 0.0f;
}

void FFICAttributeBool::Set(TSharedRef<FFICAttribute> InAttrib) {
	FOnUpdate OnUpdateBuf = OnUpdate;
	if (InAttrib->GetAttributeType() == GetAttributeType()) {
		*this = *StaticCastSharedRef<FFICAttributeBool>(InAttrib);
	}
	OnUpdate = OnUpdateBuf;
	OnUpdate.Broadcast();
}

TSharedRef<FFICAttribute> FFICAttributeBool::Get() {
	return MakeShared<FFICAttributeBool>(*this);
}

TSharedRef<FFICEditorAttributeBase> FFICAttributeBool::CreateEditorAttribute() {
	return MakeShared<FFICEditorAttributeBool>(this);
}

FFICKeyframeBool* FFICAttributeBool::SetKeyframe(FICFrame Time, FFICKeyframeBool Keyframe) {
	FFICKeyframeBool* KF = &Keyframes.FindOrAdd(Time);
	*KF = Keyframe;
	OnUpdate.Broadcast();
	return KF;
}

bool FFICAttributeBool::GetValue(FICFrameFloat Time) {
	FICFrame Frame = TNumericLimits<FICFrame>::Min();
	bool Value = FallBackValue;
	for (TTuple<FICFrame, FFICKeyframeBool> Keyframe : Keyframes) {
		if (Keyframe.Key > Frame && Keyframe.Key <= Time) {
			Frame = Keyframe.Key;
			Value = Keyframe.Value.Value;
		}
	}
	return Value;
}
