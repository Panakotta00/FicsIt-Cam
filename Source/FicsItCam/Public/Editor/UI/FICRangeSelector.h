﻿#pragma once

#include "FICEditorStyle.h"
#include "FICEvents.h"
#include "SLeafWidget.h"
#include "Data/FICTypes.h"

class SFICRangeSelector : public SLeafWidget {
	SLATE_BEGIN_ARGS(SFICRangeSelector) :
		_Style(&FFICEditorStyles::Get().GetWidgetStyle<FFICRangeSelectorStyle>("RangeSelector")),
		_ActiveFrameEnabled(false) {
			Clipping(EWidgetClipping::ClipToBoundsAlways);
		}
		SLATE_STYLE_ARGUMENT(FFICRangeSelectorStyle, Style)
		SLATE_ATTRIBUTE(FFICFrameRange, FullRange)
		SLATE_ATTRIBUTE(FFICFrameRange, ActiveRange)
		SLATE_ATTRIBUTE(FICFrame, ActiveFrame)
		SLATE_ATTRIBUTE(bool, ActiveFrameEnabled)
	
		SLATE_EVENT(FFICFrameRangeChanged, OnActiveRangeChanged)
		SLATE_EVENT(FFICFrameChanged, OnActiveFrameChanged)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	const FFICRangeSelectorStyle* Style = nullptr;
	TAttribute<FFICFrameRange> FullRange;
	TAttribute<FFICFrameRange> ActiveRange;
	TAttribute<FICFrame> ActiveFrame;
	TAttribute<bool> ActiveFrameEnabled;
	
	FFICFrameRangeChanged OnActiveRangeChanged;
	FFICFrameChanged OnActiveFrameChanged;
	
	FFICFrameRange PrevActiveRange;

public:
	// Begin SWidget
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	virtual bool IsInteractable() const override;
	virtual TSharedPtr<IToolTip> GetToolTip() override;
	virtual void OnToolTipClosing() override;
	// End SWidget

	float FrameToLocalPos(FICFrame Frame) const;
	FICFrame LocalPosToFrame(float LocalPos) const;
	bool IsLocalPosNearFrame(float LocalPos, FICFrame Frame) const;

	FFICFrameRange GetActiveRange() const { return ActiveRange.Get(); }
	void SetActiveRange(const FFICFrameRange& InRange, bool bStoreAsPrev = true) {
		bool _ = OnActiveRangeChanged.ExecuteIfBound(InRange);
		if (bStoreAsPrev) PrevActiveRange = InRange;
	}

	FICFrame GetActiveFrame() const { return ActiveFrame.Get(); }
	void SetActiveFrame(FICFrame Frame) {
		bool _ = OnActiveFrameChanged.ExecuteIfBound(Frame);
	}
};

enum EFICRangeSelectorDragDropType {
	FIC_RangeSelect_None,
	FIC_RangeSelect_Begin,
	FIC_RangeSelect_End,
	FIC_RangeSelect_Both,
	FIC_RangeSelect_Frame,
};

class FFICRangeSelectorDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICRangeSelectorDragDrop, FDragDropOperation)

	TSharedRef<SFICRangeSelector> RangeSelector;

	FFICFrameRange InitRange;
	FICFrame InitFrame;
	EFICRangeSelectorDragDropType ChangeType;
	
	FFICRangeSelectorDragDrop(FPointerEvent InitEvent, TSharedRef<SFICRangeSelector> InRangeSelector, EFICRangeSelectorDragDropType InType) : RangeSelector(InRangeSelector), ChangeType(InType) {
		InitFrame = RangeSelector->LocalPosToFrame(RangeSelector->GetCachedGeometry().AbsoluteToLocal(InitEvent.GetScreenSpacePosition()).X);
		InitRange = RangeSelector->GetActiveRange();
	}
	
	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual FCursorReply OnCursorQuery() override { return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight); }
	// End FDragDropOperation
};