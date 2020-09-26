#pragma once

#include "SlateBasics.h"

class UFICEditorContext;

DECLARE_DELEGATE_TwoParams(FFICRangeChanged, float, float)

class SFICRangeSelector : public SLeafWidget {
	SLATE_BEGIN_ARGS(SFICRangeSelector) {}
		SLATE_ARGUMENT(float, RangeStart)
		SLATE_ARGUMENT(float, RangeEnd)
		SLATE_ARGUMENT(float, SelectStart)
		SLATE_ARGUMENT(float, SelectEnd)
		SLATE_ARGUMENT(FFICRangeChanged, SelectStartChanged)
		SLATE_ARGUMENT(FFICRangeChanged, SelectEndChanged)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TAttribute<float> RangeStart;
	TAttribute<float> RangeEnd;
	TAttribute<float> SelectStart;
	TAttribute<float> SelectEnd;
	TAttribute<FFICRangeChanged> SelectStartChanged;
	TAttribute<FFICRangeChanged> SelectEndChanged;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
};

class SFICTimelinePanel : public SVerticalBox {
	SLATE_BEGIN_ARGS(SFICTimelinePanel) {}
		SLATE_ARGUMENT(UFICEditorContext*, Context)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	UFICEditorContext* Context = nullptr;
};
