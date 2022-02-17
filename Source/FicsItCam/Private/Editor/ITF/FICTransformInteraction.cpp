#include "Editor/ITF/FICTransformInteraction.h"

#include "FGColoredInstanceMeshProxy.h"
#include "BaseGizmos/TransformGizmo.h"
#include "BaseGizmos/TransformProxy.h"
#include "Buildables/FGBuildable.h"
#include "Components/ProxyInstancedStaticMeshComponent.h"
#include "Editor/FICEditorSubsystem.h"

#pragma optimize("", off)

void Test(UTransformProxy* Proxy, FTransform Transform) {
	AFICEditorSubsystem* SubSys = AFICEditorSubsystem::GetFICEditorSubsystem(Proxy->GetWorld());
	AActor* Selection = SubSys->GetSelection();
	auto SavedMobility = Selection->GetRootComponent()->Mobility;
	Selection->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	Selection->GetRootComponent()->SetWorldTransform(Transform, false, nullptr, ETeleportType::TeleportPhysics);
	Selection->GetRootComponent()->SetMobility(SavedMobility);
	/*for (UActorComponent* Comp : Selection->GetComponents()) {
		UFGColoredInstanceMeshProxy* Proxy = Cast<UFGColoredInstanceMeshProxy>(Comp);
		
	}*/
	Selection->RerunConstructionScripts();
	UE_LOG(LogTemp, Warning, TEXT("MeeP!!!!"));
}
#pragma optimize("", on)

void UFICTransformInteraction::Initialize() {
	AFICEditorSubsystem* SubSys = AFICEditorSubsystem::GetFICEditorSubsystem(GetWorld());
	SelectionChangedEventHandle = SubSys->OnSelectionChanged.AddLambda([this, SubSys]() {
		AActor* Selection = SubSys->GetSelection();
		
		UInteractiveGizmoManager* GizmoManager = SubSys->ToolsContext->GizmoManager;

		// destroy existing gizmos if we have any
		if (TransformGizmo != nullptr)
		{
			GizmoManager->DestroyAllGizmosByOwner(this);
			TransformGizmo = nullptr;
			TransformProxy = nullptr;
		}

		if (!Selection) {
			return;
		}

		TransformProxy = NewObject<UTransformProxy>(this);
		TransformProxy->SetTransform(Selection->GetActorTransform());
		TransformProxy->OnTransformChanged.AddStatic(&Test);

		ETransformGizmoSubElements GizmoElements = ETransformGizmoSubElements::FullTranslateRotateScale;
		TransformGizmo = GizmoManager->CreateCustomTransformGizmo(GizmoElements, this);
		TransformGizmo->SetActiveTarget(TransformProxy);
	});
}

void UFICTransformInteraction::Shutdown() {
	AFICEditorSubsystem* SubSys = AFICEditorSubsystem::GetFICEditorSubsystem(GetWorld());
	if (SelectionChangedEventHandle.IsValid()) {
		if (SubSys) {
			SubSys->OnSelectionChanged.Remove(SelectionChangedEventHandle);
		}
		SelectionChangedEventHandle = FDelegateHandle();
	}

	UInteractiveGizmoManager* GizmoManager = SubSys->ToolsContext->GizmoManager;
	GizmoManager->DestroyAllGizmosByOwner(this);
	TransformGizmo = nullptr;
	TransformProxy = nullptr;
}
