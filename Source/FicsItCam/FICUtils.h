#pragma once

#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/TextureRenderTarget2D.h"

FRotator NormalizeRotator(FRotator Rot);

bool FIC_SaveRenderTargetAsJPG(const FString& FilePath, UTextureRenderTarget2D* RenderTarget);
