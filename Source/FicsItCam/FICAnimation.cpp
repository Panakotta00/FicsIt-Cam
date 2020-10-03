#include "FICAnimation.h"

FFICKeyframeRef::~FFICKeyframeRef() {
	if (bShouldDestroy && Keyframe) delete Keyframe;
	Keyframe = nullptr;
}

void FFICAttribute::RecalculateAllKeyframes() {
	TArray<int64> Keys;
	GetKeyframes().GetKeys(Keys);
	for (int64 Time : Keys) {
		RecalculateKeyframe(Time);
	}
}

#pragma optimize("", off)
bool FFICAttribute::GetPrevKeyframe(int64 Time, int64& outTime, TSharedPtr<FFICKeyframeRef>& outKeyframe) {
	TMap<int64, TSharedPtr<FFICKeyframeRef>> Keyframes = GetKeyframes();
	TArray<int64> Keys;
	Keyframes.GetKeys(Keys);
	Keys.Sort();
	if (Keys.Num() < 1) return false;
	int32 Index = Keys.Find(Time);
	if (Index < 0) {
		for (int32 i = 0; i < Keys.Num(); ++i) {
			if (Keys[i] > Time) {
				Index = i-1;
				break;
			}
		}
		if (Index < 0) return false;
	} else {
		Index -= 1;
	}
	if (Index > 0 && Index < Keys.Num()) {
		outTime = Keys[Index];
		outKeyframe = Keyframes[outTime];
		return true;
	}
	return false;
}
#pragma optimize("", on)

bool FFICAttribute::GetNextKeyframe(int64 Time, int64& outTime, TSharedPtr<FFICKeyframeRef>& outKeyframe) {
	TMap<int64, TSharedPtr<FFICKeyframeRef>> Keyframes = GetKeyframes();
	TArray<int64> Keys;
	Keyframes.GetKeys(Keys);
	Keys.Sort();
	if (Keys.Num() < 1) return false;
	int32 Index = Keys.Find(Time);
	if (Index < 0) {
		int64 LastDiff = 0;
		for (int32 i = Keys.Num()-1; i >= 0; --i) {
			if (Keys[i] < Time) {
				Index = i+1;
				break;
			}
		}
		if (Index < 0) return false;
	} else {
		Index = Index+1;
	}
	if (Index > 0 && Index < Keys.Num()) {
		outTime = Keys[Index];
		outKeyframe = Keyframes[outTime];
		return true;
	}
	return false;
}

float Interpolate(FVector2D P0, FVector2D P1, FVector2D P2, FVector2D P3, float t) {
	float Lower = 0.0;
	float Upper = 1.0;
	float Current = 0.5;
	float CurrentT;
	float CurrentV;
	do {
		CurrentT = FMath::Pow(1-Current, 3) * P0.X + 3*FMath::Pow(1-Current, 2) * Current * P1.X + 3*(1-Current) * Current*Current * P2.X + Current*Current*Current * P3.X;
		CurrentV = FMath::Pow(1-Current, 3) * P0.Y + 3*FMath::Pow(1-Current, 2) * Current * P1.Y + 3*(1-Current) * Current*Current * P2.Y + Current*Current*Current * P3.Y;
		if (CurrentT < t) {
			Lower = Current;
		} else if (CurrentT > t) {
			Upper = Current;
		}
		Current = Lower + ((Upper - Lower)/2.0);
	} while (FMath::Abs(t - CurrentT) > 0.001);
	return CurrentV;
}

TMap<int64, TSharedPtr<FFICKeyframeRef>> FFICFloatAttribute::GetKeyframes() {
	TMap<int64, TSharedPtr<FFICKeyframeRef>> OutKeyframes;
	TArray<int64> Keys;
	Keyframes.GetKeys(Keys);
	for (int64 Key : Keys) OutKeyframes.Add(Key, MakeShared<FFICKeyframeRef>(Keyframes.Find(Key)));
	return OutKeyframes;
}

EFICKeyframeType FFICFloatAttribute::GetAllowedKeyframeTypes() const {
	return FIC_KF_ALL;
}

