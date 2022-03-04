#include "Editor/UI/FICSceneObjectOutliner.h"

#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICKeyframeControl.h"
#include "Editor/UI/FICSceneObjectCreation.h"

void SFICSceneObjectOutlinerRow::Construct(const FArguments& InArgs, UFICEditorContext* InContext, UObject* InSceneObject) {
	Context = InContext;
	SceneObject = InSceneObject;
	
	ChildSlot[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()[
			SNew(SBox)
			.VAlign(VAlign_Center)
			.MinDesiredWidth(20)[
				SNew(SEditableText)
				.IsEnabled_Lambda([this]() {
					return Context->GetSelectedSceneObject() == SceneObject;
				})
				.IsReadOnly_Lambda([this]() {
					return Context->GetSelectedSceneObject() != SceneObject;
				})
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type Type) {
					Cast<IFICSceneObject>(SceneObject)->SetSceneObjectName(Text.ToString());
				})
				.Text_Lambda([this]() {
					return FText::FromString(Cast<IFICSceneObject>(SceneObject)->GetSceneObjectName());
				})
			]
		]
		+SHorizontalBox::Slot().FillWidth(1)
		+SHorizontalBox::Slot().AutoWidth().Padding(5).VAlign(VAlign_Center).HAlign(HAlign_Center)[
			SNew(SButton)
			.Visibility_Lambda([this]() {
				return IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
			})
			.Text(FText::FromString("/\\"))
			.OnClicked_Lambda([this]() {
				Context->MoveSceneObject(SceneObject, -1);
				return FReply::Handled();
			})
		]
		+SHorizontalBox::Slot().AutoWidth().Padding(5).VAlign(VAlign_Center).HAlign(HAlign_Center)[
			SNew(SButton)
			.Visibility_Lambda([this]() {
				return IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
			})
			.Text(FText::FromString("\\/"))
			.OnClicked_Lambda([this]() {
				Context->MoveSceneObject(SceneObject, 1);
				return FReply::Handled();
			})
		]
		+SHorizontalBox::Slot().AutoWidth().Padding(5).VAlign(VAlign_Center).HAlign(HAlign_Center)[
			SNew(SFICKeyframeControl, Context, Context->GetEditorAttributes()[SceneObject])
			.Frame_Lambda([this]() {
				return Context->GetCurrentFrame();
			})
		]
	];
}

bool SFICSceneObjectOutlinerRow::IsInteractable() const {
	return SCompoundWidget::IsInteractable();
}

FReply SFICSceneObjectOutlinerRow::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		bClick = true;
		return FReply::Handled().CaptureMouse(AsShared()).DetectDrag(AsShared(), EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

FReply SFICSceneObjectOutlinerRow::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		if (bClick) {
			bClick = false;
			Context->SetSelectedSceneObject(SceneObject);
			return FReply::Handled().ReleaseMouseCapture();
		}
	}
	return FReply::Unhandled().ReleaseMouseCapture();
}

FReply SFICSceneObjectOutlinerRow::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled().ReleaseMouseCapture().BeginDragDrop(MakeShared<FFICSceneObjectDragDrop>(Context, SceneObject, false));
}

SFICSceneObjectOutliner::~SFICSceneObjectOutliner() {
	if (Context) Context->OnSceneObjectSelectionChanged.Remove(OnSelectionChangedDelegateHandle);
}

void SFICSceneObjectOutliner::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;

	OnSceneObjectsChangedDelegateHandle = Context->OnSceneObjectsChanged.AddSP(this, &SFICSceneObjectOutliner::UpdateSceneObjects);
	OnSelectionChangedDelegateHandle = Context->OnSceneObjectSelectionChanged.AddSP(this, &SFICSceneObjectOutliner::UpdateSelection);

	ChildSlot[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().FillHeight(1)[
			SAssignNew(SceneObjectListWidget, SListView<TSharedPtr<FFICSceneObjectReference>>)
			.ListItemsSource(&SceneObjectList)
			.SelectionMode(ESelectionMode::Single)
			.OnSelectionChanged_Lambda([this](TSharedPtr<FFICSceneObjectReference> SelectedObject, ESelectInfo::Type Type) {
				if (Type != ESelectInfo::Type::Direct) {
					if (SelectedObject) Context->SetSelectedSceneObject(SelectedObject->SceneObject);
					else Context->SetSelectedSceneObject(nullptr);
				}
			})
			.OnKeyDownHandler_Lambda([this](const FGeometry&, const FKeyEvent& Event) {
				if (Event.GetKey() == EKeys::Delete) {
					TArray<TSharedPtr<FFICSceneObjectReference>> Selected;
					if (SceneObjectListWidget->GetSelectedItems(Selected) > 0) {
						Context->RemoveSceneObject(Selected[0]->SceneObject);
					}
					return FReply::Handled();
				}
				return FReply::Unhandled();
			})
			.OnGenerateRow_Lambda([this](TSharedPtr<FFICSceneObjectReference> SceneObject, const TSharedRef<STableViewBase>& Base) {
				TSharedRef<STableRow<TSharedPtr<FFICSceneObjectReference>>> Row = SNew(STableRow<TSharedPtr<FFICSceneObjectReference>>, Base);
				Row->SetRowContent(
					SNew(SFICSceneObjectOutlinerRow, Context, SceneObject->SceneObject)
				);
				return Row;
			})
		]
	];

	UpdateSceneObjects();
	UpdateSelection();
}

void SFICSceneObjectOutliner::UpdateSceneObjects() {
	SceneObjectList.Empty();
	SceneObjectMap.Empty();
	for (UObject* SceneObject : Context->GetScene()->GetSceneObjects()) {
		TSharedRef<FFICSceneObjectReference> Ref = MakeShared<FFICSceneObjectReference>(SceneObject);
		SceneObjectList.Add(Ref);
		SceneObjectMap.Add(SceneObject, Ref);
	}
	if (SceneObjectListWidget) SceneObjectListWidget->RebuildList();
}

void SFICSceneObjectOutliner::UpdateSelection() {
	if (!SceneObjectListWidget) return;
	SceneObjectListWidget->ClearSelection();
	if (Context->GetSelectedSceneObject()) {
		SceneObjectListWidget->SetSelection(SceneObjectMap[Context->GetSelectedSceneObject()]);
	}
}
