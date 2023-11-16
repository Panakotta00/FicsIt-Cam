#include "Editor/Data/FICEditorCameraActor.h"

#include "EngineUtils.h"
#include "Components/LineBatchComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Editor/Data/FICEditorAttributeBool.h"
#include "Engine/TextureRenderTarget2D.h"
#include "BaseGizmos/CombinedTransformGizmo.h"

UFICEditorCameraPathComponent::UFICEditorCameraPathComponent() {
	bAutoActivate = true;
	bTickInEditor = true;
	PrimaryComponentTick.bCanEverTick = true;

	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	bUseEditorCompositing = true;
	SetGenerateOverlapEvents(false);
		
	bIgnoreStreamingManagerUpdate = true;
}

/*FPrimitiveSceneProxy* UFICEditorCameraPathComponent::CreateSceneProxy() {
	return new FFICEditorCameraPathSceneProxy(this, FramePoints, KeyframePoints, Hovered);
}*/

FFICEditorCameraPathSceneProxy::FFICEditorCameraPathSceneProxy(const UFICEditorCameraPathComponent* InComponent, const TArray<FVector>& FramePoints, const TSet<int64> KeyframePoints, int64 Hovered) : FPrimitiveSceneProxy(InComponent)/*, FramePoints(FramePoints), KeyframePoints(KeyframePoints), Hovered(Hovered)*/ {
	bWillEverBeLit = false;
}

SIZE_T FFICEditorCameraPathSceneProxy::GetTypeHash() const {
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

/*void FFICEditorCameraPathSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const {
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++) {
		if (VisibilityMap & (1 << ViewIndex)) {
			const FSceneView* View = Views[ViewIndex];
			FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

			if (FramePoints.Num() > 0) {
				const FVector& PrevPoint = FramePoints[0];
				for (int32 i = 1; i < FramePoints.Num(); ++i) {
					const FVector& Point = FramePoints[1];
					PDI->DrawLine(PrevPoint, Point,  FColor::Red, SDPG_World, 5);

					FColor PointColor = FColor::Blue;
					bool bIsKeyframe = KeyframePoints.Contains(i);
					if (Hovered == i) PointColor = FColor::Green; 
					else if (bIsKeyframe) PointColor = FColor::Yellow;
					if (bIsKeyframe || Hovered == i || PrevPoint != Point) PDI->DrawPoint(Point, PointColor, 20, SDPG_World);
				}
			}
		}
	}
}

FPrimitiveViewRelevance FFICEditorCameraPathSceneProxy::GetViewRelevance(const FSceneView* View) const {
	FPrimitiveViewRelevance ViewRelevance;
	ViewRelevance.bDrawRelevance = IsShown(View);
	ViewRelevance.bDynamicRelevance = true;
	ViewRelevance.bSeparateTranslucency = ViewRelevance.bNormalTranslucency = true;
	return ViewRelevance;
}*/

uint32 FFICEditorCameraPathSceneProxy::GetMemoryFootprint() const {
	return sizeof(*this) + GetAllocatedSize();
}

/*uint32 FFICEditorCameraPathSceneProxy::GetAllocatedSize() const {
	return FPrimitiveSceneProxy::GetAllocatedSize() + FramePoints.GetAllocatedSize() + KeyframePoints.GetAllocatedSize();
}*/

void UFICEditorCameraPathComponent::UpdateFramePoints() {
	FramePoints.SetNumUninitialized(EditorContext->GetActiveRange().Length());
	FramePoints.SetNumUninitialized(0, false);
	KeyframePoints.Empty();
	for (int64 Time : EditorContext->GetActiveRange()) {
		bool bIsKeyframe = EditorContext->GetEditorAttributes()[Camera]->Get<FFICEditorAttributeBase>("Position").GetKeyframe(Time).IsValid();
		FVector Loc = FFICAttributePosition::FromEditorAttribute(EditorContext->GetEditorAttributes()[Camera]->Get<FFICEditorAttributeGroup>("Position"), Time);
		if (bIsKeyframe) KeyframePoints.Add(FramePoints.Num());
		FramePoints.Add(Loc);
	}
}


AFICEditorCameraActor::AFICEditorCameraActor() {
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("RootComponent"));
	
	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	CaptureComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	RenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("RenderTarget"));
	RenderTarget->InitAutoFormat(320, 320);
	CaptureComponent->TextureTarget = RenderTarget;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	CaptureComponent->DetailMode = DM_MAX;
	CaptureComponent->LODDistanceFactor = 0.01;
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	CaptureComponent->bUseRayTracingIfEnabled = true;
	CaptureComponent->ShowFlags.SetTemporalAA(true);
	CaptureComponent->bCaptureEveryFrame = false;
	CameraPreviewBrush = FSlateImageBrush(RenderTarget, FVector2D(RenderTarget->SizeX, RenderTarget->SizeY));
	
	PrimaryActorTick.bCanEverTick = true;

	LineBatcher = CreateDefaultSubobject<ULineBatchComponent>(TEXT("LineBatcher"));
	LineBatcher->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	LineBatcher->SetMobility(EComponentMobility::Movable);
	
	SelectionHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SelectionHitBox"));
	SelectionHitBox->InitBoxExtent(FVector(60.f,40.f,40.f));
	SelectionHitBox->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	SelectionHitBox->SetMobility(EComponentMobility::Movable);
	SelectionHitBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SelectionHitBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel10, ECollisionResponse::ECR_Block);
	SelectionHitBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	CameraPathComponent = CreateDefaultSubobject<UFICEditorCameraPathComponent>(TEXT("CameraPathComponent"));
	CameraPathComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AFICEditorCameraActor::BeginPlay() {
	Super::BeginPlay();
}

