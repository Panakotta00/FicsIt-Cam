#pragma once

#include "FGGameUserSettings.h"
#include "FICUtils.h"
#include "Command/CommandSender.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/FICCameraReference.h"
#include "Runtime/FICCaptureCamera.h"
#include "FICCameraArgument.generated.h"

USTRUCT()
struct FFICCameraArgument {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FFICCameraReference CameraReference = FFICCameraReference();

	UPROPERTY(SaveGame)
	FFICCameraSettingsSnapshot CameraSettingsSnapshot = FFICCameraSettingsSnapshot();

	UPROPERTY(SaveGame)
	FVector2D Resolution = FVector2D(256, 256);

	UPROPERTY(SaveGame)
	FVector2D SensorDimensions = FVector2D(23.76, 13.365);

	UPROPERTY(SaveGame)
	bool bUseCinematic = false;

	UPROPERTY(SaveGame)
	FString Name = TEXT("");

	FVector2D GetResolution(UObject* WorldContext) {
		AFICScene* Scene = CameraReference.GetScene(WorldContext);
		if (!Scene) return Resolution;
		return FVector2D(Scene->ResolutionWidth, Scene->ResolutionHeight);
	}

	FVector2D GetSensorDimensions(UObject* WorldContext) {
		AFICScene* Scene = CameraReference.GetScene(WorldContext);
		if (!Scene) return SensorDimensions;
		return Scene->SensorDimension;
	}

	bool GetUseCinematic(UObject* WorldContext) {
		AFICScene* Scene = CameraReference.GetScene(WorldContext);
		if (!Scene) return bUseCinematic;
		return Scene->bUseCinematic;
	}

	FFICCameraSettingsSnapshot GetCameraSettingsSnapshot(UObject* WorldContext) {
		FFICCameraSettingsSnapshot Snapshot = CameraReference.GetSnapshot(WorldContext);
		if (!Snapshot.IsValid()) return CameraSettingsSnapshot;
		return Snapshot;
	}

	FString GetName() {
		if (CameraReference.IsValid(nullptr)) return CameraReference.ToString();
		return Name;
	}

	FString GetSimpleName() {
		FString Val = GetName();
		Val.ReplaceInline(TEXT("#"), TEXT("S-"));
		Val.ReplaceInline(TEXT(">"), TEXT("A-"));
		Val.ReplaceInline(TEXT("~"), TEXT("-"));
		Val.ReplaceInline(TEXT(":"), TEXT("-"));
		return Val;
	}

	void InitalizeCaptureCamera(AFICCaptureCamera* CaptureCamera) {
		CaptureCamera->SetCamera(true, GetUseCinematic(CaptureCamera));
		FVector2D ResolutionValue = GetResolution(CaptureCamera);
		CaptureCamera->RenderTarget->ResizeTarget(ResolutionValue.X, ResolutionValue.Y);
		UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(CaptureCamera->Camera);
		if (CineCamera) {
			FVector2D SensorDimensionsValue = GetSensorDimensions(CaptureCamera);
			CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::Manual;
			CineCamera->Filmback.SensorWidth = SensorDimensionsValue.X;
			CineCamera->Filmback.SensorHeight = SensorDimensionsValue.Y;
		} else {
			CaptureCamera->Camera->SetAspectRatio(Resolution.X / Resolution.Y);
		}
	}

	void UpdateCameraSettings(AFICCaptureCamera* CaptureCamera) {
		FFICCameraSettingsSnapshot CameraSettings = GetCameraSettingsSnapshot(CaptureCamera);
		CaptureCamera->SetActorLocation(CameraSettings.Location);
		CaptureCamera->SetActorRotation(CameraSettings.Rotation);
		CaptureCamera->Camera->SetFieldOfView(CameraSettings.FOV);
		UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(CaptureCamera->Camera);
		if (CineCamera) {
			CineCamera->CurrentAperture = CameraSettings.Aperture;
			CineCamera->FocusSettings.ManualFocusDistance = CameraSettings.FocusDistance;
		}
		//CaptureCamera->CopyCameraData(CaptureCamera->Camera);
	}

	static FFICCameraArgument FromCli(UCommandSender* InSender, const FFICCameraReference& CameraRef, const FString& Name, TArray<FString> Array) {
		FFICCameraArgument Arg;
		Arg.CameraReference = CameraRef;
		if (CameraRef.IsValid(nullptr)) return Arg;
		Arg.Name = Name;
		Arg.CameraSettingsSnapshot = UFICUtils::CreateCameraSettingsSnapshotFromView(InSender);
		FIntPoint Resolution = UFGGameUserSettings::GetFGGameUserSettings()->GetScreenResolution();
		Arg.Resolution = FVector2D(Resolution.X, Resolution.Y);
		return Arg;
	}
};