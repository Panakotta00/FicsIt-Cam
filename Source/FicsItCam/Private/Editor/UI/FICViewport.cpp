#include "Editor/UI/FICViewport.h"

#include "FICSubsystem.h"
#include "Components/LineBatchComponent.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICSceneObjectCreation.h"

SFICViewport::~SFICViewport() {
	if (Context) Context->OnOverlayWidgetsChanged.Remove(OnViewportOverlaysChangedHandle);
}

void SFICViewport::Construct(const FArguments& InArgs, UFICEditorContext* InContext, TSharedRef<SWidget> InGameWidget) {
	Context = InContext;
	GameWidget = InGameWidget;

	ChildSlot[
		SAssignNew(Overlay, SOverlay)
	];

	OnViewportOverlaysChangedHandle = Context->OnOverlayWidgetsChanged.AddSP(this, &SFICViewport::UpdateViewportOverlays);

	UpdateViewportOverlays();
}

void SFICViewport::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
	TSharedPtr<FFICSceneObjectDragDrop> SceneObjectDrag = DragDropEvent.GetOperationAs<FFICSceneObjectDragDrop>();
	if (SceneObjectDrag) {
		if (!SceneObjectDrag->SceneObject && SceneObjectDrag->bSceneObjectTemp) {
			UObject* CDO = SceneObjectDrag->SceneObjectClass->GetDefaultObject();
			UObject* SceneObject = Cast<IFICSceneObject>(CDO)->CreateNewObject(AFICSubsystem::GetFICSubsystem(Context), Context->GetScene());
			Context->AddSceneObject(SceneObject);
			SceneObjectDrag->SceneObject = SceneObject;
		}
	}
}

FReply SFICViewport::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
	TSharedPtr<FFICSceneObjectDragDrop> SceneObjectDrag = DragDropEvent.GetOperationAs<FFICSceneObjectDragDrop>();
	if (SceneObjectDrag) {
		UObject* Object = SceneObjectDrag->SceneObject;
		IFICSceneObject3D* SceneObject3D = Cast<IFICSceneObject3D>(Object);
		if (SceneObject3D) {
			FHitResult Hit;
			APlayerController* PlayerController = Cast<APlayerController>(Context->GetPlayerCharacter()->GetController());
			FVector WorldOrigin;
			FVector WorldDirection;
			FVector2D MousePosition = MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());
			UGameplayStatics::DeprojectScreenToWorld(PlayerController, MousePosition, WorldOrigin, WorldDirection);
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(SceneObject3D->GetActor());
			if (Object->GetWorld()->LineTraceSingleByChannel(Hit, WorldOrigin, WorldOrigin + WorldDirection * 10000, ECollisionChannel::ECC_Camera, Params)) {
				FTransform Transform = SceneObject3D->GetSceneObjectTransform();
				Transform.SetLocation(Hit.Location);
				SceneObject3D->SetSceneObjectTransform(Transform);
				return FReply::Handled();
			}
		}
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SFICViewport::OnDragLeave(const FDragDropEvent& DragDropEvent) {
	TSharedPtr<FFICSceneObjectDragDrop> SceneObjectDrag = DragDropEvent.GetOperationAs<FFICSceneObjectDragDrop>();
	if (SceneObjectDrag) {
		if (SceneObjectDrag->SceneObject && SceneObjectDrag->bSceneObjectTemp) {
			Context->RemoveSceneObject(SceneObjectDrag->SceneObject);
			SceneObjectDrag->SceneObject = nullptr;
		}
	}
}

void SFICViewport::UpdateViewportOverlays() {
	Overlay->ClearChildren();
	Overlay->AddSlot()[
		GameWidget.ToSharedRef()
	];
	for (TSharedRef<SWidget> Widget : Context->GetOverlayWidgets()) {
		Overlay->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)[
			Widget
		];
	}
}
