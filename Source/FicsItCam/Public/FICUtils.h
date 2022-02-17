﻿#pragma once

#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/TextureRenderTarget2D.h"
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
};

