#include "Editor/ITF/FICSelectionInteraction.h"

#include "Editor/FICEditorContext.h"

UFICSelectionInteraction::UFICSelectionInteraction() {
	ClickBehavior = NewObject<USingleClickInputBehavior>();
	ClickBehavior->Initialize(this);

	BehaviorSet = NewObject<UInputBehaviorSet>();
	BehaviorSet->Add(ClickBehavior, this);
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
		RayHit.bHit = true;
		RayHit.HitDepth = TNumericLimits<float>::Max();
		RayHit.HitIdentifier = 0;
		RayHit.HitOwner = this;
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
		Context->SetSelectedSceneObject(nullptr);
	}
}
