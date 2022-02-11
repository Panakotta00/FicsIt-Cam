#pragma once

#include "SlateBasics.h"

DECLARE_DELEGATE_TwoParams(FFICRangeChanged, int64 /* Previous Value */, int64 /* New Value */)

class SFICRangeSelector : public SLeafWidget {
	static FSlateColorBrush DefaultBackgroundBrush;
	static FLinearColor DefaultRangeIncrementColor;
	static FSlateColorBrush DefaultSelectBrush;
	static FLinearColor DefaultSelectHandleColor;
	static FLinearColor DefaultHighlightColor;
	
	SLATE_BEGIN_ARGS(SFICRangeSelector) :
		_RangeStart(0),
		_RangeEnd(100),
		_SelectStart(25),
		_SelectEnd(75),
		_Highlight(50),
		_HighlightEnabled(false),
		_BackgroundBrush(&DefaultBackgroundBrush),
		_RangeIncrementColor(DefaultRangeIncrementColor),
		_SelectBrush(&DefaultSelectBrush),
		_SelectHandleColor(DefaultSelectHandleColor),
		_HighlightColor(DefaultHighlightColor) {
			Clipping(EWidgetClipping::ClipToBoundsAlways);
		}
		SLATE_ATTRIBUTE(int64, RangeStart)
		SLATE_ATTRIBUTE(int64, RangeEnd)
		SLATE_ATTRIBUTE(int64, SelectStart)
		SLATE_ATTRIBUTE(int64, SelectEnd)
		SLATE_ATTRIBUTE(int64, Highlight)
		SLATE_ATTRIBUTE(bool, HighlightEnabled)
		SLATE_EVENT(FFICRangeChanged, SelectStartChanged)
		SLATE_EVENT(FFICRangeChanged, SelectEndChanged)
		SLATE_EVENT(FFICRangeChanged, HighlightChanged)
		SLATE_ATTRIBUTE(const FSlateBrush*, BackgroundBrush)
		SLATE_ATTRIBUTE(FLinearColor, RangeIncrementColor)
		SLATE_ATTRIBUTE(const FSlateBrush*, SelectBrush)
		SLATE_ATTRIBUTE(FLinearColor, SelectHandleColor)
		SLATE_ATTRIBUTE(FLinearColor, HighlightColor)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TAttribute<int64> RangeStart;
	TAttribute<int64> RangeEnd;
	TAttribute<int64> SelectStartAttr;
	TAttribute<int64> SelectEndAttr;
	TAttribute<int64> HighlightAttr;
	TAttribute<bool> HighlightEnabled;
	FFICRangeChanged SelectStartChanged;
	FFICRangeChanged SelectEndChanged;
	FFICRangeChanged HighlightChanged;
	TAttribute<const FSlateBrush*> BackgroundBrush;
	TAttribute<FLinearColor> RangeIncrementColor;
	TAttribute<const FSlateBrush*> SelectBrush;
	TAttribute<FLinearColor> SelectHandleColor;
	TAttribute<FLinearColor> HighlightColor;

	int64 SelectStart = 0;
	int64 SelectEnd = 0;
	int64 PrevSelectStart = 0;
	int64 PrevSelectEnd = 0;
	int64 Highlight = 0;

	bool bStartDrag = false;
	bool bEndDrag = false;
	bool bSelectDrag = false;
	bool bHighlightDrag = false;
	int64 SelectDragStart  = 0;
	int64 SelectDragEnd = 0;
	int64 DragStartPos = 0;
	int64 LastDragPos = 0;
	int64 DragDelta = 0;

public:
	// Begin SWidget
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool IsInteractable() const override;
	virtual TSharedPtr<IToolTip> GetToolTip() override;
	virtual void OnToolTipClosing() override;
	// End SWidget

	float RangePosToLocalPos(const FGeometry& MyGeometry, int64 RangePos) const;
	int64 LocalPosToRangePos(const FGeometry& MyGeometry, float LocalPos) const;
	bool IsLocalPosNearRangePos(const FGeometry& MyGeometry, float LocalPos, int64 RangePos) const;
	
	void SetSelectStart(int64 Start);
	void SetSelectEnd(int64 End);
	void SetHighlight(int64 Highlight);
	int64 GetSelectStart() const;
	int64 GetSelectEnd() const;
	int64 GetHighlight() const;
};