#include "FicsItCam/Public/Data/Attributes/FICAttribute.h"

void FFICAttribute::RecalculateAllKeyframes() {
	TArray<int64> Keys;
	GetKeyframes().GetKeys(Keys);
	for (int64 Time : Keys) {
		RecalculateKeyframe(Time);
	}

	OnUpdateBroadcast();
}

TSharedPtr<FFICKeyframe> FFICAttribute::GetPrevKeyframe(FICFrame Time, FICFrame& OutTime) {
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = GetKeyframes();
	TArray<FICFrame> Keys;
	Keyframes.GetKeys(Keys);
	Keys.Sort();
	if (Keys.Num() < 1) return nullptr;
	int32 Index = Keys.Find(Time);
	if (Index < 0) {
		int64 LastDiff = TNumericLimits<int64>::Max();
		for (int32 i = 0; i < Keys.Num(); ++i) {
			int64 KF = Keys[i];
			if (KF < Time && (Time - KF < LastDiff)) {
				LastDiff = Time - KF;
				Index = i;
			}
		}
		if (Index < 0) return nullptr;
	} else {
		Index -= 1;
	}
	if (Index >= 0 && Index < Keys.Num()) {
		OutTime = Keys[Index];
		return Keyframes[OutTime];
	}
	return nullptr;
}

TSharedPtr<FFICKeyframe> FFICAttribute::GetNextKeyframe(FICFrame Time, FICFrame& OutTime) {
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = GetKeyframes();
	TArray<FICFrame> Keys;
	Keyframes.GetKeys(Keys);
	Keys.Sort();
	if (Keys.Num() < 1) return nullptr;
	int32 Index = Keys.Find(Time);
	if (Index < 0) {
		int64 LastDiff = TNumericLimits<FICFrame>::Max();
		for (int32 i = 0; i < Keys.Num(); ++i) {
			int64 KF = Keys[i];
			if (KF > Time && (KF - Time < LastDiff)) {
				LastDiff = KF - Time;
				Index = i;
			}
		}
		if (Index < 0) return nullptr;
	} else {
		Index = Index+1;
	}
	if (Index >= 0 && Index < Keys.Num()) {
		OutTime = Keys[Index];
		return Keyframes[OutTime];
	}
	return nullptr;
}
