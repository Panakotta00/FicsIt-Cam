#pragma once
#include "Editor/FICEditorContext.h"

#include "FICTransformInteraction.generated.h"

class UTransformProxy;
class UTransformGizmo;
class URuntimeMeshSceneObject;

UCLASS()
class UFICTransformInteraction : public UObject, public FTickableGameObject {
	GENERATED_BODY()
protected:
	UPROPERTY()
	UFICEditorContext* Context = nullptr;
	FDelegateHandle SelectionChangedEventHandle;
	FDelegateHandle ValueChangedHandled;

	bool bInGizmoInteraction = false;

	UPROPERTY()
	UTransformProxy* TransformProxy;

	UPROPERTY()
	UTransformGizmo* TransformGizmo;
	
public:
	// Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return !!Context; }
	virtual TStatId GetStatId() const override { return UObject::GetStatID(); }
	// End FTickableGameObject
	
	void Initialize(UFICEditorContext* InContext);
	void Shutdown();
};