#pragma once
#include "FICEditorStyle.h"
#include "HorizontalBox.h"
#include "MultiBoxBuilder.h"
#include "TabManager.h"

class UFICEditorContext;

class SFICEditor : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICEditor) :
		_Style(&FFICEditorStyles::Get().GetWidgetStyle<FFICEditorStyle>(TEXT("EditorStyle"))) {}
		SLATE_STYLE_ARGUMENT(FFICEditorStyle, Style)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* Context, TSharedPtr<SWidget> GameWidget, TSharedPtr<SViewport> Viewport);
	
private:
	UFICEditorContext* Context = nullptr;
	TSharedPtr<SWidget> GameWidget;
	TSharedPtr<SWidget> GameSpacer;
	TSharedPtr<SViewport> GameViewport;
	SHorizontalBox::FSlot* GameSlot = nullptr;

	const FFICEditorStyle* Style = nullptr;

	SVerticalBox::FSlot* EditorSlot = nullptr;
	TSharedPtr<FTabManager> TabManager;
	TSharedPtr<FTabManager::FLayout> DefaultLayout;

	bool bIsLeft = false;
	bool bIsRight = false;
	float KeyPressTime = 0;

	void RegisterTabs();
	FMenuBarBuilder CreateMenuBar();
	void LoadLayout(TSharedPtr<FTabManager::FLayout> Layout);

public:
	// Begin SWidget
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual void OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;
	// End SWidget
};
