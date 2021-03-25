#include "FICEditor.h"

#include "Engine/World.h"
#include "FICEditorContext.h"
#include "FICTimeline.h"
#include "FICDetails.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "FicsItCam/FicsItCamModule.h"
#include "FicsItCam/FICSubsystem.h"

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

void Dump(int Ident, TSharedPtr<SWidget> Widget) {
	FString IdentStr = FString::ChrN(Ident, ' ');
	if (!Widget.IsValid()) {
		UE_LOG(LogFicsItCam, Warning, TEXT("%sNull"), *IdentStr);
		return;
	}
	FVector2D Size = Widget->GetDesiredSize();
	FGeometry Geo = Widget->GetCachedGeometry();
	UE_LOG(LogFicsItCam, Warning, TEXT("%sWidget: %s %fx%f %f %f"), *IdentStr, *Widget->GetTypeAsString(), Size.X, Size.Y, Geo.AbsolutePosition.X, Geo.AbsolutePosition.Y);
	FChildren* Children = Widget->GetAllChildren();
	if (Children) for (int i = 0; i < Children->Num(); ++i) {
		Dump(Ident+1, Children->GetChildAt(i));
	}
}

void SFICEditor::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	static bool firstTick = true;
	if (firstTick) {
		Dump(0, FSlateApplication::Get().GetActiveTopLevelWindow());
		firstTick = false;
	}

	if (bIsLeft || bIsRight) {
		KeyPressTime += InDeltaTime;
		while (KeyPressTime > 0.5) {
			Context->SetCurrentFrame(Context->GetCurrentFrame() + (bIsLeft ? -1 : 1));
			KeyPressTime -= 0.2;
		}
	}
}

FReply SFICEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::RightAlt) {
		if (GameWidget->HasUserFocus(InKeyEvent.GetUserIndex())) {
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
		if (InKeyEvent.GetKey() == EKeys::I) {
			int64 Time = Context->GetCurrentFrame();
			if (Context->PosX.GetKeyframe(Time) && Context->PosY.GetKeyframe(Time) && Context->PosZ.GetKeyframe(Time) && Context->RotPitch.GetKeyframe(Time) && Context->RotYaw.GetKeyframe(Time) && Context->RotRoll.GetKeyframe(Time) && Context->FOV.GetKeyframe(Time) &&
                !Context->PosX.HasChanged(Time) && !Context->PosY.HasChanged(Time) && !Context->PosZ.HasChanged(Time) && !Context->RotPitch.HasChanged(Time) && !Context->RotYaw.HasChanged(Time) && !Context->RotRoll.HasChanged(Time) && !Context->FOV.HasChanged(Time)) {
				Context->All.RemoveKeyframe(Time);
                } else {
                	Context->All.SetKeyframe(Time);
                }
			return FReply::Handled();
		} else if (InKeyEvent.GetKey() == EKeys::Left) {
			int64 Rate = 1;
			if (InKeyEvent.GetModifierKeys().IsControlDown()) Rate = 10;
			Context->SetCurrentFrame(Context->GetCurrentFrame()-Rate);
			bIsLeft = true;
			KeyPressTime = 0;
			return FReply::Handled();
		} else if (InKeyEvent.GetKey() == EKeys::Right) {
			int64 Rate = 1;
			if (InKeyEvent.GetModifierKeys().IsControlDown()) Rate = 10;
			Context->SetCurrentFrame(Context->GetCurrentFrame()+Rate);
			bIsRight= true;
			KeyPressTime = 0;
			return FReply::Handled();
		} else if (InKeyEvent.GetKey() == EKeys::N) {
			int64 Time;
			TSharedPtr<FFICKeyframeRef> KF;
			if (Context->All.GetAttribute()->GetPrevKeyframe(Context->GetCurrentFrame(), Time, KF)) Context->SetCurrentFrame(Time);
			return FReply::Handled();
		} else if (InKeyEvent.GetKey() == EKeys::M) {
			int64 Time;
			TSharedPtr<FFICKeyframeRef> KF;
			if (Context->All.GetAttribute()->GetNextKeyframe(Context->GetCurrentFrame(), Time, KF)) Context->SetCurrentFrame(Time);
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SFICEditor::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Left && bIsLeft) {
		bIsLeft = false;
		return FReply::Handled();
	} else if (InKeyEvent.GetKey() == EKeys::Right && bIsRight) {
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

#pragma optimize("", on)
