#include "Editor/FICEditorCameraActor.h"

#include "BaseGizmos/TransformGizmo.h"
#include "Components/LineBatchComponent.h"

AFICEditorCameraActor::AFICEditorCameraActor() {
	PrimaryActorTick.bCanEverTick = true;
	
	LineBatcher = CreateDefaultSubobject<ULineBatchComponent>(TEXT("LineBatcher"));
	LineBatcher->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AFICEditorCameraActor::BeginPlay() {
	Super::BeginPlay();

	//Gizmo = FTransformGizmoActorFactory().CreateNewGizmoActor(GetWorld());
	TProxy = NewObject<UTransformProxy>();
	TProxy->SetTransform(GetActorTransform());
}

void AFICEditorCameraActor::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (EditorContext) {
		if (EditorContext->bMoveCamera) {
			UpdateGizmo();
		}
		
		if (EditorContext->bShowPath) {
			FTransform Transform = GetActorTransform();
			GetWorld()->LineBatcher->DrawBox(FBox(FVector(-1, -1, -1), FVector(1, 1, 1)), Transform.GetScaled(FVector(60, 40, 40)).ToMatrixWithScale(), FColor::Green, SDPG_World);
			GetWorld()->LineBatcher->DrawDirectionalArrow(Transform.ToMatrixNoScale(), FColor::Green, 200, 20, SDPG_World);
			GetWorld()->LineBatcher->DrawLine(GetActorLocation(), GetActorTransform().TransformPositionNoScale(FVector(0, 0, 100)), FColor::Green, SDPG_World);
		}
	}
}

void AFICEditorCameraActor::UpdateGizmo() {
	if (TProxy) TProxy->SetTransform(GetActorTransform());
	if (Gizmo) Gizmo->SetActorTransform(GetActorTransform());
}
