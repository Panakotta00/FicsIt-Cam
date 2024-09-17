#pragma once

#include "InteractiveTool.h"
#include "InteractiveToolBuilder.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "FICGrabTool.generated.h"

class UKeyAsModifierInputBehavior;
class UMouseHoverBehavior;
class USingleClickInputBehavior;

UENUM()
enum EFICGrabToolAxisLock {
	FIC_GrabTool_Lock_None,
	FIC_GrabTool_Lock_X,
	FIC_GrabTool_Lock_Y,
	FIC_GrabTool_Lock_Z,
};

UCLASS()
class UFICGrabToolAxiLockTarget : public UObject, public IModifierToggleBehaviorTarget {
	GENERATED_BODY()
public:
	UPROPERTY()
	class UFICGrabTool* GrabTool = nullptr;
	
	// Begin IModifierToggleBehaviorTarget
	virtual void OnUpdateModifierState(int ModifierID, bool bIsOn) override;
	// End IModifierToggleBehaviorTarget
};

UCLASS()
class UFICGrabTool : public UInteractiveTool, public IClickBehaviorTarget, public IHoverBehaviorTarget {
	GENERATED_BODY()
public:
	UPROPERTY()
	USingleClickInputBehavior* ClickBehavior;
	UPROPERTY()
	UMouseHoverBehavior* HoverBehavior;
	UPROPERTY()
	UKeyAsModifierInputBehavior* KeyAsModifierBehaviourX;
	UPROPERTY()
	UKeyAsModifierInputBehavior* KeyAsModifierBehaviourY;
	UPROPERTY()
	UKeyAsModifierInputBehavior* KeyAsModifierBehaviourZ;
	UPROPERTY()
	UInputBehaviorSet* BehaviorSet;
	UPROPERTY()
	UFICGrabToolAxiLockTarget* ModifierTarget;

	UPROPERTY()
	UObject* SceneObject = nullptr;
	UPROPERTY()
	FTransform InitialTransform;
	UPROPERTY()
	TEnumAsByte<EFICGrabToolAxisLock> AxisLock = FIC_GrabTool_Lock_None;

	const int Length = 1000000;

	UFICGrabTool();
	
	// Begin UInteractiveTool
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;
	virtual void OnTick(float DeltaTime) override;

	virtual bool HasAccept() const override;
	virtual bool CanAccept() const override;
	// End UInteractiveTool

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
};

UCLASS()
class UFICGrabToolBuilder : public UInteractiveToolBuilder {
	GENERATED_BODY()
public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};