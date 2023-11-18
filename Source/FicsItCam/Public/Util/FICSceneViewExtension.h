#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "SequenceExporter.h"

class FFICSceneViewExtension : public FSceneViewExtensionBase {
public:
	TSharedPtr<FSequenceExporter> Exporter;
	ULocalPlayer* Player;

	FFICSceneViewExtension(const FAutoRegister& Register, ULocalPlayer* Player);
	
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
	virtual void SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo) override;
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override;
	virtual void PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override;
};
