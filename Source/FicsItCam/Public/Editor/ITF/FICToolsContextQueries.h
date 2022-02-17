#pragma once

#include "InteractiveToolsContext.h"
#include "GameFramework/Character.h"

class FFICToolsContextQueries : public IToolsContextQueriesAPI {
public:
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

	virtual bool ExecuteSceneSnapQuery(const FSceneSnapQueryRequest& Request, TArray<FSceneSnapQueryResult>& Results) const override {
		return false;
	}

	virtual UMaterialInterface* GetStandardMaterial(EStandardToolContextMaterials MaterialType) const override {
		return UMaterial::GetDefaultMaterial(MD_Surface);
	}

protected:
	UInteractiveToolsContext* ToolsContext;
	UWorld* TargetWorld;

	TArray<AActor*> Selection;
};
