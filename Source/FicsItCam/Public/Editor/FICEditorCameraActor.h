#pragma once

#include "FICEditorContext.h"
#include "BaseGizmos/TransformGizmo.h"
#include "FICEditorCameraActor.generated.h"

UCLASS()
class AFICEditorCameraActor : public AActor {
	GENERATED_BODY()
public:
	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;

	UPROPERTY()
	ULineBatchComponent* LineBatcher = nullptr;
	
	UPROPERTY()
	ATransformGizmoActor* Gizmo = nullptr;
	UPROPERTY()
	UTransformProxy* TProxy = nullptr;

	AFICEditorCameraActor();
	
	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	void UpdateGizmo();
};
