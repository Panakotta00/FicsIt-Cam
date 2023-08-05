﻿#include "FICUtils.h"
#include "FicsItCam/Public/FICUtils.h"

#include "EnhancedPlayerInput.h"
#include "Editor/FICEditorSubsystem.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "Input/FGEnhancedInputComponent.h"
#include "Runtime/FICCameraReference.h"

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

FFICCameraSettingsSnapshot UFICUtils::CreateCameraSettingsSnapshotFromView(UObject* WorldContext) {
	FFICCameraSettingsSnapshot Snapshot;
	APlayerCameraManager* CameraManager = WorldContext->GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	Snapshot.Location = CameraManager->GetCameraLocation();
	Snapshot.Rotation = CameraManager->GetCameraRotation();
	Snapshot.FOV = CameraManager->GetFOVAngle();
	return Snapshot;
}

bool UFICUtils::IsValidFICObjectName(const FString& InName) {
	static FRegexPattern Pattern(TEXT("^\\w+$"));
	FRegexMatcher Match(Pattern, InName);
	if (Match.FindNext()) {
		return true;
	}
	return false;
}

bool UFICUtils::IsAction(UObject* WorldContext, const FKeyEvent& InKeyEvent, const FName& InActionName) {
	//UInputAction* Action;
	//Cast<UFGEnhancedInputComponent>(WorldContext->GetWorld()->GetFirstPlayerController()->GetCharacter()->InputComponent)->KeyBindings
	/*const FInputActionKeyMapping* Mapping = AFICEditorSubsystem::GetFICEditorSubsystem(WorldContext)->KeyMappings.Find(InActionName);
	return Mapping && Mapping->Key == InKeyEvent.GetKey() &&
		Mapping->bAlt == InKeyEvent.GetModifierKeys().IsAltDown() &&
		Mapping->bCmd == InKeyEvent.GetModifierKeys().IsCommandDown() &&
		Mapping->bCtrl == InKeyEvent.GetModifierKeys().IsControlDown() &&
		Mapping->bShift == InKeyEvent.GetModifierKeys().IsShiftDown();*/
	return false;
	// TODO: Maybe use Enhanced Input System directly, even if Editor Widget is Focused... needs some experiments
}

FRotator UFICUtils::AdditiveRotation(FRotator OldRotation, FRotator NewRotation) {
	FRotator RotOldN = OldRotation;
	while (RotOldN.Pitch < -180.0) RotOldN.Pitch += 360.0;
	while (RotOldN.Pitch > 180.0) RotOldN.Pitch -= 360.0;
	while (RotOldN.Yaw < -180.0) RotOldN.Yaw += 360.0;
	while (RotOldN.Yaw > 180.0) RotOldN.Yaw -= 360.0;
	while (RotOldN.Roll < -180.0) RotOldN.Roll += 360.0;
	while (RotOldN.Roll > 180.0) RotOldN.Roll -= 360.0;
	FRotator RotDiff = NewRotation - RotOldN;
	while (RotDiff.Pitch < -180.0) RotDiff.Pitch += 360.0;
	while (RotDiff.Pitch > 180.0) RotDiff.Pitch -= 360.0;
	while (RotDiff.Yaw < -180.0) RotDiff.Yaw += 360.0;
	while (RotDiff.Yaw > 180.0) RotDiff.Yaw -= 360.0;
	while (RotDiff.Roll < -180.0) RotDiff.Roll += 360.0;
	while (RotDiff.Roll > 180.0) RotDiff.Roll -= 360.0;
	return OldRotation + RotDiff;
}

FString UFICUtils::AdjustSceneObjectName(AFICScene* Scene, FString Name) {
	static FRegexPattern Pattern(TEXT("^(\\w+)(_[0-9]+)$"));
	FRegexMatcher Match(Pattern, Name);
	if (Match.FindNext()) Name = Match.GetCaptureGroup(1);
	int Num = 1;
	FString NewName = Name;
	while (true) {
		bool bFound = false;
		for (UObject* Object : Scene->GetSceneObjects()) {
			if (Cast<IFICSceneObject>(Object)->GetSceneObjectName() == NewName) {
				bFound = true;
				break;
			}
		}
		if (bFound) {
			NewName = FString::Printf(TEXT("%s_%i"), *Name, Num++);
		} else {
			break;
		}
	}
	return NewName;
}
