#pragma once

#include "FICTransformInteraction.generated.h"

class UTransformProxy;
class UTransformGizmo;
class URuntimeMeshSceneObject;

UCLASS()
class UFICTransformInteraction : public UObject {
	GENERATED_BODY()
public:
	void Initialize();
	void Shutdown();

protected:
	FDelegateHandle SelectionChangedEventHandle;

	UPROPERTY()
	UTransformProxy* TransformProxy;

	UPROPERTY()
	UTransformGizmo* TransformGizmo;
};