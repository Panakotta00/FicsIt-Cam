#pragma once

#include "InputBehaviorSet.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "BaseBehaviors/MouseHoverBehavior.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "Editor/FICEditorSubsystem.h"

#include "FICSelectionInteraction.generated.h"

class UFICCamera;
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
class UFICSelectionInteraction : public UObject, public IInputBehaviorSource, public IClickBehaviorTarget, public IHoverBehaviorTarget {
	GENERATED_BODY()
private:
	UPROPERTY()
	USingleClickInputBehavior* ClickBehavior;
	UPROPERTY()
	UMouseHoverBehavior* HoverBehavior;
	UPROPERTY()
	UInputBehaviorSet* BehaviorSet;
	UPROPERTY()
	UFICEditorContext* Context;
	UPROPERTY()
	UFICCamera* LastHovered = nullptr;

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

	// Begin IHoverBhaviourTarget
	virtual FInputRayHit BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos) override;
	virtual void OnBeginHover(const FInputDeviceRay& DevicePos) override;
	virtual bool OnUpdateHover(const FInputDeviceRay& DevicePos) override;
	virtual void OnEndHover() override;
	// End IHoverBehaviourTarget

	bool HitCameraPath(const FRay& InRay, UFICCamera*& OutCamera, int32& OutFrame, float& OutDistance);
};
