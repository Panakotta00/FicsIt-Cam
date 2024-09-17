#include "Editor/UI/FICEditor.h"

#include "EnhancedPlayerInput.h"
#include "FICConfigurationStruct.h"
#include "FICSubsystem.h"
#include "FICUtils.h"
#include "Engine/World.h"
#include "Editor/FICEditorContext.h"
#include "Editor/FICEditorSubsystem.h"
#include "Editor/Data/FICEditorAttributeGroupDynamic.h"
#include "Slate/SceneViewport.h"
#include "Widgets/Docking/SDockTab.h"
#include "Editor/UI/FICTimeline.h"
#include "Editor/UI/FICEditorSettings.h"
#include "Editor/UI/FICSceneObjectCreation.h"
#include "Editor/UI/FICSceneObjectDetails.h"
#include "Editor/UI/FICSceneObjectOutliner.h"
#include "Editor/UI/FICSceneSettings.h"
#include "Editor/UI/FICViewport.h"
#include "Widgets/Layout/SDPIScaler.h"

#define LOCTEXT_NAMESPACE "FicsItCam.Editor"

FSlateColorBrush SFICEditor::Background = FSlateColorBrush(FColor::FromHex("030303"));

void SFICEditor::Construct(const FArguments& InArgs, UFICEditorContext* InContext, TSharedPtr<SWidget> InGameWidget, TSharedPtr<SViewport> InViewport) {
	Context = InContext;
	GameWidget = InGameWidget;
	GameViewport = InViewport;
	
	TSharedRef<SDockTab> MajorTab = SNew(SDockTab).TabRole(ETabRole::MajorTab);
	TabManager = FGlobalTabmanager::Get()->NewTabManager(MajorTab);
	TabManager->SetOnPersistLayout(FTabManager::FOnPersistLayout::CreateLambda([this](const TSharedRef<FTabManager::FLayout>& Layout) {
		AFICEditorSubsystem::GetFICEditorSubsystem(Context)->LastEditorLayout = Layout->ToString();
	}));

	RegisterTabs();

	DefaultLayout = FTabManager::NewLayout("Default")
	->AddArea(FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split(FTabManager::NewSplitter()
			->SetSizeCoefficient(0.7)
			->Split(FTabManager::NewSplitter()
				->SetSizeCoefficient(0.2)
				->SetOrientation(Orient_Vertical)
				->Split(FTabManager::NewStack()->AddTab("New Scene Object", ETabState::OpenedTab))
				->Split(FTabManager::NewStack()->AddTab("Scene Object Outliner", ETabState::OpenedTab))
				->Split(FTabManager::NewStack()->AddTab("Scene Object Details", ETabState::OpenedTab))
				->Split(FTabManager::NewStack()->AddTab("Scene Settings", ETabState::OpenedTab))
				->Split(FTabManager::NewStack()->AddTab("Editor Settings", ETabState::OpenedTab)))
			->Split(FTabManager::NewStack()->AddTab("Viewport", ETabState::OpenedTab)->SetSizeCoefficient(0.8)->SetHideTabWell(true)))
		->Split(FTabManager::NewStack()->AddTab("Timeline", ETabState::OpenedTab)->SetSizeCoefficient(0.3)->SetHideTabWell(true)));

	FFICConfigurationStruct Config = FFICConfigurationStruct::GetActiveConfig(Context);
	
	ChildSlot[
		SNew(SDPIScaler)
		.DPIScale(Config.DPIScaling)
		.Content()[
			SNew(SOverlay)
			+SOverlay::Slot()[
				SNew(SImage)
				.Image(&Background)
			]
			+SOverlay::Slot()[
				SNew(SVerticalBox)
				+SVerticalBox::Slot().AutoHeight()[
					CreateMenuBar().MakeWidget()
				]
				+SVerticalBox::Slot().Expose(EditorSlot)
			]
		]
	];

	LoadLayout(nullptr);
}

