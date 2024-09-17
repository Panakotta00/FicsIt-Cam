#pragma once

#include "CoreMinimal.h"
#include "HitProxies.h"
#include "InteractiveToolsContext.h"
#include "GameFramework/Character.h"
#include "Materials/Material.h"
#include "MaterialDomain.h"
#include "SceneQueries/SceneSnappingManager.h"

class FFICToolsContextQueries : public IToolsContextQueriesAPI {
public:
	// TODO: Check Updates on InteractiveToolsFramework (Snapping, Viewports, World etc.)
	
	FFICToolsContextQueries(UInteractiveToolsContext* InContext, UWorld* InWorld) {
		ToolsContext = InContext;
		TargetWorld = InWorld;
	}

	virtual void GetCurrentSelectionState(FToolBuilderState& StateOut) const override {
		StateOut.ToolManager = ToolsContext->ToolManager;
		StateOut.GizmoManager = ToolsContext->GizmoManager;
		StateOut.World = TargetWorld;

		for (AActor* Actor : Selection)
		{
			StateOut.SelectedActors.Add(Actor);
		}
	}

	virtual void GetCurrentViewState(FViewCameraState& StateOut) const override {
		AActor* Character = TargetWorld->GetFirstPlayerController()->GetCharacter();

		FVector Location;
		FRotator Rotation;
		Character->GetActorEyesViewPoint(Location, Rotation);

		StateOut.Position = Location;
		StateOut.Orientation = Rotation.Quaternion();
		StateOut.HorizontalFOVDegrees = 90;
		StateOut.OrthoWorldCoordinateWidth = 1;
		StateOut.AspectRatio = 1.0;
		StateOut.bIsOrthographic = false;
		StateOut.bIsVR = false;
	}

	virtual EToolContextCoordinateSystem GetCurrentCoordinateSystem() const override {
		return EToolContextCoordinateSystem::Local;
	}

	virtual UMaterialInterface* GetStandardMaterial(EStandardToolContextMaterials MaterialType) const override {
		return UMaterial::GetDefaultMaterial(MD_Surface);
	}

	virtual UWorld* GetCurrentEditingWorld() const override {
		 return TargetWorld;
	}

	virtual FToolContextSnappingConfiguration GetCurrentSnappingSettings() const override {
		FToolContextSnappingConfiguration Config;
		Config.bEnablePositionGridSnapping = false;
		Config.bEnableRotationGridSnapping = false;
		return Config;
	}

	virtual FViewport* GetHoveredViewport() const override {
		return nullptr;
	}

	virtual FViewport* GetFocusedViewport() const override {
		return nullptr;
	}

protected:
	UInteractiveToolsContext* ToolsContext;
	UWorld* TargetWorld;

	TArray<AActor*> Selection;
};
