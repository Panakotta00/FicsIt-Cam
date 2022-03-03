#pragma once

#include "FICKeyframeControl.h"
#include "Brushes/SlateColorBrush.h"

DECLARE_DELEGATE_TwoParams(FFICKeyframeViewTimelineRangeChanged, int64 /* New Min Value */, int64 /* New Max Value */)

class SFICKeyframesView : public SPanel {
	SLATE_BEGIN_ARGS(SFICKeyframesView) :
	_AnimationBrush(&DefaultAnimationBrush) {}
	SLATE_ATTRIBUTE(int64, ActiveFrame)
	SLATE_ATTRIBUTE(int64, ActiveRange)
	SLATE_ATTRIBUTE(int64, ActiveRangeEnd)
	SLATE_ATTRIBUTE(int64, AnimationStart)
	SLATE_ATTRIBUTE(int64, AnimationEnd)
	SLATE_ATTRIBUTE(const FSlateBrush*, AnimationBrush)
	SLATE_ARGUMENT(TArray<FFICEditorAttributeBase*>, Attributes)
	SLATE_EVENT(FFICKeyframeViewTimelineRangeChanged, OnTimelineRangedChanged)
	SLATE_END_ARGS()
public:
	static FSlateColorBrush DefaultAnimationBrush;

	TSlotlessChildren<SFICKeyframeControl> Children;

	TAttribute<int64> ActiveFrame;
	TAttribute<int64> TimelineRangeBegin;
	TAttribute<int64> TimelineRangeEnd;
	TAttribute<int64> AnimationStart;
	TAttribute<int64> AnimationEnd;
	TAttribute<const FSlateBrush*> AnimationBrush;
	FFICKeyframeViewTimelineRangeChanged OnTimelineRangeChanged;

	TArray<FFICEditorAttributeBase*> Attributes;
	TMap<FFICEditorAttributeBase*, FDelegateHandle> DelegateHandles;
	
	// Begin SFICKeyframesView
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual bool IsInteractable() const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SFICKeyframesView
};
