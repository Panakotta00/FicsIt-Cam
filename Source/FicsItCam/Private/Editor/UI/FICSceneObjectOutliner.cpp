#include "Editor/UI/FICSceneObjectOutliner.h"

#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICKeyframeControl.h"

SFICSceneObjectOutliner::~SFICSceneObjectOutliner() {
	if (Context) Context->OnSceneObjectSelectionChanged.Remove(OnSelectionChangedDelegateHandle);
}

void SFICSceneObjectOutliner::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;

	OnSceneObjectsChangedDelegateHandle = Context->OnSceneObjectsChanged.AddRaw(this, &SFICSceneObjectOutliner::UpdateSceneObjects);
	OnSelectionChangedDelegateHandle = Context->OnSceneObjectSelectionChanged.AddRaw(this, &SFICSceneObjectOutliner::UpdateSelection);

	ChildSlot[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight()[
			SNew(SButton)
			.Text(FText::FromString("Add"))
			.OnClicked_Lambda([this]() {
				/*TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
				for (TObjectIterator<UParticleSystem> System; System; ++System) {
					List->AddSlot()[
						SNew(SButton)
						.Text(FText::FromString(System->GetName()))
						.OnClicked_Lambda([this]() {
							FVector Pos = Context->GetScene()->GetWorld()->GetFirstPlayerController()->GetCharacter()->GetActorLocation();
							UFICParticleSystem* System = NewObject<UFICParticleSystem>();
							System->Position.X.SetDefaultValue(Pos.X);
							System->Position.Y.SetDefaultValue(Pos.Y);
							System->Position.Z.SetDefaultValue(Pos.Z);
							Context->AddSceneObject(System);
							return FReply::Handled();
						})
					];
				}
				FSlateApplication::Get().AddWindow(SNew(SWindow)
					.Content()[
						SNew(SScrollBox)
						+SScrollBox::Slot()[
							List
						]
					]);*/
				FVector Pos = Context->GetPlayerCharacter()->GetActorLocation();
				FRotator Rot = Context->GetPlayerCharacter()->GetControlRotation();
				UFICCamera* Camera = NewObject<UFICCamera>();
				Camera->Position.X.SetDefaultValue(Pos.X);
				Camera->Position.Y.SetDefaultValue(Pos.Y);
				Camera->Position.Z.SetDefaultValue(Pos.Z);
				Camera->Rotation.Pitch.SetDefaultValue(Rot.Pitch);
				Camera->Rotation.Yaw.SetDefaultValue(Rot.Yaw);
				Camera->Rotation.Roll.SetDefaultValue(Rot.Roll);
				Context->AddSceneObject(Camera);
				
				return FReply::Handled();
			})
		]
		+SVerticalBox::Slot().FillHeight(1)[
			SAssignNew(SceneObjectListWidget, SListView<TSharedPtr<FFICSceneObjectReference>>)
			.ListItemsSource(&SceneObjectList)
			.SelectionMode(ESelectionMode::Single)
			.OnSelectionChanged_Lambda([this](TSharedPtr<FFICSceneObjectReference> SelectedObject, ESelectInfo::Type Type) {
				if (Type != ESelectInfo::Type::Direct) {
					if (SelectedObject) Context->SetSelection(SelectedObject->SceneObject);
					else Context->SetSelection(nullptr);
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
				return SNew(STableRow<TSharedPtr<FFICSceneObjectReference>>, Base)
				.Content()[
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
						SNew(SFICKeyframeControl, Context, Context->GetEditorAttributes()[SceneObject->SceneObject])
						.Frame_Lambda([this]() {
							return Context->GetCurrentFrame();
						})
					]
				];
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
	if (Context->Selection) {
		SceneObjectListWidget->SetSelection(SceneObjectMap[Context->Selection]);
	}
}
