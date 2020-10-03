#pragma once

#include "SlateBasics.h"

class UFICEditorContext;

class SFICDetails : public SCompoundWidget {
	static FSlateColorBrush DefaultBackgroundBrush;
	
	SLATE_BEGIN_ARGS(SFICDetails) :
		_Background(&DefaultBackgroundBrush) {}
		SLATE_ARGUMENT(UFICEditorContext*, Context)
		SLATE_ATTRIBUTE(const FSlateBrush*, Background)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	UFICEditorContext* Context = nullptr;
	TAttribute<const FSlateBrush*> BackgroundBrush;
};