void AFICEditorCameraActor::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (EditorContext) {
		if (EditorContext->bShowPath) {
			LineBatcher->Flush();
			bool Active = EditorContext->GetActiveCamera() == Camera;
			FColor Color = Active ? FColor::Green : FColor::Red;
			FTransform Transform = GetActorTransform();
			LineBatcher->DrawBox(FBox(FVector(-1, -1, -1), FVector(1, 1, 1)), Transform.GetScaled(FVector(60, 40, 40)).ToMatrixWithScale(), Color, SDPG_World);
			LineBatcher->DrawDirectionalArrow((FTransform(FVector(60, 0, 0)) * GetActorTransform()).ToMatrixNoScale(), Color, 100, 20, SDPG_World);
			LineBatcher->DrawLine(GetActorTransform().TransformPositionNoScale(FVector(0, 0, 40)), GetActorTransform().TransformPositionNoScale(FVector(0, 0, 100)), Color, SDPG_World);

			CameraPathComponent->UpdateFramePoints();
		}

		CaptureComponent->HiddenActors.Empty();
		for (TActorIterator<ACombinedTransformGizmoActor> Gizmo(GetWorld()); Gizmo; ++Gizmo) {
			CaptureComponent->HiddenActors.Add(TObjectPtr<AActor>((AActor*)*Gizmo));
		}
		// TODO: This is a shitty fix for something that would require a big change (layer system to define when what editor stuff needs to get rendered)
	}

	// cleanup camera preview
	if (CameraPreview && CameraPreview.IsUnique()) {
		CameraPreview.Reset();
		CaptureComponent->bCaptureEveryFrame = false;
	}
}

UObject* AFICEditorCameraActor::Select() {
	return Camera;
}

void AFICEditorCameraActor::UpdateValues(TSharedRef<FFICEditorAttributeBase> Attribute) {
	FVector Pos = FFICAttributePosition::FromEditorAttribute(Attribute->Get<FFICEditorAttributeGroup>("Position"));
	FRotator Rot = FFICAttributeRotation::FromEditorAttribute(Attribute->Get<FFICEditorAttributeGroup>("Rotation"));
	SetActorLocation(Pos);
	SetActorRotation(Rot);
	CaptureComponent->FOVAngle = Attribute->Get("Lens Settings").Get<TFICEditorAttribute<FFICFloatAttribute>>("FOV").GetValue();
}

TSharedRef<SWidget> AFICEditorCameraActor::GetCameraPreview() {
	UpdateRenderTarget();
	CaptureComponent->bCaptureEveryFrame = true;
	if (!CameraPreview) {
		CameraPreview = SNew(SImage)
		.Image(&CameraPreviewBrush);
	}
	return CameraPreview.ToSharedRef();
}

void AFICEditorCameraActor::UpdateRenderTarget() {
	float AspectRatio = (float)EditorContext->GetScene()->ResolutionWidth / (float)EditorContext->GetScene()->ResolutionHeight;
	RenderTarget->InitAutoFormat(AspectRatio*320, 320);
	CameraPreviewBrush = FSlateImageBrush(RenderTarget, FVector2D(RenderTarget->SizeX, RenderTarget->SizeY));
}
