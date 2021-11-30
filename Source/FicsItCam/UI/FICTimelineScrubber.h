#pragma once

#include "SlateBasics.h"

DECLARE_DELEGATE_TwoParams(FFICTimelineScrubberFrameChanged, int64 /* Previous Value */, int64 /* New Value */)

class SFICTimelineScrubber : public SLeafWidget {
	static FSlateColorBrush DefaultBackgroundBrush;
	static FSlateColorBrush DefaultAnimationBrush;
	static FLinearColor DefaultIncrementColor;
	static FLinearColor DefaultFrameColor;
	
	SLATE_BEGIN_ARGS(SFICTimelineScrubber) :
		_RangeStart(0),
		_RangeEnd(100),
		_Frame(50),
		_BackgroundBrush(&DefaultBackgroundBrush),
		_AnimationBrush(&DefaultAnimationBrush),
	    _IncrementColor(DefaultIncrementColor),
		_FrameColor(DefaultFrameColor) {
			Clipping(EWidgetClipping::ClipToBoundsAlways);
		}
		SLATE_ATTRIBUTE(int64, AnimationStart)
		SLATE_ATTRIBUTE(int64, AnimationEnd)
		SLATE_ATTRIBUTE(int64, RangeStart)
	    SLATE_ATTRIBUTE(int64, RangeEnd)
		SLATE_ATTRIBUTE(int64, Frame)
		SLATE_ATTRIBUTE(const FSlateBrush*, BackgroundBrush)
        SLATE_ATTRIBUTE(FLinearColor, IncrementColor)
        SLATE_ATTRIBUTE(FLinearColor, FrameColor)
		SLATE_ATTRIBUTE(const FSlateBrush*, AnimationBrush)
		SLATE_EVENT(FFICTimelineScrubberFrameChanged, FrameChanged)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TAttribute<int64> AnimationStart;
	TAttribute<int64> AnimationEnd;
	TAttribute<int64> RangeStart;
	TAttribute<int64> RangeEnd;
	TAttribute<int64> FrameAttr;
	TAttribute<const FSlateBrush*> BackgroundBrush;
	TAttribute<FLinearColor> IncrementColor;
	TAttribute<FLinearColor> FrameColor;
	TAttribute<const FSlateBrush*> AnimationBrush;
	FFICTimelineScrubberFrameChanged FrameChanged;

	int64 Frame = 0;
	bool bDrag = false;
	
public:
	// Begin SWidget
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual bool IsInteractable() const override;
	virtual TSharedPtr<IToolTip> GetToolTip() override;
	virtual void OnToolTipClosing() override;
	// End SWidget
	
	float RangePosToLocalPos(const FGeometry& MyGeometry, int64 RangePos) const;
	int64 LocalPosToRangePos(const FGeometry& MyGeometry, float LocalPos) const;

	void SetFrame(int64 Frame);
	int64 GetFrame() const;
};