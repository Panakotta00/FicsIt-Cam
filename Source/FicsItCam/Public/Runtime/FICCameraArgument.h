#pragma once

#include "FGGameUserSettings.h"
#include "FICUtils.h"
#include "Command/CommandSender.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/FICCameraReference.h"
#include "Runtime/FICCaptureCamera.h"
#include "FICCameraArgument.generated.h"

USTRUCT(BlueprintType)
struct FFICCameraArgument {
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite, meta=(ExposeOnSpawn))
	FFICCameraReference CameraReference = FFICCameraReference();

	UPROPERTY(SaveGame, BlueprintReadWrite, meta=(ExposeOnSpawn))
	FFICCameraSettingsSnapshot CameraSettingsSnapshot = FFICCameraSettingsSnapshot();

	UPROPERTY(SaveGame, BlueprintReadWrite, meta=(ExposeOnSpawn))
	FVector2D Resolution = FVector2D(256, 256);

	UPROPERTY(SaveGame, BlueprintReadWrite, meta=(ExposeOnSpawn))
	FVector2D SensorDimensions = FVector2D(23.76, 13.365);

	UPROPERTY(SaveGame, BlueprintReadWrite, meta=(ExposeOnSpawn))
	bool bUseCinematic = false;

	UPROPERTY(SaveGame, BlueprintReadWrite, meta=(ExposeOnSpawn))
	FString Name = TEXT("");

	FVector2D GetResolution(UObject* WorldContext) const {
		AFICScene* Scene = CameraReference.GetScene(WorldContext);
		if (!Scene) return Resolution;
		return FVector2D(Scene->ResolutionWidth, Scene->ResolutionHeight);
	}

	FVector2D GetSensorDimensions(UObject* WorldContext) const {
		AFICScene* Scene = CameraReference.GetScene(WorldContext);
		if (!Scene) return SensorDimensions;
		return Scene->SensorDimension;
	}

	bool GetUseCinematic(UObject* WorldContext) const {
		AFICScene* Scene = CameraReference.GetScene(WorldContext);
		if (!Scene) return bUseCinematic;
		return Scene->bUseCinematic;
	}

	FFICCameraSettingsSnapshot GetCameraSettingsSnapshot(UObject* WorldContext) const {
		FFICCameraSettingsSnapshot Snapshot = CameraReference.GetSnapshot(WorldContext);
		if (!Snapshot.IsValid()) return CameraSettingsSnapshot;
		return Snapshot;
	}

	FString GetName() const {
		if (CameraReference.IsValid(nullptr)) return CameraReference.ToString();
		return Name;
	}

	FString GetSimpleName() const {
		FString Val = GetName();
		Val.ReplaceInline(TEXT("#"), TEXT("S-"));
		Val.ReplaceInline(TEXT(">"), TEXT("A-"));
		Val.ReplaceInline(TEXT("~"), TEXT("-"));
		Val.ReplaceInline(TEXT(":"), TEXT("-"));
		return Val;
	}

	void InitalizeCaptureCamera(AFICCaptureCamera* CaptureCamera) const {
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

	void UpdateCameraSettings(AFICCaptureCamera* CaptureCamera) const {
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

UCLASS()
class UFICCameraArgumentLib : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContext"))
	static FVector2D GetResolution(UObject* WorldContext, const FFICCameraArgument& CameraArgument) {
		return CameraArgument.GetResolution(WorldContext);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContext"))
	static FVector2D GetSensorDimensions(UObject* WorldContext, const FFICCameraArgument& CameraArgument) {
		return CameraArgument.GetSensorDimensions(WorldContext);
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContext"))
	static bool GetUseCinematic(UObject* WorldContext, const FFICCameraArgument& CameraArgument) {
		return CameraArgument.GetUseCinematic(WorldContext);
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContext"))
	static FFICCameraSettingsSnapshot GetCameraSettingsSnapshot(UObject* WorldContext, const FFICCameraArgument& CameraArgument) {
		return CameraArgument.GetCameraSettingsSnapshot(WorldContext);
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FString GetName(const FFICCameraArgument& CameraArgument) {
		return CameraArgument.GetName();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FString GetSimpleName(const FFICCameraArgument& CameraArgument) {
		return CameraArgument.GetSimpleName();
	}

	UFUNCTION(BlueprintCallable)
	static void InitalizeCaptureCamera(const FFICCameraArgument& CameraArgument, AFICCaptureCamera* CaptureCamera) {
		return CameraArgument.InitalizeCaptureCamera(CaptureCamera);
	}

	UFUNCTION(BlueprintCallable)
	static void UpdateCameraSettings(const FFICCameraArgument& CameraArgument, AFICCaptureCamera* CaptureCamera) {
		return CameraArgument.UpdateCameraSettings(CaptureCamera);
	}
};
