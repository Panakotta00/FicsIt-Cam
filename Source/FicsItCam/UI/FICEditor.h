#pragma once

#include "SlateBasics.h"

class UFICEditorContext;

class SFICEditor : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICEditor) {}
		SLATE_ATTRIBUTE(UFICEditorContext*, Context);
		SLATE_ATTRIBUTE(TSharedPtr<SWidget>, GameWidget);
		SLATE_ARGUMENT(TSharedPtr<SViewport>, Viewport)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	
private:
	static FSlateColorBrush Background;
	
	UFICEditorContext* Context = nullptr;
	TSharedPtr<SWidget> GameWidget;
	TSharedPtr<SWidget> GameSpacer;
	TSharedPtr<SViewport> GameViewport;
	SHorizontalBox::FSlot* GameSlot = nullptr;

	bool bIsLeft = false;
	bool bIsRight = false;
	float KeyPressTime = 0;

public:
	// Begin SWidget
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual void OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;
	virtual FReply OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	// End SWidget
};
