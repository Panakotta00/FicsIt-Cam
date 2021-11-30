#pragma once

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
};


