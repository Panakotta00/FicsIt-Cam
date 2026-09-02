#pragma once

#include "CoreMinimal.h"
#include "Slate/SlateTextures.h"
#include "Slate/SceneViewport.h"

class FFICDummyViewport : public ISlateViewport {
public:
	FFICDummyViewport(FTextureRHIRef RenderTargetTexture, FIntPoint InSize)
		: RenderTarget( new FSlateRenderTargetRHI(RenderTargetTexture, InSize.X, InSize.Y))
		, Size(InSize) {
		BeginInitResource(RenderTarget);
	}

	~FFICDummyViewport() {
		ReleaseResourceAndFlush(RenderTarget);
		delete RenderTarget;
	}

	// Begin ISlateViewport
	virtual FIntPoint GetSize() const override {
		return Size;
	}

	virtual FSlateShaderResource* GetViewportRenderTargetTexture() const override {
		return RenderTarget;
	}

	virtual bool RequiresVsync() const override {
		return false;
	}
	// End ISlateViewport

private:
	FSlateRenderTargetRHI* RenderTarget;
	FIntPoint Size;
};