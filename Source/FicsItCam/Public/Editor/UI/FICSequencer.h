﻿#pragma once

#include "Data/FICTypes.h"
#include "FICSequencerRow.h"
#include "SelectionManager.h"
#include "Editor/Data/FICEditorAttributeBase.h"

class SFICSequencerTreeView;
class SFICSequencerRowAttribute;

DECLARE_DELEGATE_TwoParams(FFICSequencerGenerateRow, TSharedPtr<ITableRow>, TSharedPtr<SFICSequencerRow>&)

class SFICSequencer : public SPanel {
	SLATE_BEGIN_ARGS(SFICSequencer) : _Style(&FFICSequencerStyle::GetDefault()) {}
	SLATE_STYLE_ARGUMENT(FFICSequencerStyle, Style)
	
	SLATE_ATTRIBUTE(FICFrame, Frame)
	SLATE_ATTRIBUTE(FFICFrameRange, FrameRange)
	SLATE_ATTRIBUTE(FFICFrameRange, FrameHighlightRange)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext, SFICSequencerTreeView* InTreeView);

private:
	TSlotlessChildren<SFICSequencerRow> Children;
	TArray<TSharedPtr<ITableRow>> LinearRows;

	const FFICSequencerStyle* Style = nullptr;

	TAttribute<FICFrame> ActiveFrame;
	TAttribute<FFICFrameRange> FrameRange;
	TAttribute<FFICFrameRange> FrameHighlightRange;
	FSelectionManager SelectionManager;

	FDelegateHandle ActiveFrameDelegate;

	TMap<TSharedPtr<FFICSequencerRowMeta>, TSharedPtr<SFICSequencerRow>> MetaToWidget;
	TMap<TSharedPtr<SFICSequencerRow>, TSharedPtr<FFICSequencerRowMeta>> WidgetToMeta;

	FFICFrameRange OldFrameRange;

	FSoftObjectPtr CopiedKeyframesSceneObject;
	TArray<TTuple<FICFrame, FFICAttribute*, FFICKeyframeData>> CopiedKeyframes;
	
public:
	UFICEditorContext* Context = nullptr;
	SFICSequencerTreeView* TreeView = nullptr;
	
	SFICSequencer();
	virtual ~SFICSequencer() override;
	
	// Begin SWidget
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SWidget

	FSelectionManager& GetSelectionManager();
	const FSelectionManager& GetSelectionManager() const;

	void UpdateRows();

	int32 GetRowIndexByWidget(TSharedRef<SFICSequencerRow> InWidget);

	FICFrame LocalToFrame(float Local) const;
	double LocalToFrameF(float Local) const;
	float FrameToLocal(FICFrame InFrame) const;
	float GetFramePerLocal() const;

	void FrameRangeChanged();
	void ActiveFrameChanged();
};
