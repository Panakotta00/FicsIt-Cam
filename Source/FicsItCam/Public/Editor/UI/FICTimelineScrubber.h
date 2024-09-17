#pragma once

#include "Data/FICTypes.h"
#include "FICEvents.h"

class SFICTimelineScrubber : public SLeafWidget {
	static FSlateColorBrush DefaultBackgroundBrush;
	static FSlateColorBrush DefaultAnimationBrush;
	static FLinearColor DefaultIncrementColor;
	static FLinearColor DefaultFrameColor;
	
	SLATE_BEGIN_ARGS(SFICTimelineScrubber) :
		_BackgroundBrush(&DefaultBackgroundBrush),
	    _IncrementColor(DefaultIncrementColor),
		_FrameColor(DefaultFrameColor),
		_AnimationBrush(&DefaultAnimationBrush) {
			Clipping(EWidgetClipping::ClipToBoundsAlways);
		}
		SLATE_ATTRIBUTE(FFICFrameRange, FullRange)
	    SLATE_ATTRIBUTE(FFICFrameRange, ActiveRange)
		SLATE_ATTRIBUTE(FICFrame, ActiveFrame)
	
		SLATE_EVENT(FFICFrameRangeChanged, OnActiveRangeChanged)
		SLATE_EVENT(FFICFrameChanged, OnActiveFrameChanged)
		
		SLATE_ATTRIBUTE(const FSlateBrush*, BackgroundBrush)
        SLATE_ATTRIBUTE(FLinearColor, IncrementColor)
        SLATE_ATTRIBUTE(FLinearColor, FrameColor)
		SLATE_ATTRIBUTE(const FSlateBrush*, AnimationBrush)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TAttribute<FFICFrameRange> FullRange;
	TAttribute<FFICFrameRange> ActiveRange;
	TAttribute<FICFrame> ActiveFrame;

	FFICFrameRangeChanged OnActiveRangeChanged;
	FFICFrameChanged OnActiveFrameChanged;
	
	TAttribute<const FSlateBrush*> BackgroundBrush;
	TAttribute<FLinearColor> IncrementColor;
	TAttribute<FLinearColor> FrameColor;
	TAttribute<const FSlateBrush*> AnimationBrush;
	
public:
	// Begin SWidget
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual bool IsInteractable() const override;
	virtual TSharedPtr<IToolTip> GetToolTip() override;
	virtual void OnToolTipClosing() override;
	// End SWidget
	
	float FrameToLocalPos(FICFrame Frame) const;
	FICFrame LocalPosToFrame(float LocalPos) const;

	void SetActiveFrame(FICFrame Frame);
	FICFrame GetActiveFrame() const;
};

class FFICTimelineScrubberDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICTimelineScrubberDragDrop, FDragDropOperation)

	TSharedRef<SFICTimelineScrubber> TimelineScrubber;

	FICFrame InitFrame;
	
	FFICTimelineScrubberDragDrop(FPointerEvent InitEvent, TSharedRef<SFICTimelineScrubber> InRangeSelector) : TimelineScrubber(InRangeSelector) {
		InitFrame = TimelineScrubber->LocalPosToFrame(TimelineScrubber->GetCachedGeometry().AbsoluteToLocal(InitEvent.GetScreenSpacePosition()).X);
	}
	
	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual FCursorReply OnCursorQuery() override { return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight); }
	// End FDragDropOperation
};