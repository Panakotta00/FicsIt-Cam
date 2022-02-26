#pragma once

#include "InteractiveGizmo.h"
#include "FICSceneObject3D.generated.h"

UINTERFACE()
class UFICSceneObject3D : public UInterface {
	GENERATED_BODY()
};

class FICSITCAM_API IFICSceneObject3D {
	GENERATED_BODY()
public:
	virtual bool Is3DSceneObject() { return false; }
	virtual ETransformGizmoSubElements GetGizmoSubElements() { return ETransformGizmoSubElements::FullTranslateRotateScale; }
	virtual void SetSceneObjectTransform(FTransform InTransform) {}
	virtual FTransform GetSceneObjectTransform() { return FTransform(); }
};
