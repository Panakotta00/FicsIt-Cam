#pragma once

#include "InputBehaviorSet.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "Editor/FICEditorSubsystem.h"

#include "FICSelectionInteraction.generated.h"

UCLASS()
class UFICSelectionInteraction : public UObject, public IInputBehaviorSource, public IClickBehaviorTarget {
	GENERATED_BODY()
private:
	UPROPERTY()
	USingleClickInputBehavior* ClickBehavior;
	UPROPERTY()
	UInputBehaviorSet* BehaviorSet;

public:
	UFICSelectionInteraction() {
		ClickBehavior = NewObject<USingleClickInputBehavior>();
		ClickBehavior->Initialize(this);

		BehaviorSet = NewObject<UInputBehaviorSet>();
		BehaviorSet->Add(ClickBehavior, this);
	}
	
	// Begin IInputBehaviourSource
	virtual const UInputBehaviorSet* GetInputBehaviors() const {
		return BehaviorSet;
	}
	// End IInputBehaviourSource

	// Begin IClickBehaviourTarget
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override {
		FInputRayHit RayHit;
		
		FHitResult Result;
		GetWorld()->LineTraceSingleByObjectType(Result, ClickPos.WorldRay.Origin, ClickPos.WorldRay.Origin + ClickPos.WorldRay.Direction * 10000, FCollisionObjectQueryParams());
		if (Result.Actor.IsValid()) {
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
	
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override {
		FHitResult Result;
		GetWorld()->LineTraceSingleByObjectType(Result, ClickPos.WorldRay.Origin, ClickPos.WorldRay.Origin + ClickPos.WorldRay.Direction * 10000, FCollisionObjectQueryParams());
		if (Result.Actor.IsValid()) {
			AFICEditorSubsystem::GetFICEditorSubsystem(GetWorld())->SetSelection(Result.GetActor());
		} else {
			AFICEditorSubsystem::GetFICEditorSubsystem(GetWorld())->SetSelection(nullptr);
		}
	}
	// End IClickBehaviourTarget
};
