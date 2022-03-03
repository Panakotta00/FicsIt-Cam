#include "Editor/UI/FICSceneObjectCreation.h"

#include "FICSubsystem.h"
#include "Data/Objects/FICSceneObject.h"
#include "Editor/FICEditorContext.h"
#include "Widgets/Layout/SScrollBox.h"

FFICSceneObjectDragDrop::FFICSceneObjectDragDrop(UClass* InSceneObjectClass, bool bSceneObjectTemp): SceneObjectClass(InSceneObjectClass), bSceneObjectTemp(bSceneObjectTemp) {}
FFICSceneObjectDragDrop::FFICSceneObjectDragDrop(UObject* InSceneObject, bool bSceneObjectTemp) : SceneObject(InSceneObject), bSceneObjectTemp(bSceneObjectTemp) {}

FSlateColorBrush SFICSceneObjectCreationRow::DefaultHoverBackground = FSlateColorBrush(FColor::FromHex("333333"));

void SFICSceneObjectCreationRow::Construct(const FArguments& InArgs, UFICEditorContext* InContext, UClass* InSceneObjectClass) {
	Context = InContext;
	SceneObjectClass = InSceneObjectClass;
	
	ChildSlot[
		SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(&DefaultHoverBackground)
			.Visibility_Lambda([this]() {
				return IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
			})
		]
		+SOverlay::Slot()[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(5)[
				SNew(STextBlock)
				.Text(FText::FromString(Cast<IFICSceneObject>(SceneObjectClass->GetDefaultObject())->GetSceneObjectName()))
			]
		]
	];
}

FReply SFICSceneObjectCreationRow::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		bClick = true;
		return FReply::Handled().CaptureMouse(AsShared()).DetectDrag(AsShared(), EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

FReply SFICSceneObjectCreationRow::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		if (bClick && IsHovered()) {
			UObject* CDO = SceneObjectClass->GetDefaultObject();
			UObject* SceneObject = Cast<IFICSceneObject>(CDO)->CreateNewObject(AFICSubsystem::GetFICSubsystem(Context), Context->GetScene());
			Context->AddSceneObject(SceneObject);
			return FReply::Handled().ReleaseMouseCapture();
		}
	}
	return FReply::Unhandled().ReleaseMouseCapture();
}

FReply SFICSceneObjectCreationRow::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled().ReleaseMouseCapture().BeginDragDrop(MakeShared<FFICSceneObjectDragDrop>(SceneObjectClass, true));
}

void SFICSceneObjectCreation::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;

	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);
	ChildSlot[
		ScrollBox
	];

	for (TObjectIterator<UClass> Class; Class; ++Class) {
		if (!Class->ImplementsInterface(UFICSceneObject::StaticClass())) continue;
		UObject* Obj = Class->GetDefaultObject();
		IFICSceneObject* SceneObj = Cast<IFICSceneObject>(Obj);

		ScrollBox->AddSlot()[
			SNew(SFICSceneObjectCreationRow, Context, *Class)
		];
	}
}
