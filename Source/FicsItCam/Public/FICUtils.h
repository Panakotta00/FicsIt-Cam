#pragma once

#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/FICCameraReference.h"
#include "FICUtils.generated.h"

UCLASS(Abstract)
class UFICUtils : public UObject {
	GENERATED_BODY()

public:
	UFUNCTION()
	static FRotator NormalizeRotator(FRotator Rot);

	UFUNCTION()
	static FString KeymappingToString(const FString& Keymapping);

	UFUNCTION()
	static float BezierInterpolate(FVector2D P0, FVector2D P1, FVector2D P2, FVector2D P3, float t);

	UFUNCTION()
	static FFICCameraSettingsSnapshot CreateCameraSettingsSnapshotFromView(UObject* WorldContext);

	UFUNCTION()
	static bool IsValidFICObjectName(const FString& InName);

	UFUNCTION()
	static bool IsAction(UObject* WorldContext, const FKeyEvent& InKeyEvent, const FName& ActionName);

	template<typename T>
	static T Modulo(T a, T b) {
      T r = a % b;
      return r >= 0 ? r : r + FMath::Abs(b);
    }

	UFUNCTION()
	static FRotator AdditiveRotation(FRotator OldRotation, FRotator NewRotation);
};


