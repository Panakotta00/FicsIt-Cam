#pragma once

#include "CoreMinimal.h"
#include "InteractiveGizmo.h"
#include "UObject/Interface.h"
#include "FICSceneObject.generated.h"

UINTERFACE()
class UFICSceneObject : public UInterface {
	GENERATED_BODY()
};

class FICSITCAM_API IFICSceneObject {
	GENERATED_BODY()

public:
	virtual FString GetSceneObjectName() = 0;
	virtual void SetSceneObjectName(FString Name) = 0;
	
	virtual FFICAttribute& GetRootAttribute() = 0;
	virtual void InitDefaultValues() {}

	virtual void InitEditor(UFICEditorContext* Context) {}
	virtual void UnloadEditor(UFICEditorContext* Context) {}
	virtual void EditorUpdate(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) {}
	virtual void Select(UFICEditorContext* Context) {}
	virtual void Unselect(UFICEditorContext* Context) {}

	virtual bool Is3DSceneObject() { return false; }
	virtual ETransformGizmoSubElements GetGizmoSubElements() { return ETransformGizmoSubElements::FullTranslateRotateScale; }
	virtual void SetSceneObjectTransform(FTransform InTransform) {}
	virtual FTransform GetSceneObjectTransform() { return FTransform(); }
};
