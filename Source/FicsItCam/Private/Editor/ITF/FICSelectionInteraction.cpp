#include "Editor/ITF/FICSelectionInteraction.h"

#include "Editor/FICEditorContext.h"
#include "Editor/Data/FICEditorCameraActor.h"

UFICSelectionInteraction::UFICSelectionInteraction() {
	ClickBehavior = NewObject<USingleClickInputBehavior>();
	ClickBehavior->Initialize(this);
	HoverBehavior = NewObject<UMouseHoverBehavior>();
	HoverBehavior->Initialize(this);

	BehaviorSet = NewObject<UInputBehaviorSet>();
	BehaviorSet->Add(ClickBehavior, this);
	BehaviorSet->Add(HoverBehavior, this);
}

void UFICSelectionInteraction::Initialize(UFICEditorContext* InContext) {
	Context = InContext;
}

FInputRayHit UFICSelectionInteraction::IsHitByClick(const FInputDeviceRay& ClickPos) {
	FInputRayHit RayHit;
		
	FHitResult Result;
	GetWorld()->LineTraceSingleByChannel(Result, ClickPos.WorldRay.Origin, ClickPos.WorldRay.Origin + ClickPos.WorldRay.Direction * 10000, ECollisionChannel::ECC_GameTraceChannel10);
	if (Result.Actor.IsValid() && Result.GetActor()->Implements<UFICSelectionInteractionTarget>()) {
		RayHit.bHit = true;
		RayHit.HitDepth = Result.Distance;
		//RayHit.HitNormal = ;			// todo - can compute from bary coords
		//RayHit.bHasHitNormal = ;		// todo - can compute from bary coords
		RayHit.HitIdentifier = Result.FaceIndex;
		RayHit.HitOwner = Result.GetActor();
	} else {
		int32 Frame;
		float Distance;
		if (HitCameraPath(ClickPos.WorldRay, LastHovered, Frame, Distance)) {
			RayHit.bHit = true;
			RayHit.HitIdentifier = Frame;
			RayHit.HitDepth = Distance;
			RayHit.HitOwner = this;
		} else {
			RayHit.bHit = true;
			RayHit.HitDepth = TNumericLimits<float>::Max();
			RayHit.HitIdentifier = 0;
			RayHit.HitOwner = this;
		}
	}
	
	return RayHit;
}

void UFICSelectionInteraction::OnClicked(const FInputDeviceRay& ClickPos) {
	FHitResult Result;
	GetWorld()->LineTraceSingleByChannel(Result, ClickPos.WorldRay.Origin, ClickPos.WorldRay.Origin + ClickPos.WorldRay.Direction * 10000, ECollisionChannel::ECC_GameTraceChannel10);
	if (Result.Actor.IsValid() && Result.GetActor()->Implements<UFICSelectionInteractionTarget>()) {
		UObject* SelectedObject = Cast<IFICSelectionInteractionTarget>(Result.GetActor())->Select();
		if (SelectedObject) Context->SetSelectedSceneObject(SelectedObject);
	} else {
		UFICCamera* Camera;
		int32 Frame;
		float Distance;
		if (HitCameraPath(ClickPos.WorldRay, Camera, Frame, Distance)) {
			Context->SetSelectedSceneObject(Camera);
			Context->SetCurrentFrame(Frame);
		} else {
			Context->SetSelectedSceneObject(nullptr);
		}
	}
}

FInputRayHit UFICSelectionInteraction::BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos) {
	FInputRayHit RayHit;
	int32 Frame;
	UFICCamera* Camera;
	float Distance;
	if (HitCameraPath(PressPos.WorldRay, Camera, Frame, Distance)) {
		RayHit.bHit = true;
		RayHit.HitIdentifier = Frame;
		RayHit.HitDepth = Distance;
		RayHit.HitOwner = this;
	} else {
		RayHit.bHit = false;
	}
	return RayHit;
}

void UFICSelectionInteraction::OnBeginHover(const FInputDeviceRay& DevicePos) {
	FInputRayHit RayHit;
	int32 Frame;
	float Distance;
	if (HitCameraPath(DevicePos.WorldRay, LastHovered, Frame, Distance)) {
		LastHovered->EditorCameraActor->CameraPathComponent->Hovered = Frame - Context->GetScene()->AnimationRange.Begin;
	}
}

bool UFICSelectionInteraction::OnUpdateHover(const FInputDeviceRay& DevicePos) {
	FInputRayHit RayHit;
	int32 Frame;
	float Distance;
	if (HitCameraPath(DevicePos.WorldRay, LastHovered, Frame, Distance)) {
		LastHovered->EditorCameraActor->CameraPathComponent->Hovered = Frame - Context->GetScene()->AnimationRange.Begin;
		return true;
	}
	return false;
}

void UFICSelectionInteraction::OnEndHover() {
	if (LastHovered) LastHovered->EditorCameraActor->CameraPathComponent->Hovered = TNumericLimits<int64>::Min();
	LastHovered = nullptr;
}

bool UFICSelectionInteraction::HitCameraPath(const FRay& InRay, UFICCamera*& OutCamera, int32& OutFrame, float& OutDistance) {
	UFICCamera* BestCamera = nullptr;
	OutDistance = TNumericLimits<int32>::Max();
	for (UObject* SceneObject : Context->GetScene()->GetSceneObjects()) {
		UFICCamera* Camera = Cast<UFICCamera>(SceneObject);
		if (!Camera || !Camera->EditorCameraActor) continue;
		const TArray<FVector>& FramePoints = Camera->EditorCameraActor->CameraPathComponent->FramePoints;
		for (int PointIndex = 0; PointIndex < FramePoints.Num(); ++PointIndex) {
			const FVector& Point = FramePoints[PointIndex];
			FVector ClosesPoint;
			float Gap = FMath::PointDistToLine(Point, InRay.Direction, InRay.Origin, ClosesPoint);
			if (Gap < 20) {
				float OriginDistance = FVector::Distance(Point, InRay.Origin);
				if (OriginDistance < OutDistance) {
					OutDistance = OriginDistance;
					OutFrame = Context->GetScene()->AnimationRange.Begin + PointIndex;
					BestCamera = Camera;
				}
			}
		}
		if (BestCamera) {
			OutCamera = BestCamera;
			return true;
		}
	}
	
	return false;
}
