// Copied from the UnrealRuntimeToolsFrameworkDemo-Project (MIT) https://github.com/gradientspace/UnrealRuntimeToolsFrameworkDemo/blob/main/Source/RuntimeToolsSystem/Public/RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.cpp

#pragma once

#include "FICToolsContextRenderComponent.h"
#include "ToolContextInterfaces.h"

class FRuntimeToolsFrameworkRenderImpl : public IToolsContextRenderAPI {
public:
	UFICToolsContextRenderComponent* RenderComponent;
	TSharedPtr<FPrimitiveDrawInterface> PDI;
	const FSceneView* SceneView;
	FViewCameraState ViewCameraState;

	FRuntimeToolsFrameworkRenderImpl(UFICToolsContextRenderComponent* RenderComponentIn, const FSceneView* ViewIn, FViewCameraState CameraState) : RenderComponent(RenderComponentIn), SceneView(ViewIn), ViewCameraState(CameraState) {
		PDI = RenderComponentIn->GetPDIForView(ViewIn);
	}

	virtual FPrimitiveDrawInterface* GetPrimitiveDrawInterface() override {
		return PDI.Get();
	}

	virtual const FSceneView* GetSceneView() override {
		return SceneView;
	}

	virtual FViewCameraState GetCameraState() override {
		return ViewCameraState;
	}

	virtual EViewInteractionState GetViewInteractionState() override {
		return EViewInteractionState::Focused;
	}
};
