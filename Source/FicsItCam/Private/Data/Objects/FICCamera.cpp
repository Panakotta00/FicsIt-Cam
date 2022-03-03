#include "Data/Objects/FICCamera.h"

#include "Components/LineBatchComponent.h"
#include "Editor/Data/FICEditorCameraActor.h"
#include "Editor/FICEditorContext.h"

void UFICCamera::Tick(float DeltaTime) {
	// Draw Path
	/*if (EditorContext && EditorContext->bShowPath) {
		FVector PrevLoc = FVector::ZeroVector;
		FRotator PrevRot = FRotator::ZeroRotator;
		for (int64 Time : EditorContext->GetScene()->AnimationRange) {
			bool bIsKeyframe = EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeBase>("Position").GetKeyframe(Time).IsValid();
			FVector Loc = FFICAttributePosition::FromEditorAttribute(EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Position"), Time);
			if (bIsKeyframe || Loc != PrevLoc) EditorContext->GetScene()->GetWorld()->LineBatcher->DrawLine(Loc, Loc, bIsKeyframe ? FColor::Yellow : FColor::Blue, SDPG_World, 20);
			if (PrevLoc != FVector::ZeroVector) {
				EditorContext->GetScene()->GetWorld()->LineBatcher->DrawLine(PrevLoc, Loc, FColor::Red, SDPG_World, 5);
			}
			PrevLoc = Loc;
		}
	}*/

	if (EditorContext && EditorContext->bShowPath && EditorCameraActor) {
		TArray<FVector>& FramePoints = EditorCameraActor->CameraPathComponent->FramePoints;
		TSet<int64>& KeyframePoints = EditorCameraActor->CameraPathComponent->KeyframePoints;
		int64& Hovered = EditorCameraActor->CameraPathComponent->Hovered;
		if (FramePoints.Num() > 0) {
			FVector* PrevPoint = nullptr;
			for (int32 i = 0; i < FramePoints.Num(); ++i) {
				FVector* Point = &FramePoints[i];

				FColor PointColor = FColor::Blue;
				bool bIsKeyframe = KeyframePoints.Contains(i);
				if (Hovered == i) PointColor = FColor::Green; 
				else if (bIsKeyframe) PointColor = FColor::Yellow;
				if (bIsKeyframe || Hovered == i || !PrevPoint || *PrevPoint != *Point) EditorContext->GetScene()->GetWorld()->LineBatcher->DrawLine(*Point, *Point, PointColor, SDPG_World, 20);

				if (PrevPoint) EditorContext->GetScene()->GetWorld()->LineBatcher->DrawLine(*PrevPoint, *Point,  FColor::Red, SDPG_World, 5);

				PrevPoint = Point;
			}
		}
	}
}

UObject* UFICCamera::CreateNewObject(UObject* InOuter, AFICScene* InScene) {
	UFICCamera* Camera = NewObject<UFICCamera>(InOuter);
	APlayerController* Player = InOuter->GetWorld()->GetFirstPlayerController(); 
	FVector Pos = Player->PlayerCameraManager->GetCameraLocation();
	FRotator Rot = Player->PlayerCameraManager->GetCameraRotation();
	float FOVVal = Player->PlayerCameraManager->GetFOVAngle();
	Position.SetDefaultValue(Pos);
	Rotation.SetDefaultValue(Rot);
	FOV.SetDefaultValue(FOVVal);
	return Camera;
}

TSharedRef<SWidget> UFICCamera::CreateDetailsWidget(UFICEditorContext* InContext) {
	return InContext->GetEditorAttributes()[this]->CreateDetailsWidget(InContext);
}

void UFICCamera::InitEditor(UFICEditorContext* Context) {
	EditorContext = Context;

	EditorCameraActor = Context->GetScene()->GetWorld()->SpawnActor<AFICEditorCameraActor>();
	EditorCameraActor->Initialize(EditorContext, this);
	EditorCameraActor->UpdateValues(Context->GetEditorAttributes()[this]);
}

void UFICCamera::ShutdownEditor(UFICEditorContext* Context) {
	if (EditorCameraActor) {
		EditorCameraActor->Destroy();
		EditorCameraActor = nullptr;
	}
	EditorContext = nullptr;
}

void UFICCamera::EditorUpdate(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) {
	EditorCameraActor->UpdateValues(Attribute);
	if (EditorContext->GetActiveCamera() == this) EditorContext->UpdateCharacterValues();
}

void UFICCamera::Select(UFICEditorContext* Context) {
	if (Context->GetCameraPreview()) {
		CameraPreviewWidget = SNew(SConstraintCanvas)
		+SConstraintCanvas::Slot()
		.Anchors(FAnchors(0.6, 0.6, 1,1))
		.Offset(FMargin(0))[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			.Content()[
				EditorCameraActor->GetCameraPreview()
			]
		];
		Context->AddOverlayWidget(CameraPreviewWidget.ToSharedRef());
	}
}

void UFICCamera::Unselect(UFICEditorContext* Context) {
	if (CameraPreviewWidget) {
		Context->RemoveOverlayWidget(CameraPreviewWidget.ToSharedRef());
		CameraPreviewWidget.Reset();
	}
}

FTransform UFICCamera::GetSceneObjectTransform() {
	FVector Pos = FFICAttributePosition::FromEditorAttribute( EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Position"));
	FRotator Rot = FFICAttributeRotation::FromEditorAttribute(EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Rotation"));
	return FTransform(Rot, Pos);
}

void UFICCamera::SetSceneObjectTransform(FTransform InTransform) {
	if (EditorCameraActor) {
		EditorCameraActor->SetActorTransform(InTransform);
		EditorContext->CommitAutoKeyframe(this);
		FRotator LastRotation = FFICAttributeRotation::FromEditorAttribute(EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Rotation"));
		FRotator NewRotation = UFICUtils::AdditiveRotation(LastRotation, InTransform.Rotator());
		FFICAttributePosition::ToEditorAttribute(InTransform.GetLocation(), EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Position"));
		FFICAttributeRotation::ToEditorAttribute(NewRotation, EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Rotation"));
		EditorContext->CommitAutoKeyframe(nullptr);
	}
}

AActor* UFICCamera::GetActor() { return EditorCameraActor; }
