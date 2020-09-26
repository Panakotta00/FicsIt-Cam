#include "FICEditor.h"

#include "FICTimeline.h"
#include "FICDetails.h"

void SFICEditor::Construct(const FArguments& InArgs) {
	Context = InArgs._Context.Get();
	GameWidget = InArgs._GameWidget.Get();

	Children.Add(SNew(SGridPanel)
		.FillColumn(1, 1)
		.FillRow(1, 1)
		+SGridPanel::Slot(1,1)[
			SAssignNew(GameSpacer, SSpacer).Size(FVector2D(200, 200))
		]
		+SGridPanel::Slot(0, 1)[
			SNew(SFICDetails)
			.Context(Context)
		]
		+SGridPanel::Slot(0, 2).ColumnSpan(3)[
			SNew(SFICTimelinePanel)
			.Context(Context)
		]
	);
	
	TSharedPtr<SHorizontalBox> GameViewportContainer = StaticCastSharedPtr<SHorizontalBox>(GameWidget->GetParentWidget());
	GameViewportContainer->RemoveSlot(GameWidget.ToSharedRef());
	GameSlot = &GameViewportContainer->AddSlot();
	(*GameSlot)[
		GameWidget.ToSharedRef()
	];
}

SFICEditor::SFICEditor() : Children(this) {}

void SFICEditor::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Children[0], FVector2D(0, 0), AllottedGeometry.GetLocalSize()));
}

FVector2D SFICEditor::ComputeDesiredSize(float) const {
	return Children[0]->GetDesiredSize();
}

FChildren* SFICEditor::GetChildren() {
	return &Children;
}

void SFICEditor::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SPanel::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	if (GameSpacer.IsValid()) {
		TSharedPtr<SHorizontalBox> GameViewportContainer = StaticCastSharedPtr<SHorizontalBox>(GameWidget->GetParentWidget());
		GameSpacer->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(0, 0));
		FVector2D Pos = GameViewportContainer->GetCachedGeometry().AbsoluteToLocal(FVector2D(0,0));
		FVector2D Pos2 = GameViewportContainer->GetCachedGeometry().AbsoluteToLocal(GameSpacer->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(1, 1)));
		if (Pos != Pos2) {
			FVector2D Size = GameViewportContainer->GetCachedGeometry().GetLocalSize();
			FMargin Padding;
			Padding.Top = Pos.Y;
			Padding.Left = Pos.X;
			Padding.Bottom = Size.Y - Pos2.Y;
			Padding.Right = Size.X - Pos2.X;
		
			if (GameSlot) GameSlot->Padding(Padding);
		}
	}
}

FReply SFICEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::LeftAlt && InKeyEvent.GetModifierKeys().IsControlDown()) {
		FSlateApplication::Get().SetAllUserFocus(GameWidget);
		return FReply::Handled();
	}
	return SPanel::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SFICEditor::OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::LeftAlt && InKeyEvent.GetModifierKeys().IsControlDown()) {
		FSlateApplication::Get().SetUserFocusToGameViewport(InKeyEvent.GetUserIndex());
		return FReply::Handled();
	}
	return SPanel::OnPreviewKeyDown(MyGeometry, InKeyEvent);
}

bool SFICEditor::IsInteractable() const {
	return true;
}

#pragma optimize("", on)
