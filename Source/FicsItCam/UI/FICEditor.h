#pragma once

#include "SlateBasics.h"

class UFICEditorContext;

class SFICEditor : public SGridPanel {
	SLATE_BEGIN_ARGS(SFICEditor) {}
		SLATE_ARGUMENT(UFICEditorContext*, Context);
		SLATE_ARGUMENT(TSharedPtr<SWidget>, GameWidget);
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	
private:
	UFICEditorContext* Context = nullptr;
	TSharedPtr<SWidget> GameWidget;

	TSharedPtr<SWidget> GameSpacer;

public:
	SFICEditor();
	
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
};
