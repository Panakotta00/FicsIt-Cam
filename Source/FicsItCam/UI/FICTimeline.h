#pragma once

#include "FICRangeSelector.h"
#include "FICTimelineScrubber.h"
#include "SlateBasics.h"

class UFICEditorContext;

class SFICTimelinePanel : public SCompoundWidget {
	static FSlateColorBrush DefaultBackgroundBrush;
	
	SLATE_BEGIN_ARGS(SFICTimelinePanel) :
		_Background(&DefaultBackgroundBrush) {}
		SLATE_ARGUMENT(UFICEditorContext*, Context)
		SLATE_ATTRIBUTE(const FSlateBrush*, Background)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	UFICEditorContext* Context = nullptr;
	TAttribute<const FSlateBrush*> BackgroundBrush;

	TSharedPtr<SFICRangeSelector> VisibleRange;
	TSharedPtr<SFICTimelineScrubber> Scrubber;

	int64 ActiveRangeStart = 0;
	int64 ActiveRangeEnd = 0;

	void ActiveRangeStartChanged(int64 Prev, int64 Cur);
	void ActiveRangeEndChanged(int64 Prev, int64 Cur);

public:
	// Begin SWidget
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End SWidget
};
