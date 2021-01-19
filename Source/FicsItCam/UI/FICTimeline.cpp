#include "FICTimeline.h"

#include "FICEditorContext.h"
#include "FICNumericType.h"
#include "FICRangeSelector.h"
#include "FICTimelineScrubber.h"
#include "SNumericEntryBox.h"

FSlateColorBrush SFICTimelinePanel::DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex("030303"));

void SFICTimelinePanel::Construct(const FArguments& InArgs) {
	Context = InArgs._Context;
	BackgroundBrush = InArgs._Background;

	ActiveRangeStart = Context->GetAnimation()->AnimationStart;
	ActiveRangeEnd = Context->GetAnimation()->AnimationEnd;

	TSharedPtr<INumericTypeInterface<int64>> Interface = MakeShared<TFICNumericTypeInterface<int64>>();

	ChildSlot[SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(BackgroundBrush)
		]
		+SOverlay::Slot()[
			SNew(SGridPanel)
			.FillColumn(1, 1)
			+SGridPanel::Slot(0,0).RowSpan(2).Padding(5)[
				SNew(SGridPanel)
				+SGridPanel::Slot(0,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("Animation Start:"))
	            ]
	            +SGridPanel::Slot(0, 1).Padding(5)[
	                SNew(SBox)
	                .WidthOverride(50)
	                .Content()[
	                SNew(SNumericEntryBox<int64>)
	                    .TypeInterface(Interface)
	                    .Value_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart;
	                    })
	                    .MaxValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationEnd-1;
	                    })
	                    .MinValue(TNumericLimits<int64>::Min())
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        Context->GetAnimation()->AnimationStart = Value;
	                    })
	                ]
	            ]
	            +SGridPanel::Slot(1,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("View Start:"))
	            ]
	            +SGridPanel::Slot(1, 1).Padding(5)[
	                SNew(SBox)
	                .WidthOverride(50)
	                .Content()[
	                SNew(SNumericEntryBox<int64>)
	                    .TypeInterface(Interface)
	                    .MaxValue_Lambda([this]() {
	                        return ActiveRangeEnd-1;
	                    })
	                    .MaxSliderValue_Lambda([this]() {
	                        return ActiveRangeEnd-1;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart;
	                    })
	                    .MinSliderValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart;
	                    })
	                    .Value_Lambda([this]() {
	                        return ActiveRangeStart;
	                    })
	                    .OnValueChanged_Lambda([this](int64 Value) {
	                        ActiveRangeStart = FMath::Clamp(Value, Context->GetAnimation()->AnimationStart, ActiveRangeEnd-1);
	                    })
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        ActiveRangeStart = FMath::Clamp(Value, Context->GetAnimation()->AnimationStart, ActiveRangeEnd-1);
	                    })
	                    .AllowSpin(true)
	                ]
	            ]
	            +SGridPanel::Slot(2,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("Current Frame:"))
	            ]
	            +SGridPanel::Slot(2, 1).Padding(5)[
	                SNew(SBox)
	                .WidthOverride(50)
	                .Content()[
	                SNew(SNumericEntryBox<int64>)
	                    .TypeInterface(Interface)
	                    .MaxValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationEnd;
	                    })
	                    .MaxSliderValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationEnd;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart;
	                    })
	                    .MinSliderValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart;
	                    })
	                    .Value_Lambda([this]() {
	                        return Context->GetCurrentFrame();
	                    })
	                    .OnValueChanged_Lambda([this](int64 Value) {
	                        Context->SetCurrentFrame(Value);
	                    })
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        Context->SetCurrentFrame(Value);
	                    })
	                    .AllowSpin(true)
	                ]
	            ]
	            +SGridPanel::Slot(3,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("View End:"))
	            ]
	            +SGridPanel::Slot(3, 1).Padding(5)[
	                SNew(SBox)
	                .WidthOverride(50)
	                .Content()[
	                SNew(SNumericEntryBox<int64>)
	                    .TypeInterface(Interface)
	                    .MaxValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationEnd;
	                    })
	                    .MaxSliderValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationEnd;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return ActiveRangeStart+1;
	                    })
	                    .MinSliderValue_Lambda([this]() {
	                        return ActiveRangeStart+1;
	                    })
	                    .Value_Lambda([this]() {
	                        return ActiveRangeEnd;
	                    })
	                    .OnValueChanged_Lambda([this](int64 Value) {
	                        ActiveRangeEnd = FMath::Clamp(Value, ActiveRangeStart+1, Context->GetAnimation()->AnimationEnd);
	                    })
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        ActiveRangeEnd = FMath::Clamp(Value, ActiveRangeStart+1, Context->GetAnimation()->AnimationEnd);
	                    })
	                    .AllowSpin(true)
	                ]
	            ]
	            +SGridPanel::Slot(4,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("Animation End:"))
	            ]
	            +SGridPanel::Slot(4, 1).Padding(5)[
	                SNew(SBox)
	                .WidthOverride(50)
	                .Content()[
	                SNew(SNumericEntryBox<int64>)
	                    .TypeInterface(Interface)
	                    .Value_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationEnd;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart+1;
	                    })
	                    .MaxValue(TNumericLimits<int64>::Max())
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        Context->GetAnimation()->AnimationEnd = FMath::Clamp(Value, Context->GetAnimation()->AnimationStart+1, TNumericLimits<int64>::Max());
	                    })
	                ]
	            ]
			]
			+SGridPanel::Slot(1, 0)[
				SAssignNew(VisibleRange, SFICRangeSelector)
				.HighlightEnabled(true)
				.Highlight_Lambda([this]() {
					return Context->GetCurrentFrame();
				})
				.HighlightChanged_Lambda([this](int64 Prev, int64 Cur) {
					Context->SetCurrentFrame(Cur);
				})
				.RangeStart_Lambda([this]() {
					return Context->GetAnimation()->AnimationStart;
				})
				.RangeEnd_Lambda([this]() {
					return Context->GetAnimation()->AnimationEnd;
				})
				.SelectStart_Lambda([this]() {
					return ActiveRangeStart;
				})
				.SelectEnd_Lambda([this]() {
					return ActiveRangeEnd;
				})
				.SelectStartChanged(this, &SFICTimelinePanel::ActiveRangeStartChanged)
				.SelectEndChanged(this, &SFICTimelinePanel::ActiveRangeEndChanged)
			]
			+SGridPanel::Slot(1, 1)[
				SAssignNew(Scrubber, SFICTimelineScrubber)
				.Frame_Lambda([this]() {
					return Context->GetCurrentFrame();
				})
				.FrameChanged_Lambda([this](int64 Prev, int64 Cur) {
					Context->SetCurrentFrame(Cur);
				})
				.RangeStart_Lambda([this]() {
		            return ActiveRangeStart;
		        })
		        .RangeEnd_Lambda([this]() {
		            return ActiveRangeEnd;
		        })
			]
		]
	];
}

void SFICTimelinePanel::ActiveRangeStartChanged(int64 Prev, int64 Cur) {
	ActiveRangeStart = Cur;
}

void SFICTimelinePanel::ActiveRangeEndChanged(int64 Prev, int64 Cur) {
	ActiveRangeEnd = Cur;
}

void SFICTimelinePanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	ActiveRangeStart = FMath::Clamp(ActiveRangeStart, Context->GetAnimation()->AnimationStart, ActiveRangeEnd-1);
	ActiveRangeEnd = FMath::Clamp(ActiveRangeEnd, ActiveRangeStart+1, Context->GetAnimation()->AnimationEnd);
}
