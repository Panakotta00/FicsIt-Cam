#pragma once

#include "InputBehaviorSet.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "Editor/FICEditorSubsystem.h"

#include "FICSelectionInteraction.generated.h"

UINTERFACE()
class UFICSelectionInteractionTarget : public UInterface {
	GENERATED_BODY()
};

class IFICSelectionInteractionTarget {
	GENERATED_IINTERFACE_BODY()
public:
	virtual UObject* Select() { return nullptr; }
};

UCLASS()
class UFICSelectionInteraction : public UObject, public IInputBehaviorSource, public IClickBehaviorTarget {
	GENERATED_BODY()
private:
	UPROPERTY()
	USingleClickInputBehavior* ClickBehavior;
	UPROPERTY()
	UInputBehaviorSet* BehaviorSet;
	UPROPERTY()
	UFICEditorContext* Context;

public:
	UFICSelectionInteraction();

	void Initialize(UFICEditorContext* Context);

	// Begin IInputBehaviourSource
	virtual const UInputBehaviorSet* GetInputBehaviors() const override {
		return BehaviorSet;
	}
	// End IInputBehaviourSource

	// Begin IClickBehaviourTarget
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override;
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;
	// End IClickBehaviourTarget
};
