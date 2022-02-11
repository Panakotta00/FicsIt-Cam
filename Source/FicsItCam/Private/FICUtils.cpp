#include "FICUtils.h"
#include "FicsItCam/Public/FICUtils.h"

#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"

FRotator UFICUtils::NormalizeRotator(FRotator Rot) {
	while (Rot.Pitch > 180.0) Rot.Pitch -= 360.0f;
	while (Rot.Pitch < -180.0) Rot.Pitch += 360.0f;
	while (Rot.Yaw > 180.0) Rot.Yaw -= 360.0f;
	while (Rot.Yaw < -180.0) Rot.Yaw += 360.0f;
	while (Rot.Roll > 180.0) Rot.Roll -= 360.0f;
	while (Rot.Roll < -180.0) Rot.Roll += 360.0f;
	return Rot;
}

FString UFICUtils::KeymappingToString(const FString& Keymapping) {
	TArray<FInputActionKeyMapping> Mappings;
	UInputSettings::GetInputSettings()->GetActionMappingByName(FName(Keymapping), Mappings);
	if (Mappings.Num() > 0) {
		FInputActionKeyMapping Map = Mappings[0];
		FString Str;
		if (Map.bCtrl) Str += TEXT("CTRL + ");
		if (Map.bShift) Str += TEXT("SHIFT + ");
		if (Map.bAlt) Str += TEXT("ALT + ");
		Str += Map.Key.GetDisplayName().ToString();
		return Str;
	}
	return TEXT("");
}

float UFICUtils::BezierInterpolate(FVector2D P0, FVector2D P1, FVector2D P2, FVector2D P3, float t) {
	float Lower = 0.0;
	float Upper = 1.0;
	float Current = 0.5;
	float CurrentT;
	float CurrentV;
	int Increments = 0;
	do {
		CurrentT = FMath::Pow(1-Current, 3) * P0.X + 3*FMath::Pow(1-Current, 2) * Current * P1.X + 3*(1-Current) * Current*Current * P2.X + Current*Current*Current * P3.X;
		CurrentV = FMath::Pow(1-Current, 3) * P0.Y + 3*FMath::Pow(1-Current, 2) * Current * P1.Y + 3*(1-Current) * Current*Current * P2.Y + Current*Current*Current * P3.Y;
		if (CurrentT < t) {
			Lower = Current;
		} else if (CurrentT > t) {
			Upper = Current;
		}
		Current = Lower + ((Upper - Lower)/2.0);
	} while (FMath::Abs(t - CurrentT) > 0.001 && Increments++ < 100);
	return CurrentV;
}
