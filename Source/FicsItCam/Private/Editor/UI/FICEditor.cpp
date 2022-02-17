#include "Editor/UI/FICEditor.h"

#include "FICSubsystem.h"
#include "Engine/World.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Editor/FICEditorContext.h"
#include "Editor/FICEditorSubsystem.h"
#include "GameFramework/InputSettings.h"
#include "Slate/SceneViewport.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Editor/UI/FICTimeline.h"
#include "Editor/UI/FICDetails.h"

FSlateColorBrush SFICEditor::Background = FSlateColorBrush(FColor::FromHex("030303"));

void SFICEditor::Construct(const FArguments& InArgs, UFICEditorContext* InContext, TSharedPtr<SWidget> InGameWidget, TSharedPtr<SViewport> InViewport) {
	Context = InContext;
	GameWidget = InGameWidget;
	GameViewport = InViewport;
	
	ChildSlot[
		SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(&Background)
		]
		+SOverlay::Slot()[
			SNew(SSplitter)
			.Orientation(EOrientation::Orient_Vertical)
			+SSplitter::Slot().Value(2)[
				SNew(SSplitter)
				.Orientation(EOrientation::Orient_Horizontal)
				+SSplitter::Slot()[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)[
						SNew(SButton)
						.Text(FText::FromString("Exit"))
						.OnClicked_Lambda([this]() {
							AFICEditorSubsystem::GetFICEditorSubsystem(Context->GetScene()->GetWorld())->CloseEditor();
							return FReply::Handled();
						})
					]
					+SVerticalBox::Slot().FillHeight(1)[
						SNew(SFICDetails)
						.Context(Context)
					]
				]
				+SSplitter::Slot().Value(4)[
					GameWidget.ToSharedRef()
				]
			]
			+SSplitter::Slot().SizeRule(SSplitter::ESizeRule::FractionOfParent)[
				SNew(SFICTimelinePanel)
				.Context(Context)
			]
		]
	];
	
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

bool IsAction(UObject* Context, const FKeyEvent& InKeyEvent, const FName& ActionName) {
	TArray<FInputActionKeyMapping> Mappings = Context->GetWorld()->GetFirstPlayerController()->PlayerInput->GetKeysForAction(ActionName);
	if (Mappings.Num() > 0) {
		const FInputActionKeyMapping& Mapping = Mappings[0];
		return Mapping.Key == InKeyEvent.GetKey() &&
			Mapping.bAlt == InKeyEvent.GetModifierKeys().IsAltDown() &&
			Mapping.bCmd == InKeyEvent.GetModifierKeys().IsCommandDown() &&
			Mapping.bCtrl == InKeyEvent.GetModifierKeys().IsControlDown() &&
			Mapping.bShift == InKeyEvent.GetModifierKeys().IsShiftDown();
	}
	return false;
}

FReply SFICEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleCursor"))) {
		if (GameWidget->HasUserFocusedDescendants(InKeyEvent.GetUserIndex())) {
			FSlateApplication::Get().SetUserFocus(InKeyEvent.GetUserIndex(), SharedThis(this));
			APlayerController* Controller = Context->GetScene()->GetWorld()->GetFirstPlayerController();
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(Controller);
		} else {
			FSlateApplication::Get().SetUserFocusToGameViewport(InKeyEvent.GetUserIndex());
			APlayerController* Controller = Context->GetScene()->GetWorld()->GetFirstPlayerController();
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(Controller);
		}
		return FReply::Handled();
	} else /*if (!GameWidget->HasAnyUserFocusOrFocusedDescendants())*/ {
		if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleAllKeyframes"))) {
			auto Change = MakeShared<FFICChange_Group>();
			Change->PushChange(MakeShared<FFICChange_ActiveFrame>(Context, TNumericLimits<int64>::Min(), Context->GetCurrentFrame()));
			BEGIN_ATTRIB_CHANGE(Context->GetAllAttributes()->GetAttribute())
			// TODO: Toggle All keyframes (dynamic check if all attributes have keyframe and if is different from current value
			END_ATTRIB_CHANGE(Change)
			Context->ChangeList.PushChange(Change);
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.PrevFrame"))) {
			int64 Rate = 1;
			if (InKeyEvent.GetModifierKeys().IsControlDown()) Rate = 10;
			Context->SetCurrentFrame(Context->GetCurrentFrame()-Rate);
			bIsLeft = true;
			KeyPressTime = 0;
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.NextFrame"))) {
			int64 Rate = 1;
			if (InKeyEvent.GetModifierKeys().IsControlDown()) Rate = 10;
			Context->SetCurrentFrame(Context->GetCurrentFrame()+Rate);
			bIsRight= true;
			KeyPressTime = 0;
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.PrevKeyframe"))) {
			int64 Time;
			TSharedPtr<FFICKeyframe> KF = Context->GetAllAttributes()->GetAttribute().GetPrevKeyframe(Context->GetCurrentFrame(), Time);
			if (KF) Context->SetCurrentFrame(Time);
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.NextKeyframe"))) {
			int64 Time;
			TSharedPtr<FFICKeyframe> KF = Context->GetAllAttributes()->GetAttribute().GetNextKeyframe(Context->GetCurrentFrame(), Time);
			if (KF) Context->SetCurrentFrame(Time);
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleAutoKeyframe"))) {
			Context->bAutoKeyframe = !Context->bAutoKeyframe;
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleShowPath"))) {
			Context->bShowPath = !Context->bShowPath;
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleLockCamera"))) {
			Context->bMoveCamera = !Context->bMoveCamera;
			return FReply::Handled();
		} else if (InKeyEvent.GetKey() == EKeys::Z && InKeyEvent.IsControlDown()) {
			TSharedPtr<FFICChange> Change = Context->ChangeList.PopChange();
			if (Change) Change->UndoChange();
			return FReply::Handled();
		} else if (InKeyEvent.GetKey() == EKeys::Y && InKeyEvent.IsControlDown()) {
			TSharedPtr<FFICChange> Change = Context->ChangeList.PushChange();
			if (Change) Change->RedoChange();
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SFICEditor::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.PrevFrame")) && bIsLeft) {
		bIsLeft = false;
		return FReply::Handled();
	} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.NextFrame")) && bIsRight) {
		bIsRight = false;
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyUp(MyGeometry, InKeyEvent);
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

FReply SFICEditor::OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleCursor")) || IsAction(Context, InKeyEvent, TEXT("PauseGame"))) {
		Context->GetPlayerCharacter()->ControlViewToggle();
		return FReply::Handled();
	}
	return SCompoundWidget::OnPreviewKeyDown(MyGeometry, InKeyEvent);
}
