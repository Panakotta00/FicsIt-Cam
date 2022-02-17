#pragma once

#include "FICEditorContext.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/SceneCaptureComponent.h"
#include "FICEditorCameraActor.generated.h"

UCLASS()
class AFICEditorCameraActor : public AActor {
	GENERATED_BODY()
public:
	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;
	UPROPERTY()
	UFICCamera* Camera = nullptr;
	
	UPROPERTY()
	USceneCaptureComponent2D* CaptureComponent;
	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;
	FSlateImageBrush Brush = FSlateImageBrush("Meep", FVector2D(1,1), FColor::Green);

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
	
	void UpdateValues(TSharedRef<FFICEditorAttributeBase> Attribute);
};