void SFICEditor::RegisterTabs() {
	TabManager->RegisterTabSpawner("Viewport", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		TSharedRef<SDockTab> ViewportTab = SNew(SDockTab)
			.Content()[
				SNew(SFICViewport, Context, GameWidget.ToSharedRef())
				//GameWidget.ToSharedRef()
			].OnCanCloseTab(false)
			.Label(FText::FromString("Viewport"));
		TabManager->SetMainTab(ViewportTab);
		return ViewportTab;
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));

	TabManager->RegisterTabSpawner("Scene Object Outliner", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SFICSceneObjectOutliner, Context)
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));

	TabManager->RegisterTabSpawner("Scene Object Details", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SScrollBox)
			+SScrollBox::Slot().VAlign(VAlign_Fill).HAlign(HAlign_Fill)[
				SNew(SFICSceneObjectDetails, Context)
			]
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));

	TabManager->RegisterTabSpawner("Scene Settings", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SFICSceneSettings, Context)
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));

	TabManager->RegisterTabSpawner("Editor Settings", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SFICEditorSettings, Context)
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));
	
	TabManager->RegisterTabSpawner("Timeline", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SFICTimelinePanel, Context)
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));

	TabManager->RegisterTabSpawner("New Scene Object", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SFICSceneObjectCreation, Context)
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));
}

FMenuBarBuilder SFICEditor::CreateMenuBar() {
	FMenuBarBuilder MenuBarBuilder(nullptr);
	MenuBarBuilder.AddMenuEntry(LOCTEXT("Exit", "Exit"), LOCTEXT("ExitTT", "Closes and Exits the Editor"), FSlateIcon(), FExecuteAction::CreateLambda([this]() {
		TabManager->SavePersistentLayout();
		FSlateApplication::Get().ClearAllUserFocus();
		AFICEditorSubsystem::GetFICEditorSubsystem(Context->GetScene()->GetWorld())->CloseEditor();
	}));
	MenuBarBuilder.AddPullDownMenu(LOCTEXT("View", "View"), LOCTEXT("ViewTT", "Views, Panels & Windows"), FNewMenuDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder) {
		TabManager->PopulateLocalTabSpawnerMenu(MenuBuilder);
	}));
	MenuBarBuilder.AddPullDownMenu(LOCTEXT("Layout", "Layout"), LOCTEXT("LayoutTT", "Editor Panel/View Layouts"), FNewMenuDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder) {
		MenuBuilder.AddMenuEntry(LOCTEXT("Default", "Default"), LOCTEXT("DefaultTT", "Default Editor Layout"), FSlateIcon(), FExecuteAction::CreateLambda([this]() {
			LoadLayout(DefaultLayout.ToSharedRef());
		}));
		AFICEditorSubsystem* SubSys = AFICEditorSubsystem::GetFICEditorSubsystem(Context);
		if (SubSys->EditorLayouts.Num() > 0) {
			MenuBuilder.AddSeparator();
			for (const TPair<FString, FString> Layout : SubSys->EditorLayouts) {
				MenuBuilder.AddMenuEntry(FText::FromString(Layout.Key), TAttribute<FText>(), FSlateIcon(), FExecuteAction::CreateLambda([this, Layout]() {
					LoadLayout(FTabManager::FLayout::NewFromString(Layout.Value));
				}));
			}
		}
		MenuBuilder.AddSeparator();
		MenuBuilder.AddSubMenu(LOCTEXT("LayoutManager", "Manage Layouts"), LOCTEXT("LayoutManagerTT", "Opens a window that allows you to control your collection of layouts."), FNewMenuDelegate::CreateLambda([this, SubSys](FMenuBuilder& MenuBuilder) {
			MenuBuilder.AddWrapperSubMenu(LOCTEXT("NewLayout", "New Layout"), LOCTEXT("NewLayoutTT", "Stores your current layout for later reuse."), FOnGetContent::CreateLambda([this, SubSys]() {
				TSharedRef<SEditableTextBox> TextBox = SNew(SEditableTextBox).MinDesiredWidth(100);
				
				return SNew(SGridPanel)
				+SGridPanel::Slot(0, 0)
				.Padding(5)
				.VAlign(VAlign_Center)[
					SNew(STextBlock)
					.Text(LOCTEXT("NewLayout_LayoutName", "Layout Name: "))
				]
				+SGridPanel::Slot(1, 0)
				.Padding(5)[
					TextBox
				]
				+SGridPanel::Slot(0, 1)
				.ColumnSpan(2)
				.HAlign(HAlign_Left)
				.Padding(5)[
					SNew(SButton)
					.Text(LOCTEXT("NewLayout_StoreLayout", "Store Layout"))
					.OnClicked_Lambda([this, SubSys, TextBox]() {
						FString Text = TextBox->GetText().ToString().TrimStartAndEnd();
						if (Text.Len() > 0) {
							SubSys->EditorLayouts.Add(Text, TabManager->PersistLayout()->ToString());
							// TODO: Maybe add notification for successful save of layout
						}
						return FReply::Handled();
					})
				];
			}), FSlateIcon());
			if (SubSys->EditorLayouts.Num() > 0) {
				MenuBuilder.AddSeparator();
				for (const TPair<FString, FString> Layout : SubSys->EditorLayouts) {
					MenuBuilder.AddMenuEntry(FText::FromString(LOCTEXT("RemoveLayout", "Remove ").ToString() + Layout.Key), TAttribute<FText>(), FSlateIcon(), FExecuteAction::CreateLambda([this, Layout, SubSys]() {
						SubSys->EditorLayouts.Remove(Layout.Key);
					}));
				}
			}
		}));
	}));
	return MenuBarBuilder;
}

