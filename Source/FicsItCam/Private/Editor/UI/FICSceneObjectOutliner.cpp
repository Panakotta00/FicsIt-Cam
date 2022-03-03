#include "Editor/UI/FICSceneObjectOutliner.h"

#include "FICSubsystem.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICKeyframeControl.h"

SFICSceneObjectOutliner::~SFICSceneObjectOutliner() {
	if (Context) Context->OnSceneObjectSelectionChanged.Remove(OnSelectionChangedDelegateHandle);
}

void SFICSceneObjectOutliner::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;

	OnSceneObjectsChangedDelegateHandle = Context->OnSceneObjectsChanged.AddSP(this, &SFICSceneObjectOutliner::UpdateSceneObjects);
	OnSelectionChangedDelegateHandle = Context->OnSceneObjectSelectionChanged.AddSP(this, &SFICSceneObjectOutliner::UpdateSelection);

	ChildSlot[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight()[
			SAssignNew(WidgetAddButton, SButton)
			.Text(FText::FromString("Add"))
			.OnClicked_Lambda([this]() {
				OpenAddSceneObjectMenu();
				return FReply::Handled();
			})
		]
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
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth()[
						SNew(SBox)
						.VAlign(VAlign_Center)
						.MinDesiredWidth(20)[
							SNew(SEditableText)
							.IsEnabled_Lambda([this, SceneObject]() {
								return SceneObjectListWidget->IsItemSelected(SceneObject);
							})
							.IsReadOnly_Lambda([this, SceneObject]() {
								return !SceneObjectListWidget->IsItemSelected(SceneObject);
							})
							.OnTextCommitted_Lambda([SceneObject](const FText& Text, ETextCommit::Type Type) {
								Cast<IFICSceneObject>(SceneObject->SceneObject)->SetSceneObjectName(Text.ToString());
							})
							.Text_Lambda([SceneObject]() {
								return FText::FromString(Cast<IFICSceneObject>(SceneObject->SceneObject)->GetSceneObjectName());
							})
						]
					]
					+SHorizontalBox::Slot().FillWidth(1)
					+SHorizontalBox::Slot().AutoWidth().Padding(5).VAlign(VAlign_Center).HAlign(HAlign_Center)[
						SNew(SButton)
						.Visibility_Lambda([Row]() {
							return Row->IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
						})
						.Text(FText::FromString("/\\"))
						.OnClicked_Lambda([this, SceneObject]() {
							Context->MoveSceneObject(SceneObject->SceneObject, -1);
							return FReply::Handled();
						})
					]
					+SHorizontalBox::Slot().AutoWidth().Padding(5).VAlign(VAlign_Center).HAlign(HAlign_Center)[
						SNew(SButton)
						.Visibility_Lambda([Row]() {
							return Row->IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
						})
						.Text(FText::FromString("\\/"))
						.OnClicked_Lambda([this, SceneObject]() {
							Context->MoveSceneObject(SceneObject->SceneObject, 1);
							return FReply::Handled();
						})
					]
					+SHorizontalBox::Slot().AutoWidth().Padding(5).VAlign(VAlign_Center).HAlign(HAlign_Center)[
						SNew(SFICKeyframeControl, Context, Context->GetEditorAttributes()[SceneObject->SceneObject])
						.Frame_Lambda([this]() {
							return Context->GetCurrentFrame();
						})
					]
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

void SFICSceneObjectOutliner::OpenAddSceneObjectMenu() {
	FMenuBuilder MenuBuilder(true, NULL);
	for (TObjectIterator<UClass> Class; Class; ++Class) {
		if (!Class->ImplementsInterface(UFICSceneObject::StaticClass())) continue;
		UObject* Obj = Class->GetDefaultObject();
		IFICSceneObject* SceneObj = Cast<IFICSceneObject>(Obj);
		MenuBuilder.AddMenuEntry(
	        FText::FromString(SceneObj->GetSceneObjectName()),
	        FText(),
	        FSlateIcon(),
			FExecuteAction::CreateLambda([Class, this]() {
				UObject* CDO = Class->GetDefaultObject();
				UObject* SceneObject = Cast<IFICSceneObject>(CDO)->CreateNewObject(AFICSubsystem::GetFICSubsystem(Context), Context->GetScene());
	            Context->AddSceneObject(SceneObject);
	        }));
	}
	FWidgetPath WidgetPath;
	FSlateApplication::Get().GeneratePathToWidgetChecked(SharedThis(this), WidgetPath);
	FSlateApplication::Get().PushMenu(SharedThis(this), WidgetPath, MenuBuilder.MakeWidget(), WidgetAddButton->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(0, 1)), FPopupTransitionEffect::ContextMenu);
}
