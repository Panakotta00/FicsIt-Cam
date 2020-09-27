#pragma once

#include "FICRangeSelector.h"
#include "FICTimelineScrubber.h"
#include "SlateBasics.h"

class UFICEditorContext;

class SFICTimelinePanel : public SVerticalBox {
	SLATE_BEGIN_ARGS(SFICTimelinePanel) {}
		SLATE_ARGUMENT(UFICEditorContext*, Context)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	UFICEditorContext* Context = nullptr;

	TSharedPtr<SFICRangeSelector> VisibleRange;
	TSharedPtr<SFICTimelineScrubber> Scrubber;

	int64 ActiveRangeStart = 0;
	int64 ActiveRangeEnd = 0;
	
	void ActiveRangeStartChanged(int64 Prev, int64 Cur);
	void ActiveRangeEndChanged(int64 Prev, int64 Cur);
};
