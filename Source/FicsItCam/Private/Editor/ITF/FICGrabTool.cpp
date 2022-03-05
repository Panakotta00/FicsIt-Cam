#include "Editor/ITF/FICGrabTool.h"

#include "BaseBehaviors/KeyAsModifierInputBehavior.h"
#include "BaseBehaviors/MouseHoverBehavior.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "Data/Objects/FICSceneObject3D.h"
#include "Editor/FICEditorContext.h"
#include "Editor/FICEditorSubsystem.h"

void UFICGrabToolAxiLockTarget::OnUpdateModifierState(int ModifierID, bool bIsOn) {
	if (!bIsOn) return;
	switch (ModifierID) {
	case 0:
		GrabTool->AxisLock = FIC_GrabTool_Lock_X;
		break;
	case 1:
		GrabTool->AxisLock = FIC_GrabTool_Lock_Y;
		break;
	case 2:
		GrabTool->AxisLock = FIC_GrabTool_Lock_Z;
		break;
	default: break;
	}
}

UFICGrabTool::UFICGrabTool() {
	ClickBehavior = CreateDefaultSubobject<USingleClickInputBehavior>("ClickBehaviour");
	ClickBehavior->Initialize(this);
	ClickBehavior->SetDefaultPriority(FInputCapturePriority(40));
	HoverBehavior = CreateDefaultSubobject<UMouseHoverBehavior>("HoverBehavior");
	HoverBehavior->Initialize(this);
	HoverBehavior->SetDefaultPriority(FInputCapturePriority(40));
	ModifierTarget = CreateDefaultSubobject<UFICGrabToolAxiLockTarget>("ModifierTarget");
	ModifierTarget->GrabTool = this;
	KeyAsModifierBehaviourX = CreateDefaultSubobject<UKeyAsModifierInputBehavior>("KeyAsModifierBehaviourX");
	KeyAsModifierBehaviourY = CreateDefaultSubobject<UKeyAsModifierInputBehavior>("KeyAsModifierBehaviourY");
	KeyAsModifierBehaviourZ = CreateDefaultSubobject<UKeyAsModifierInputBehavior>("KeyAsModifierBehaviourZ");
	KeyAsModifierBehaviourX->Initialize(ModifierTarget, 0, EKeys::X);
	KeyAsModifierBehaviourY->Initialize(ModifierTarget, 1, EKeys::Y);
	KeyAsModifierBehaviourZ->Initialize(ModifierTarget, 2, EKeys::Z);
	
	BehaviorSet = CreateDefaultSubobject<UInputBehaviorSet>("BehaviorSet");
	BehaviorSet->Add(ClickBehavior, this);
	BehaviorSet->Add(HoverBehavior, this);
	BehaviorSet->Add(KeyAsModifierBehaviourX, this);
	BehaviorSet->Add(KeyAsModifierBehaviourY, this);
	BehaviorSet->Add(KeyAsModifierBehaviourZ, this);
}

void UFICGrabTool::Setup() {
	InitialTransform = Cast<IFICSceneObject3D>(SceneObject)->GetSceneObjectTransform();
}

void UFICGrabTool::Shutdown(EToolShutdownType ShutdownType) {
	Super::Shutdown(ShutdownType);

	if (ShutdownType == EToolShutdownType::Cancel) {
		Cast<IFICSceneObject3D>(SceneObject)->SetSceneObjectTransform(InitialTransform);
	}
}

