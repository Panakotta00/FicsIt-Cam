#pragma once

#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Editor/FICEditorContext.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/FICCameraReference.h"
#include "FICUtils.generated.h"

UCLASS(Abstract)
class UFICUtils : public UObject {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static FRotator NormalizeRotator(FRotator Rot);

	UFUNCTION(BlueprintCallable)
	static FString KeymappingToString(const FString& Keymapping);

	UFUNCTION(BlueprintCallable)
	static float BezierInterpolate(FVector2D P0, FVector2D P1, FVector2D P2, FVector2D P3, float t);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContext"))
	static FFICCameraSettingsSnapshot CreateCameraSettingsSnapshotFromView(UObject* WorldContext);

	UFUNCTION(BlueprintCallable)
	static bool IsValidFICObjectName(const FString& InName);

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContext"))
	static bool IsAction(UObject* WorldContext, const FKeyEvent& InKeyEvent, const FName& ActionName);

	template<typename T>
	static T Modulo(T a, T b) {
      T r = a % b;
      return r >= 0 ? r : r + FMath::Abs(b);
    }

	UFUNCTION(BlueprintCallable)
	static FRotator AdditiveRotation(FRotator OldRotation, FRotator NewRotation);

	UFUNCTION(BlueprintCallable)
	static FString AdjustSceneObjectName(AFICScene* Scene, FString Name);
};


