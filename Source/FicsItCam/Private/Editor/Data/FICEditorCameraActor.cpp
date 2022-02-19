#include "Editor/Data/FICEditorCameraActor.h"

#include "BaseGizmos/TransformGizmo.h"
#include "Components/LineBatchComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Editor/Data/FICEditorAttributeBool.h"
#include "Engine/TextureRenderTarget2D.h"

AFICEditorCameraActor::AFICEditorCameraActor() {
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("RootComponent"));
	
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
	LineBatcher->SetupAttachment(RootComponent);
	LineBatcher->SetMobility(EComponentMobility::Movable);
	
	SelectionHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SelectionHitBox"));
	SelectionHitBox->InitBoxExtent(FVector(60.f,40.f,40.f));
	SelectionHitBox->SetupAttachment(RootComponent);
	SelectionHitBox->SetMobility(EComponentMobility::Movable);
	SelectionHitBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SelectionHitBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel10, ECollisionResponse::ECR_Block);
	SelectionHitBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
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
			LineBatcher->DrawDirectionalArrow(Transform.ToMatrixNoScale(), Color, 200, 20, SDPG_World);
			LineBatcher->DrawLine(GetActorLocation(), GetActorTransform().TransformPositionNoScale(FVector(0, 0, 100)), Color, SDPG_World);
		}
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
}