void SFICEditor::LoadLayout(TSharedPtr<FTabManager::FLayout> Layout) {
	if (!Layout) {
		FString LastLayout = AFICEditorSubsystem::GetFICEditorSubsystem(Context)->LastEditorLayout;
		if (LastLayout.Len() > 0) {
			Layout = FTabManager::FLayout::NewFromString(LastLayout);
		}
	}
	if (!Layout) Layout = DefaultLayout.ToSharedRef();

	EditorSlot->DetachWidget();
	TabManager->CloseAllAreas();
	EditorSlot->AttachWidget(TabManager->RestoreFrom(Layout.ToSharedRef(), FSlateApplication::Get().FindWidgetWindow(SharedThis(this))).ToSharedRef());
}

void SFICEditor::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	if (bIsLeft || bIsRight) {
		KeyPressTime += InDeltaTime;
		while (KeyPressTime > 0.5) {
			Context->SetCurrentFrame(Context->GetCurrentFrame() + (bIsLeft ? -1 : 1));
			KeyPressTime -= 0.2;
		}
	}

	FVector2D ViewportSize = GameWidget->GetCachedGeometry().GetAbsoluteSize();
	FVector2D GameSize = FVector2D(Context->GetScene()->ResolutionWidth, Context->GetScene()->ResolutionHeight);
	FVector2D Size = Context->GetScene()->GetWorld()->GetGameViewport()->GetGameViewport()->GetSizeXY();
	Size *= Scalability::GetResolutionScreenPercentage() / 100.0;
	float SecondaryPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SecondaryScreenPercentage.GameViewport"))->GetFloat();
	if (SecondaryPercentage) Size *= SecondaryPercentage / 100.0;
	Context->SensorWidthAdjust = GameSize.X / Size.X;
}

FReply SFICEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	FInputDeviceState InputState;
	InputState.InputDevice = EInputDevices::Keyboard;
	InputState.SetModifierKeyStates(
		InKeyEvent.GetModifierKeys().IsShiftDown(), InKeyEvent.GetModifierKeys().IsAltDown(),
		InKeyEvent.GetModifierKeys().IsControlDown(), InKeyEvent.GetModifierKeys().IsCommandDown());
	InputState.Keyboard.ActiveKey.Button = InKeyEvent.GetKey();
	InputState.Keyboard.ActiveKey.SetStates(true, true, !false);
	AFICEditorSubsystem::GetFICEditorSubsystem(Context)->ToolsContext->InputRouter->PostInputEvent(InputState);
	
	if (InKeyEvent.GetKey() == EKeys::Delete) {
		if (Context->GetSelectedSceneObject()) {
			Context->ChangeList.PushChange(MakeShared<FFICChange_RemoveSceneObject>(Context, Context->GetSelectedSceneObject()));
			Context->RemoveSceneObject(Context->GetSelectedSceneObject());
		}
		return FReply::Handled();
	} else if (InKeyEvent.GetKey() == EKeys::Escape) {
		UInteractiveToolManager* ToolManager = AFICEditorSubsystem::GetFICEditorSubsystem(Context)->ToolsContext->ToolManager;
		if (ToolManager->HasActiveTool(EToolSide::Mouse)) {
			ToolManager->DeactivateTool(EToolSide::Mouse, EToolShutdownType::Cancel);
		} else if (Context->GetPlayerCharacter()->IsControlView()) {
			Context->GetPlayerCharacter()->SetControlView(false);
		}
	}
	FReply Reply = SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
	if (!Reply.IsEventHandled()) {
		FInputKeyParams Params(InKeyEvent.GetKey(), IE_Pressed, 1.0);
		Context->GetWorld()->GetFirstPlayerController()->InputKey(Params);
		return FReply::Handled();
	}
	return Reply;
}

