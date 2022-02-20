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

	Selection->RerunConstructionScripts();
	UE_LOG(LogTemp, Warning, TEXT("MeeP!!!!"));
}
#pragma optimize("", on)

void UFICTransformInteraction::Tick(float DeltaTime) {
	/*if (Context && TransformGizmo) {
		IFICSceneObject* Selection = Cast<IFICSceneObject>(Context->GetSelectedSceneObject());

		if (Selection && Selection->Is3DSceneObject()) {
			TransformProxy->SetTransform(Selection->GetSceneObjectTransform());
			TransformGizmo->SetNewGizmoTransform(Selection->GetSceneObjectTransform());
		}
	}*/
}

void UFICTransformInteraction::Initialize(UFICEditorContext* InContext) {
	Context = InContext;
	
	SelectionChangedEventHandle = Context->OnSceneObjectSelectionChanged.AddLambda([this]() {
		UInteractiveGizmoManager* GizmoManager = AFICEditorSubsystem::GetFICEditorSubsystem(Context)->ToolsContext->GizmoManager;

		if (TransformGizmo != nullptr) {
			GizmoManager->DestroyAllGizmosByOwner(this);
			TransformGizmo = nullptr;
			TransformProxy = nullptr;
		}

		IFICSceneObject* Selection = Cast<IFICSceneObject>(Context->GetSelectedSceneObject());

		if (!Selection || !Selection->Is3DSceneObject() || (Context->GetLockCameraToView() && Context->GetSelectedSceneObject() == Context->GetActiveCamera())) {
			return;
		}

		TransformProxy = NewObject<UTransformProxy>(this);
		TransformProxy->SetTransform(Selection->GetSceneObjectTransform());
		TransformProxy->OnTransformChanged.AddLambda([this](UTransformProxy* Proxy, FTransform Transform) {
			IFICSceneObject* Selection = Cast<IFICSceneObject>(Context->GetSelectedSceneObject());
			if (!Selection || !Selection->Is3DSceneObject() || bInGizmoInteraction) return;
			bInGizmoInteraction = true;
			Selection->SetSceneObjectTransform(Transform);
			bInGizmoInteraction = false;
		});

		TransformGizmo = GizmoManager->CreateCustomTransformGizmo(Selection->GetGizmoSubElements(), this);
		TransformGizmo->SetActiveTarget(TransformProxy);

		ValueChangedHandled = Context->GetEditorAttributes()[Context->GetSelectedSceneObject()]->OnValueChanged.AddLambda([this]() {
			if (Context && TransformGizmo) {
				IFICSceneObject* Selection = Cast<IFICSceneObject>(Context->GetSelectedSceneObject());

				if (Selection && Selection->Is3DSceneObject() && !bInGizmoInteraction) {
					bInGizmoInteraction = true;
					TransformProxy->SetTransform(Selection->GetSceneObjectTransform());
					TransformGizmo->SetNewGizmoTransform(Selection->GetSceneObjectTransform());
					bInGizmoInteraction = false;
				}
			}
		});
	});
}

void UFICTransformInteraction::Shutdown() {
	if (SelectionChangedEventHandle.IsValid()) {
		Context->OnSceneObjectSelectionChanged.Remove(SelectionChangedEventHandle);
		SelectionChangedEventHandle = FDelegateHandle();
	}

	UInteractiveGizmoManager* GizmoManager = AFICEditorSubsystem::GetFICEditorSubsystem(Context)->ToolsContext->GizmoManager;
	GizmoManager->DestroyAllGizmosByOwner(this);
	TransformGizmo = nullptr;
	TransformProxy = nullptr;
}
