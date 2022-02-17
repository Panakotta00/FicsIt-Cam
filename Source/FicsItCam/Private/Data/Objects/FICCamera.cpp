#include "Data/Objects/FICCamera.h"

#include "Components/LineBatchComponent.h"
#include "Editor/FICEditorCameraActor.h"
#include "Editor/FICEditorContext.h"

void UFICCamera::Tick(float DeltaTime) {
	// Draw Path
	if (EditorContext && EditorContext->bShowPath) {
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
	}
}

void UFICCamera::SetSceneObjectName(FString Name) {
	SceneObjectName = Name;
}

void UFICCamera::InitEditor(UFICEditorContext* Context) {
	EditorContext = Context;

	EditorCameraActor = Context->GetScene()->GetWorld()->SpawnActor<AFICEditorCameraActor>();
	EditorCameraActor->EditorContext = EditorContext;
	EditorCameraActor->Camera = this;
	EditorCameraActor->UpdateValues(Context->GetEditorAttributes()[this]);
}

void UFICCamera::UnloadEditor(UFICEditorContext* Context) {
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
