#include "Editor/ITF/FICTransformInteraction.h"

#include "BaseGizmos/CombinedTransformGizmo.h"
#include "BaseGizmos/TransformProxy.h"
#include "Editor/FICEditorSubsystem.h"
#include "Editor/Data/FICEditorAttributeBase.h"

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

	TFunction<void()> SelectionChanged = [this]() {
		UInteractiveGizmoManager* GizmoManager = AFICEditorSubsystem::GetFICEditorSubsystem(Context)->ToolsContext->GizmoManager;

		if (TransformGizmo != nullptr) {
			GizmoManager->DestroyAllGizmosByOwner(this);
			TransformGizmo = nullptr;
			TransformProxy = nullptr;
		}

		IFICSceneObject3D* Selection = Cast<IFICSceneObject3D>(Context->GetSelectedSceneObject());

		if (!Selection || !Selection->Is3DSceneObject() || (Context->GetLockCameraToView() && Context->GetSelectedSceneObject() == Context->GetActiveCamera())) {
			return;
		}

		TransformProxy = NewObject<UTransformProxy>(this);
		TransformProxy->SetTransform(Selection->GetSceneObjectTransform());
		TransformProxy->OnTransformChanged.AddLambda([this](UTransformProxy* Proxy, FTransform Transform) {
			IFICSceneObject3D* Selection = Cast<IFICSceneObject3D>(Context->GetSelectedSceneObject());
			if (!Selection || bInGizmoInteraction) return;
			bInGizmoInteraction = true;
			Selection->SetSceneObjectTransform(Transform);
			bInGizmoInteraction = false;
		});

		TransformGizmo = GizmoManager->CreateCustomTransformGizmo(Selection->GetGizmoSubElements(), this);
		TransformGizmo->SetActiveTarget(TransformProxy);
		
		ValueChangedHandled = Context->GetEditorAttributes()[Context->GetSelectedSceneObject()]->OnValueChanged.AddLambda([this]() {
			if (Context && TransformGizmo) {
				IFICSceneObject3D* Selection = Cast<IFICSceneObject3D>(Context->GetSelectedSceneObject());

				if (Selection && Selection->Is3DSceneObject() && !bInGizmoInteraction) {
					bInGizmoInteraction = true;
					TransformProxy->SetTransform(Selection->GetSceneObjectTransform());
					TransformGizmo->SetNewGizmoTransform(Selection->GetSceneObjectTransform());
					bInGizmoInteraction = false;
				}
			}
		});
	};
	SelectionChangedEventHandle = Context->OnSceneObjectSelectionChanged.AddLambda(SelectionChanged);
	SelectionChanged();
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
