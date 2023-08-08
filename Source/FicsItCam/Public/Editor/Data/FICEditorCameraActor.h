#pragma once

#include "Editor/FICEditorContext.h"
#include "Editor/ITF/FICSelectionInteraction.h"
#include "FICEditorCameraActor.generated.h"

UCLASS()
class UFICEditorCameraPathComponent : public UPrimitiveComponent {
	GENERATED_BODY()
public:
	UPROPERTY()
	UFICCamera* Camera = nullptr;
	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;

	TArray<FVector> FramePoints;
	TSet<int64> KeyframePoints;
	int64 Hovered = TNumericLimits<int64>::Min();

	UFICEditorCameraPathComponent();
	
	// Begin UPrimitiveComponent
	//virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	// End UPrimitiveComponent
	
	void UpdateFramePoints();
};

class FFICEditorCameraPathSceneProxy : public FPrimitiveSceneProxy {
public:
	FFICEditorCameraPathSceneProxy(const UFICEditorCameraPathComponent* InComponent, const TArray<FVector>& FramePoints, const TSet<int64> KeyframePoints, int64 Hovered);

	virtual SIZE_T GetTypeHash() const override;
	//virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	//virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual uint32 GetMemoryFootprint() const override;
	//uint32 GetAllocatedSize() const;

	//TArray<FVector> FramePoints;
	//TSet<int64> KeyframePoints;
	//int64 Hovered;
};

UCLASS()
class AFICEditorCameraActor : public AActor, public IFICSelectionInteractionTarget {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;
	UPROPERTY()
	UFICCamera* Camera = nullptr;

public:
	UPROPERTY()
	USceneCaptureComponent2D* CaptureComponent = nullptr;
	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget = nullptr;
	FSlateImageBrush CameraPreviewBrush = FSlateImageBrush(TEXT("CameraPreview"), FVector2D(1,1), FLinearColor::Green);

	UPROPERTY()
	ULineBatchComponent* LineBatcher = nullptr;
	UPROPERTY()
	UBoxComponent* SelectionHitBox = nullptr;
	UPROPERTY()
	UFICEditorCameraPathComponent* CameraPathComponent = nullptr;

	TSharedPtr<SWidget> CameraPreview;
	
	AFICEditorCameraActor();
	
	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	// Begin IFICSelectionInteractionTarget
	virtual UObject* Select() override;
	// End IFICSelectionInteractionTarget

	void Initialize(UFICEditorContext* InContext, UFICCamera* InCamera) {
		EditorContext = InContext;
		Camera = InCamera;
		CameraPathComponent->EditorContext = EditorContext;
		CameraPathComponent->Camera = Camera;
	}

	void UpdateValues(TSharedRef<FFICEditorAttributeBase> Attribute);
	TSharedRef<SWidget> GetCameraPreview();
	void UpdateRenderTarget();
};
