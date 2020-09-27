#include "FICTimeline.h"

#include "FICEditorContext.h"
#include "FICRangeSelector.h"
#include "FICTimelineScrubber.h"

void SFICTimelinePanel::Construct(const FArguments& InArgs) {
	Context = InArgs._Context;

	ActiveRangeStart = Context->Animation->AnimationStart;
	ActiveRangeEnd = Context->Animation->AnimationEnd;
	
	AddSlot()[
		SAssignNew(VisibleRange, SFICRangeSelector)
		.HighlightEnabled(true)
		.Highlight_Lambda([this]() {
			return Context->CurrentFrame;
		})
		.RangeStart_Lambda([this]() {
			return Context->Animation->AnimationStart;
		})
		.RangeEnd_Lambda([this]() {
			return Context->Animation->AnimationEnd;
		})
		.SelectStart_Lambda([this]() {
			return ActiveRangeStart;
		})
		.SelectEnd_Lambda([this]() {
			return ActiveRangeEnd;
		})
		.SelectStartChanged(this, &SFICTimelinePanel::ActiveRangeStartChanged)
		.SelectEndChanged(this, &SFICTimelinePanel::ActiveRangeEndChanged)
	];
	AddSlot()[
		SAssignNew(Scrubber, SFICTimelineScrubber)
		.Frame_Lambda([this]() {
			return Context->CurrentFrame;
		})
		.FrameChanged_Lambda([this](int64 Prev, int64 Cur) {
			Context->CurrentFrame = Cur;
		})
		.RangeStart_Lambda([this]() {
            return ActiveRangeStart;
        })
        .RangeEnd_Lambda([this]() {
            return ActiveRangeEnd;
        })
	];
}

void SFICTimelinePanel::ActiveRangeStartChanged(int64 Prev, int64 Cur) {
	ActiveRangeStart = Cur;
}

void SFICTimelinePanel::ActiveRangeEndChanged(int64 Prev, int64 Cur) {
	ActiveRangeEnd = Cur;
}
