#include "FICEditor.h"

#include "Engine/World.h"
#include "FICEditorContext.h"
#include "FICTimeline.h"
#include "FICDetails.h"
#include "WidgetBlueprintLibrary.h"

void SFICEditor::Construct(const FArguments& InArgs) {
	Context = InArgs._Context.Get();
	GameWidget = InArgs._GameWidget.Get();

	TSharedPtr<SHorizontalBox> GameViewportContainer = StaticCastSharedPtr<SHorizontalBox>(GameWidget->GetParentWidget());
	
	Children.Add(SNew(SGridPanel)
		.FillColumn(1, 1)
		.FillRow(1, 1)
		+SGridPanel::Slot(1,1)[
			GameViewportContainer.ToSharedRef()
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

FReply SFICEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::LeftAlt && InKeyEvent.GetModifierKeys().IsControlDown()) {
		if (GameWidget->HasUserFocus(InKeyEvent.GetUserIndex())) {
			APlayerController* Controller = Context->GetWorld()->GetFirstPlayerController();
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(Controller);
			FSlateApplication::Get().SetUserFocus(InKeyEvent.GetUserIndex(), SharedThis(this));
		} else {
			APlayerController* Controller = Context->GetWorld()->GetFirstPlayerController();
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(Controller);
			FSlateApplication::Get().SetUserFocusToGameViewport(InKeyEvent.GetUserIndex());
		}
		return FReply::Handled();
	}
	return SPanel::OnKeyDown(MyGeometry, InKeyEvent);
}

bool SFICEditor::SupportsKeyboardFocus() const {
	return true;
}

void SFICEditor::OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) {
	SPanel::OnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);
	if (!PreviousFocusPath.ContainsWidget(GameWidget.ToSharedRef()) && NewWidgetPath.ContainsWidget(GameWidget.ToSharedRef())) {
		APlayerController* Controller = Context->GetWorld()->GetFirstPlayerController();
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(Controller);
	}
}

#pragma optimize("", on)
