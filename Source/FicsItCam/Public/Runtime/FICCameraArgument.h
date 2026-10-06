#pragma once

#include "Misc/DefaultValueHelper.h"
#include "FGGameUserSettings.h"
#include "FICUtils.h"
#include "GenericPlatform/GenericPlatformHttp.h"
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
	FMinimalViewInfo ViewInfo;

	UPROPERTY(SaveGame, BlueprintReadWrite, meta=(ExposeOnSpawn))
	FVector2D Resolution = FVector2D(256, 256);

	UPROPERTY(SaveGame, BlueprintReadWrite, meta=(ExposeOnSpawn))
	FString Name = TEXT("");

	FVector2D GetResolution(UObject* WorldContext) const {
		AFICScene* Scene = CameraReference.GetScene(WorldContext);
		FVector2D res = Resolution;
		if (Scene) res = FVector2D(Scene->ResolutionWidth, Scene->ResolutionHeight);
		if (auto option = FGenericPlatformHttp::GetUrlParameter(CameraReference.GetData(), TEXT("ResX"))) {
			int resX = res.X;
			if (FDefaultValueHelper::ParseInt(*option, resX)) {
				res.X = resX;
			}
		}
		if (auto option = FGenericPlatformHttp::GetUrlParameter(CameraReference.GetData(), TEXT("ResY"))) {
			int resY = res.Y;
			if (FDefaultValueHelper::ParseInt(*option, resY)) {
				res.Y = resY;
			}
		}
		return res;
	}

	FMinimalViewInfo GetViewInfo(UObject* WorldContext) const {
		if (CameraReference.IsValid(WorldContext)) {
			return CameraReference.GetViewInfo(WorldContext);
		}
		return ViewInfo;
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
		CaptureCamera->SetCamera(true);
		FVector2D ResolutionValue = GetResolution(CaptureCamera);
		CaptureCamera->RenderTarget->ResizeTarget(ResolutionValue.X, ResolutionValue.Y);
		if (CaptureCamera->Camera && ResolutionValue.Y > 0) {
			CaptureCamera->Camera->SetAspectRatio(ResolutionValue.X / ResolutionValue.Y);
		}
	}

	void UpdateCameraSettings(AFICCaptureCamera* CaptureCamera) const {
		FMinimalViewInfo CamView = GetViewInfo(CaptureCamera);
		CaptureCamera->UpdateCaptureWithViewInfo(CamView);
		if (CaptureCamera->Camera) {
			CaptureCamera->Camera->SetFieldOfView(CamView.FOV);
			CaptureCamera->Camera->PostProcessSettings = CamView.PostProcessSettings;
			CaptureCamera->Camera->PostProcessBlendWeight = CamView.PostProcessBlendWeight;
		}
	}

	static FFICCameraArgument FromCli(UCommandSender* InSender, const FFICCameraReference& CameraRef, const FString& Name, TArray<FString> Array) {
		FFICCameraArgument Arg;
		Arg.CameraReference = CameraRef;
		if (CameraRef.IsValid(nullptr)) return Arg;
		Arg.Name = Name;
		Arg.ViewInfo = UFICUtils::CreateViewInfoFromView(InSender);
		FIntPoint Resolution = UFGGameUserSettings::GetFGGameUserSettings()->GetScreenResolution();
		FURL url(nullptr, *CameraRef.GetData(), TRAVEL_Absolute);
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
	static FMinimalViewInfo GetViewInfo(UObject* WorldContext, const FFICCameraArgument& CameraArgument) {
		return CameraArgument.GetViewInfo(WorldContext);
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