TSharedPtr<FFICKeyframeRef> FFICFloatAttribute::AddKeyframe(int64 Time) {
	if (Keyframes.Contains(Time)) return MakeShared<FFICKeyframeRef>(&Keyframes[Time]);
	FFICFloatKeyframe Keyframe;
	Keyframe.KeyframeType = FIC_KF_EASE;
	Keyframe.Value = GetValue(Time);
	return MakeShared<FFICKeyframeRef>(SetKeyframe(Time, Keyframe));
}

void FFICFloatAttribute::RemoveKeyframe(int64 Time) {
	Keyframes.Remove(Time);
}

void FFICFloatAttribute::MoveKeyframe(int64 From, int64 To) {
	if (From == To) return;
	FFICFloatKeyframe* FromKeyframe = Keyframes.Find(From);
	if (!FromKeyframe) return;
	SetKeyframe(To, *FromKeyframe);
	RemoveKeyframe(From);
}

void FFICFloatAttribute::RecalculateKeyframe(int64 Time) {
	TArray<int64> Keys;
	Keyframes.GetKeys(Keys);
	Keys.Sort();
	int32 Index = Keys.Find(Time);
	if (Index < 0) return;
	
	FFICFloatKeyframe* PK = nullptr;
	int64 PTime = 0;
	if (Index > 0 && Keys.Num() > 1) {
		PTime = Keys[Index-1];
		PK = &Keyframes[PTime];
	}
	FFICFloatKeyframe* K = nullptr;
	if (Index >= 0 && Index < Keys.Num()) {
		K = &Keyframes[Keys[Index]];
	}
	FFICFloatKeyframe* NK = nullptr;
	int64 NTime = 0;
	if (Index < Keys.Num()-1) {
		NTime = Keys[Index+1];
		NK = &Keyframes[NTime];
	}

	if (!K) return;
	if (K->KeyframeType & (FIC_KF_CUSTOM | FIC_KF_LINEAR | FIC_KF_MIRROR | FIC_KF_STEP)) return;
	if (PK) {
		float PKTimeDiff = Time - PTime;
		float PKValueDiff = K->Value - PK->Value;
		if (NK) {
			float NKTimeDiff = NTime - Time;
			float NKValueDiff = NK->Value - K->Value;
			float KTimeDiff = (PKTimeDiff + NKTimeDiff) / 2;
			float KValueDiff = (PKValueDiff + NKValueDiff) / 2;

			if (K->KeyframeType == FIC_KF_EASE) {
				K->InTanTime = K->OutTanTime = KTimeDiff / 2;
				K->InTanValue = K->OutTanValue = KValueDiff / 2;
			} else if (K->KeyframeType == FIC_KF_EASEINOUT) {
				K->OutTanTime = NKTimeDiff / 2;
				K->InTanTime = PKTimeDiff / 2;
				K->OutTanValue = 0;
			}
		} else {
			if (K->KeyframeType == FIC_KF_EASE) {
				K->InTanTime = PKTimeDiff / 2;
				K->InTanValue = PKValueDiff / 2;
			} else if (K->KeyframeType == FIC_KF_EASEINOUT) {
				K->InTanTime = PKTimeDiff / 2;
				K->InTanValue = 0;
			}
		}
	} else {
		if (NK) {
			float NKTimeDiff = NTime - Time;
			float NKValueDiff = NK->Value - K->Value;
		
			if (K->KeyframeType == FIC_KF_EASE) {
				K->OutTanTime = NKTimeDiff / 2;
				K->OutTanValue = NKValueDiff / 2;
			} else if (K->KeyframeType == FIC_KF_EASEINOUT) {
				K->OutTanTime = NKTimeDiff / 2;
				K->OutTanValue = 0;
			}
		}
	}
}

FFICFloatKeyframe* FFICFloatAttribute::SetKeyframe(int64 Time, FFICFloatKeyframe Keyframe) {
	FFICFloatKeyframe* KF = &Keyframes.FindOrAdd(Time);
	*KF = Keyframe;
	return KF;
}