void UFICGrabTool::Render(IToolsContextRenderAPI* RenderAPI) {
	FVector Origin = InitialTransform.GetLocation();
	FVector X = InitialTransform.TransformVector(FVector(1,0,0));
	FVector Y = InitialTransform.TransformVector(FVector(0,1,0));
	FVector Z = InitialTransform.TransformVector(FVector(0,0,1));
	switch (AxisLock) {
	case FIC_GrabTool_Lock_X:
		SceneObject->GetWorld()->LineBatcher->DrawLine(Origin - X*Length, Origin + X*Length, FColor::Red, SDPG_World, 10);
		break;
	case FIC_GrabTool_Lock_Y:
		SceneObject->GetWorld()->LineBatcher->DrawLine(Origin - Y*Length, Origin + Y*Length, FColor::Green, SDPG_World, 10);
		break;
	case FIC_GrabTool_Lock_Z:
		SceneObject->GetWorld()->LineBatcher->DrawLine(Origin - Z*Length, Origin + Z*Length, FColor::Blue, SDPG_World, 10);
	default: ;
	}
}

void UFICGrabTool::OnTick(float DeltaTime) {}

bool UFICGrabTool::HasAccept() {
	return true;
}

bool UFICGrabTool::CanAccept() {
	return true;
}

FInputRayHit UFICGrabTool::IsHitByClick(const FInputDeviceRay& ClickPos) {
	return FInputRayHit(-1000);
}

void UFICGrabTool::OnClicked(const FInputDeviceRay& ClickPos) {
	GetToolManager()->DeactivateTool(EToolSide::Mouse, EToolShutdownType::Accept);
}

FInputRayHit UFICGrabTool::BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos) {
	return FInputRayHit(-10000);
}

void UFICGrabTool::OnBeginHover(const FInputDeviceRay& DevicePos) {}

bool UFICGrabTool::OnUpdateHover(const FInputDeviceRay& DevicePos) {
	FVector Origin = InitialTransform.GetLocation();
	FVector Normal;
	switch (AxisLock) {
	case FIC_GrabTool_Lock_X:
		Normal = InitialTransform.TransformVector(FVector(1,0,0));
		break;
	case FIC_GrabTool_Lock_Y:
		Normal = InitialTransform.TransformVector(FVector(0,1,0));
		break;
	case FIC_GrabTool_Lock_Z:
		Normal = InitialTransform.TransformVector(FVector(0,0,1));
		break;
	default:
		Normal = DevicePos.WorldRay.Direction;
		break;
	}
	FVector PlaneNormal = FVector::CrossProduct(FVector::CrossProduct(Normal, DevicePos.WorldRay.Direction), Normal);
	bool bDirectPlane = false;
	if (PlaneNormal.ContainsNaN() || PlaneNormal.IsNearlyZero()) {
		PlaneNormal = -DevicePos.WorldRay.Direction;
		bDirectPlane = true;
	}
	FVector NewLocation = FMath::LinePlaneIntersection(DevicePos.WorldRay.Origin, DevicePos.WorldRay.Origin + DevicePos.WorldRay.Direction * Length, Origin, PlaneNormal);
	if (!bDirectPlane) NewLocation = FMath::ClosestPointOnLine(Origin - Normal*Length, Origin + Normal*Length, NewLocation);
	FTransform NewTransform = InitialTransform;
	NewTransform.SetLocation(NewLocation);
	Cast<IFICSceneObject3D>(SceneObject)->SetSceneObjectTransform(NewTransform);
	return true;
}

void UFICGrabTool::OnEndHover() {}

bool UFICGrabToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const {
	UFICEditorContext* Context = AFICEditorSubsystem::GetFICEditorSubsystem(SceneState.World)->GetActiveEditorContext();
	if (!Context) return false;
	UObject* SceneObject = Context->GetSelectedSceneObject();
	if (!Cast<IFICSceneObject3D>(SceneObject)) return false;
	return true;
}

UInteractiveTool* UFICGrabToolBuilder::BuildTool(const FToolBuilderState& SceneState) const {
	UFICEditorContext* Context = AFICEditorSubsystem::GetFICEditorSubsystem(SceneState.World)->GetActiveEditorContext();
	UObject* SceneObject = Context->GetSelectedSceneObject();
	UFICGrabTool* Tool = NewObject<UFICGrabTool>(SceneState.ToolManager);
	Tool->SceneObject = SceneObject;
	return Tool;
}
