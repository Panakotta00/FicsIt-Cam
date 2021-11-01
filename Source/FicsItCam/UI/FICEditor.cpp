#include "FICEditor.h"

#include "Engine/World.h"
#include "FICEditorContext.h"
#include "FICTimeline.h"
#include "FICDetails.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "FicsItCam/FicsItCamModule.h"
#include "FicsItCam/FICSubsystem.h"
#include "GameFramework/InputSettings.h"

FSlateColorBrush SFICEditor::Background = FSlateColorBrush(FColor::FromHex("030303"));

void SFICEditor::Construct(const FArguments& InArgs) {
	Context = InArgs._Context.Get();
	GameWidget = InArgs._GameWidget.Get();
    
	ChildSlot[
        SNew(SGridPanel)
        .FillColumn(1, 1)
        .FillRow(1, 1)
        +SGridPanel::Slot(0,0).ColumnSpan(2)[
            SNew(SOverlay)
            +SOverlay::Slot()[
                SNew(SImage)
                .Image(&Background)
            ]
            +SOverlay::Slot()[
                SNew(SHorizontalBox)
                +SHorizontalBox::Slot().AutoWidth()[
                    SNew(SButton)
                    .Text(FText::FromString("Exit"))
                    .OnClicked_Lambda([this]() {
                        AFICSubsystem::GetFICSubsystem(Context->GetWorld())->SetActiveAnimation(nullptr);
                        return FReply::Handled();
                    })
                ]
                +SHorizontalBox::Slot().FillWidth(1)[
                    SNew(SSpacer)
                ]
            ]
        ]
        +SGridPanel::Slot(1,1)[
			GameWidget.ToSharedRef()
		]
		+SGridPanel::Slot(0, 1)[
			SNew(SBox)
			.WidthOverride(400)
			.Content()[
				SNew(SFICDetails)
				.Context(Context)
			]
		]
		+SGridPanel::Slot(0, 2).ColumnSpan(2)[
			SNew(SFICTimelinePanel)
			.Context(Context)
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
			APlayerController* Controller = Context->GetWorld()->GetFirstPlayerController();
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(Controller);
		} else {
			FSlateApplication::Get().SetUserFocusToGameViewport(InKeyEvent.GetUserIndex());
			APlayerController* Controller = Context->GetWorld()->GetFirstPlayerController();
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(Controller);
		}
		return FReply::Handled();
	} else if (!GameWidget->HasAnyUserFocusOrFocusedDescendants()) {
		if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleAllKeyframes"))) {
			int64 Time = Context->GetCurrentFrame();
			if (Context->PosX.GetKeyframe(Time) && Context->PosY.GetKeyframe(Time) && Context->PosZ.GetKeyframe(Time) && Context->RotPitch.GetKeyframe(Time) && Context->RotYaw.GetKeyframe(Time) && Context->RotRoll.GetKeyframe(Time) && Context->FOV.GetKeyframe(Time) &&
                !Context->PosX.HasChanged(Time) && !Context->PosY.HasChanged(Time) && !Context->PosZ.HasChanged(Time) && !Context->RotPitch.HasChanged(Time) && !Context->RotYaw.HasChanged(Time) && !Context->RotRoll.HasChanged(Time) && !Context->FOV.HasChanged(Time)) {
				Context->All.RemoveKeyframe(Time);
                } else {
                	Context->All.SetKeyframe(Time);
                }
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
			TSharedPtr<FFICKeyframeRef> KF;
			if (Context->All.GetAttribute()->GetPrevKeyframe(Context->GetCurrentFrame(), Time, KF)) Context->SetCurrentFrame(Time);
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.NextKeyframe"))) {
			int64 Time;
			TSharedPtr<FFICKeyframeRef> KF;
			if (Context->All.GetAttribute()->GetNextKeyframe(Context->GetCurrentFrame(), Time, KF)) Context->SetCurrentFrame(Time);
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
		int64 Range = Context->GetAnimation()->AnimationEnd - Context->GetAnimation()->AnimationStart;
		while (Range > 300) {
			Range /= 10;
			Delta *= 10;
		}
		Context->SetCurrentFrame(Context->GetCurrentFrame() + Delta);
		return FReply::Handled();
	} else if (MouseEvent.GetModifierKeys().IsShiftDown()) {
		float Delta = MouseEvent.GetWheelDelta();
		Context->SetFlySpeed(Context->GetFlySpeed() + Delta);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

bool SFICEditor::SupportsKeyboardFocus() const {
	return true;
}

void SFICEditor::OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) {
	SCompoundWidget::OnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);
	if (!PreviousFocusPath.ContainsWidget(GameWidget.ToSharedRef()) && NewWidgetPath.ContainsWidget(GameWidget.ToSharedRef())) {
		APlayerController* Controller = Context->GetWorld()->GetFirstPlayerController();
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(Controller);
	}
}

FReply SFICEditor::OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleCursor"))) {
		if (GameWidget->HasUserFocusedDescendants(InKeyEvent.GetUserIndex())) {
			FSlateApplication::Get().SetUserFocus(InKeyEvent.GetUserIndex(), SharedThis(this));
			APlayerController* Controller = Context->GetWorld()->GetFirstPlayerController();
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(Controller);
		} else {
			FSlateApplication::Get().SetUserFocusToGameViewport(InKeyEvent.GetUserIndex());
			APlayerController* Controller = Context->GetWorld()->GetFirstPlayerController();
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(Controller);
		}
		return FReply::Handled();
	}
	return SCompoundWidget::OnPreviewKeyDown(MyGeometry, InKeyEvent);
}