FReply SFICEditor::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	FInputDeviceState InputState;
	InputState.InputDevice = EInputDevices::Keyboard;
	InputState.SetModifierKeyStates(
		InKeyEvent.GetModifierKeys().IsShiftDown(), InKeyEvent.GetModifierKeys().IsAltDown(),
		InKeyEvent.GetModifierKeys().IsControlDown(), InKeyEvent.GetModifierKeys().IsCommandDown());
	InputState.Keyboard.ActiveKey.Button = InKeyEvent.GetKey();
	InputState.Keyboard.ActiveKey.SetStates(true, true, !false);
	AFICEditorSubsystem::GetFICEditorSubsystem(Context)->ToolsContext->InputRouter->PostInputEvent(InputState);
	
	if (UFICUtils::IsAction(Context, InKeyEvent, TEXT("FicsItCam.PrevFrame")) && bIsLeft) {
		bIsLeft = false;
		return FReply::Handled();
	} else if (UFICUtils::IsAction(Context, InKeyEvent, TEXT("FicsItCam.NextFrame")) && bIsRight) {
		bIsRight = false;
		return FReply::Handled();
	}
	FReply Reply = SCompoundWidget::OnKeyUp(MyGeometry, InKeyEvent);
	if (!Reply.IsEventHandled()) {
		FInputKeyParams Params(InKeyEvent.GetKey(), IE_Released, 1.0);
		Context->GetWorld()->GetFirstPlayerController()->InputKey(Params);
		return FReply::Handled();
	}
	return Reply;
}

FReply SFICEditor::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetModifierKeys().IsControlDown()) {
		float Delta = MouseEvent.GetWheelDelta() * 3;
		int64 Range = Context->GetScene()->AnimationRange.Length();
		while (Range > 300) {
			Range /= 10;
			Delta *= 10;
		}
		Context->SetCurrentFrame(Context->GetCurrentFrame() + Delta);
		return FReply::Handled();
	} else if (MouseEvent.GetModifierKeys().IsShiftDown()) {
		float Delta = MouseEvent.GetWheelDelta();
		//Context->SetFlySpeed(Context->GetFlySpeed() + Delta);
		// TODO: Fly Speed!
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SFICEditor::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		if (GameViewport->HasAnyUserFocusOrFocusedDescendants()) {
			AFICEditorSubsystem::GetFICEditorSubsystem(Context)->OnLeftMouseDown();
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SFICEditor::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		if (GameViewport->HasAnyUserFocusOrFocusedDescendants()) {
			AFICEditorSubsystem::GetFICEditorSubsystem(Context)->OnLeftMouseUp();
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnMouseButtonUp(MyGeometry, MouseEvent);
}

bool SFICEditor::SupportsKeyboardFocus() const {
	return true;
}

void SFICEditor::OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) {
	SCompoundWidget::OnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);
	if (!PreviousFocusPath.ContainsWidget(GameWidget.ToSharedRef()) && NewWidgetPath.ContainsWidget(GameWidget.ToSharedRef())) {
		if (FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::RightMouseButton)) {
			Context->GetPlayerCharacter()->SetControlView(true, true);
		}
	}
}

#undef LOCTEXT_NAMESPACE
