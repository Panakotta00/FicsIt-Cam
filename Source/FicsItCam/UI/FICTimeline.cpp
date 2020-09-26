#include "FICTimeline.h"

FSlateColorBrush RangeBackground = FSlateColorBrush(FColor::FromHex("131313"));

void SFICRangeSelector::Construct(const FArguments& InArgs) {
	RangeStart = InArgs._RangeStart;
	RangeEnd = InArgs._RangeEnd;
	SelectStart = InArgs._SelectStart;
	SelectEnd = InArgs._SelectEnd;
	SelectStartChanged = InArgs._SelectStartChanged;
	SelectEndChanged = InArgs._SelectEndChanged;
}

int32 SFICRangeSelector::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), &RangeBackground);

	return LayerId;
}

FVector2D SFICRangeSelector::ComputeDesiredSize(float) const {
	return FVector2D(100, 100);
}

void SFICTimelinePanel::Construct(const FArguments& InArgs) {
	Context = InArgs._Context;

	AddSlot()[
		SNew(SFICRangeSelector)
	];
}
