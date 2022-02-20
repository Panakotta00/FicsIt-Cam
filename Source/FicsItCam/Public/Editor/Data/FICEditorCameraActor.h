#pragma once

#include "Editor/FICEditorContext.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Editor/ITF/FICSelectionInteraction.h"
#include "FICEditorCameraActor.generated.h"

UCLASS()
class AFICEditorCameraActor : public AActor, public IFICSelectionInteractionTarget {
	GENERATED_BODY()
public:
	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;
	UPROPERTY()
	UFICCamera* Camera = nullptr;
	
	UPROPERTY()
	USceneCaptureComponent2D* CaptureComponent = nullptr;
	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget = nullptr;
	FSlateImageBrush CameraPreviewBrush = FSlateImageBrush("CameraPreview", FVector2D(1,1), FColor::Green);

	UPROPERTY()
	ULineBatchComponent* LineBatcher = nullptr;
	UPROPERTY()
	UBoxComponent* SelectionHitBox = nullptr;

	TSharedPtr<SWidget> CameraPreview;
	
	AFICEditorCameraActor();
	
	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	// Begin IFICSelectionInteractionTarget
	virtual UObject* Select() override;
	// End IFICSelectionInteractionTarget

	void UpdateValues(TSharedRef<FFICEditorAttributeBase> Attribute);
	TSharedRef<SWidget> GetCameraPreview();
	void UpdateRenderTarget();
};
