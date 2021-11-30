#include "FICUtils.h"

#include "FGGameUserSettings.h"
#include "FicsItCamModule.h"
#include "GameFramework/InputSettings.h"

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
