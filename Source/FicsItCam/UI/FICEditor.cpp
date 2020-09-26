#include "FICEditor.h"

#include "FICTimeline.h"
#include "FICDetails.h"

void SFICEditor::Construct(const FArguments& InArgs) {
	Context = InArgs._Context;
	GameWidget = InArgs._GameWidget;

	SetRowFill(1, 1);
	SetColumnFill(1, 1);
	//AddSlot(1, 1)[
	//	GameWidget.ToSharedRef()
	//];
	//GameWidget->AssignParentWidget(SharedThis(this));
	/*AddSlot(1, 1)[
		GameSpacer.ToSharedRef()
	];*/
	AddSlot(0, 1)[
		SNew(SFICDetails)
		.Context(Context)
	];
	AddSlot(0, 2).ColumnSpan(3)[
		SNew(SFICTimelinePanel)
		.Context(Context)
	];
}

SFICEditor::SFICEditor() {
	GameSpacer = SNew(STextBlock).Text(FText::FromString("WTF???"));
}

#pragma optimize("", off)
void SFICEditor::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	SGridPanel::OnArrangeChildren(AllottedGeometry, ArrangedChildren);
}

void SFICEditor::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SGridPanel::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (GameSpacer.IsValid()) {
		TSharedPtr<SHorizontalBox> GameViewportContainer = StaticCastSharedPtr<SHorizontalBox>(FSlateApplication::Get().GetGameViewport()->GetParentWidget());

		TPanelChildren<SHorizontalBox::FSlot>& Children = *static_cast<TPanelChildren<SHorizontalBox::FSlot>*>(GameViewportContainer->GetChildren());
		GameSpacer->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(0, 0));
		FVector2D Pos = GameViewportContainer->GetCachedGeometry().AbsoluteToLocal(FVector2D(0,0));
		FVector2D Pos2 = GameViewportContainer->GetCachedGeometry().AbsoluteToLocal(GameSpacer->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(1, 1)));
		FVector2D Size = GameViewportContainer->GetCachedGeometry().GetLocalSize();
		FMargin Padding;
		Padding.Top = Pos.Y;
		Padding.Left = Pos.X;
		Padding.Bottom = Size.Y - Pos2.Y;
		Padding.Right = Size.X - Pos2.X;
	
		//Children[0].Padding(300);
	}
}
#pragma optimize("", on)
