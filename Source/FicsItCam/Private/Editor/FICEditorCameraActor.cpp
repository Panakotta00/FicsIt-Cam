#include "Editor/FICEditorCameraActor.h"

#include "BaseGizmos/TransformGizmo.h"
#include "Components/LineBatchComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Editor/Data/FICEditorAttributeBool.h"
#include "Engine/TextureRenderTarget2D.h"

AFICEditorCameraActor::AFICEditorCameraActor() {
	/*CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	CaptureComponent->SetupAttachment(GetRootComponent());
	RenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("RenderTarget"));
	RenderTarget->InitAutoFormat(320, 320);
	CaptureComponent->TextureTarget = RenderTarget;
	CaptureComponent->bCaptureEveryFrame = true;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	CaptureComponent->DetailMode = DM_MAX;
	CaptureComponent->LODDistanceFactor = 0.01;
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	CaptureComponent->bUseRayTracingIfEnabled = true;
	CaptureComponent->ShowFlags.SetTemporalAA(true);
	Brush = FSlateImageBrush(RenderTarget, FVector2D(RenderTarget->SizeX, RenderTarget->SizeY));*/
	
	PrimaryActorTick.bCanEverTick = true;
	
	LineBatcher = CreateDefaultSubobject<ULineBatchComponent>(TEXT("LineBatcher"));
	LineBatcher->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AFICEditorCameraActor::BeginPlay() {
	Super::BeginPlay();

	//Gizmo = FTransformGizmoActorFactory().CreateNewGizmoActor(GetWorld());
	/*TProxy = NewObject<UTransformProxy>();
	TProxy->SetTransform(GetActorTransform());

	FSlateApplication::Get().AddWindow(SNew(SWindow)
		.Content()[
			SNew(SImage)
			.Image(&Brush)
		]);*/
}

void AFICEditorCameraActor::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (EditorContext) {
		if (EditorContext->bMoveCamera) {
			UpdateGizmo();
		}
		
		if (EditorContext->bShowPath) {
			bool Active = EditorContext->GetActiveCamera() == Camera;
			FColor Color = Active ? FColor::Green : FColor::Red;
			FTransform Transform = GetActorTransform();
			GetWorld()->LineBatcher->DrawBox(FBox(FVector(-1, -1, -1), FVector(1, 1, 1)), Transform.GetScaled(FVector(60, 40, 40)).ToMatrixWithScale(), Color, SDPG_World);
			GetWorld()->LineBatcher->DrawDirectionalArrow(Transform.ToMatrixNoScale(), Color, 200, 20, SDPG_World);
			GetWorld()->LineBatcher->DrawLine(GetActorLocation(), GetActorTransform().TransformPositionNoScale(FVector(0, 0, 100)), Color, SDPG_World);
		}
	}
}

void AFICEditorCameraActor::UpdateGizmo() {
	if (TProxy) TProxy->SetTransform(GetActorTransform());
	if (Gizmo) Gizmo->SetActorTransform(GetActorTransform());
}

void AFICEditorCameraActor::UpdateValues(TSharedRef<FFICEditorAttributeBase> Attribute) {
	FVector Pos = FFICAttributePosition::FromEditorAttribute(Attribute->Get<FFICEditorAttributeGroup>("Position"));
	FRotator Rot = FFICAttributeRotation::FromEditorAttribute(Attribute->Get<FFICEditorAttributeGroup>("Rotation"));
	SetActorLocation(Pos);
	SetActorRotation(Rot);
	
	UpdateGizmo();
}
