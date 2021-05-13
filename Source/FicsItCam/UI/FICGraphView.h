#pragma once
#include "FICEditorContext.h"
#include "FICKeyframeControl.h"

DECLARE_DELEGATE_TwoParams(FFICGraphViewTimelineRangeChanged, int64 /* New Min Value */, int64 /* New Max Value */)
DECLARE_DELEGATE_TwoParams(FFICGraphViewValueRangeChanged, float /* New Min Value */, float /* New Max Value */)

class SFICGraphView : public SPanel {
	SLATE_BEGIN_ARGS(SFICGraphView) {}
		SLATE_ATTRIBUTE(int64, Frame)
		SLATE_ATTRIBUTE(int64, TimelineRangeBegin)
		SLATE_ATTRIBUTE(int64, TimelineRangeEnd)
		SLATE_ATTRIBUTE(float, ValueRangeBegin)
		SLATE_ATTRIBUTE(float, ValueRangeEnd)
		SLATE_ATTRIBUTE(bool, AutoFit)
		SLATE_EVENT(FFICGraphViewTimelineRangeChanged, OnTimelineRangedChanged)
		SLATE_EVENT(FFICGraphViewValueRangeChanged, OnValueRangeChanged)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TSlotlessChildren<SFICKeyframeControl> Children;

	TAttribute<int64> ActiveFrame;
	TAttribute<int64> TimelineRangeBegin;
	TAttribute<int64> TimelineRangeEnd;
	TAttribute<float> ValueRangeBegin;
	TAttribute<float> ValueRangeEnd;
	TAttribute<bool> AutoFit;
	FFICGraphViewTimelineRangeChanged OnTimelineRangeChanged;
	FFICGraphViewValueRangeChanged OnValueRangeChanged;

	TArray<FFICEditorAttributeBase*> Attributes;

	FVector2D StartLocal;
	FVector2D CurrentLocal;
	int64 DragStartFrame = 0;
	float DragStartValue = 0.0f;
	int64 DragCurrentFrame = 0;
	float DragCurrentValue = 0.0f;
	int64 DragViewStartFrameBegin, DragViewStartFrameEnd;
	float DragViewStartValueBegin, DragViewStartValueEnd;
	FFICEditorAttributeBase* DraggingAttribute = nullptr;
	bool bIsDraggingKeyframe = false;
	bool bIsDraggingView = false;
	bool bIsDragging = false;

public:
	SFICGraphView();
	
	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool IsInteractable() const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SWidget

	void SetAttributes(const TArray<FFICEditorAttributeBase*>& InAttributes);
	void Update();
	void FitAll();

	int64 LocalToFrame(float Local) const;
	float LocalToValue(float Local) const;
	float FrameToLocal(int64 InFrame) const;
	float ValueToLocal(float Value) const;
	FVector2D FrameAttributeToLocal(const FFICEditorAttributeBase* InAttribute, int64 InFrame) const;
};