float FFICFloatAttribute::GetValue(float Time) {
	float Time1 = -1;
	FFICFloatKeyframe KF1;
	float Time2 = -1;
	FFICFloatKeyframe KF2;
	for (const TPair<int64, FFICFloatKeyframe>& Keyframe : Keyframes) {
		if (Keyframe.Key < Time) {
			if (Time1 < 0 || Time1 < Keyframe.Key) {
				Time1 = Keyframe.Key;
				KF1 = Keyframe.Value;
			}
		} else if (Keyframe.Key >= Time) {
			if (Time2 < 0 || Time2 > Keyframe.Key) {
				Time2 = Keyframe.Key;
				KF2 = Keyframe.Value;
			}
		}
	}

	float Interpolated = FallBackValue;
	if (Time1 >= 0) {
		if (Time2 >= 0) {
			float Factor = (Time - Time1) / (Time2 - Time1);
			if (KF2.KeyframeType == FIC_KF_STEP) {
				Interpolated = KF1.Value;
			} else if (KF1.KeyframeType == FIC_KF_LINEAR) {
				Interpolated = FMath::Lerp(KF1.Value, KF2.Value, Factor);
			} else {
				Interpolated = Interpolate({Time1, KF1.Value}, {Time1 + KF1.OutTanTime, KF1.Value + KF1.OutTanValue},
                 {Time2 - KF2.InTanTime, KF2.Value - KF2.InTanValue}, {Time2, KF2.Value}, Time);
			}
		} else {
			return KF1.Value;
		}
	} else {
		if (Time2 >= 0) {
			return KF2.Value;
		}
	}
	return Interpolated;
}

TMap<int64, TSharedPtr<FFICKeyframeRef>> FFICGroupAttribute::GetKeyframes() {
	TMap<int64, TSharedPtr<FFICKeyframeRef>> Keyframes;
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		for (const TPair<int64, TSharedPtr<FFICKeyframeRef>>& Keyframe : Attr.Value->GetKeyframes()) {
			FFICKeyframe* KF = new FFICKeyframe();
			KF->KeyframeType = FIC_KF_CUSTOM;
			Keyframes.Add(Keyframe.Key, MakeShared<FFICKeyframeRef>(KF, true));
		}
	}
	return Keyframes;
}

EFICKeyframeType FFICGroupAttribute::GetAllowedKeyframeTypes() const {
	return FIC_KF_NONE;
}

TSharedPtr<FFICKeyframeRef> FFICGroupAttribute::AddKeyframe(int64 Time) {
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attr.Value->AddKeyframe(Time);
	}
	FFICKeyframe* KF = new FFICKeyframe();
	KF->KeyframeType = FIC_KF_CUSTOM;
	return MakeShared<FFICKeyframeRef>(KF, true);
}

void FFICGroupAttribute::RemoveKeyframe(int64 Time) {
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attr.Value->RemoveKeyframe(Time);
	}
}

void FFICGroupAttribute::MoveKeyframe(int64 From, int64 To) {
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attr.Value->MoveKeyframe(From, To);
	}
}

void FFICGroupAttribute::RecalculateKeyframe(int64 Time) {
	for (const TPair<FString, FFICAttribute*>& Attr : Children) {
		Attr.Value->RecalculateKeyframe(Time);
	}
}

AFICAnimation::AFICAnimation() {
	FOV.FallBackValue = 90.0f;
}

void AFICAnimation::RecalculateAllKeyframes() {
	PosX.RecalculateAllKeyframes();
	PosY.RecalculateAllKeyframes();
	PosZ.RecalculateAllKeyframes();
	RotYaw.RecalculateAllKeyframes();
	RotPitch.RecalculateAllKeyframes();
	RotRoll.RecalculateAllKeyframes();
	FOV.RecalculateAllKeyframes();
}

int64 AFICAnimation::GetEndOfAnimation() {
	TArray<int64> Keyframes;
	PosX.GetKeyframes().GetKeys(Keyframes);
	PosY.GetKeyframes().GetKeys(Keyframes);
	PosZ.GetKeyframes().GetKeys(Keyframes);
	RotPitch.GetKeyframes().GetKeys(Keyframes);
	RotYaw.GetKeyframes().GetKeys(Keyframes);
	RotRoll.GetKeyframes().GetKeys(Keyframes);
	FOV.GetKeyframes().GetKeys(Keyframes);
	Keyframes.Sort();
	return Keyframes.Num() > 0 ? Keyframes[Keyframes.Num()-1] : 0;
 }
