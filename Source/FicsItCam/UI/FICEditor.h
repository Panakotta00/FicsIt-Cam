#pragma once

#include "SlateBasics.h"

class UFICEditorContext;

class SFICEditor : public SPanel {
	SLATE_BEGIN_ARGS(SFICEditor) {}
		SLATE_ATTRIBUTE(UFICEditorContext*, Context);
		SLATE_ATTRIBUTE(TSharedPtr<SWidget>, GameWidget);
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	
private:
	TSlotlessChildren<SGridPanel> Children;

	UFICEditorContext* Context = nullptr;
	TSharedPtr<SWidget> GameWidget;
	TSharedPtr<SWidget> GameSpacer;
	SHorizontalBox::FSlot* GameSlot = nullptr;

public:
	SFICEditor();

	// Begin SWidget
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual void OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;
	// End SWidget
};
