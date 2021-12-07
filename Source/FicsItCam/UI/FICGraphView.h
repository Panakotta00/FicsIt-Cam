#pragma once

#include "FICEditorContext.h"
#include "FICKeyframeControl.h"

DECLARE_DELEGATE_TwoParams(FFICGraphViewTimelineRangeChanged, int64 /* New Min Value */, int64 /* New Max Value */)
DECLARE_DELEGATE_TwoParams(FFICGraphViewValueRangeChanged, float /* New Min Value */, float /* New Max Value */)

class SFICGraphView : public SPanel {
	SLATE_BEGIN_ARGS(SFICGraphView) :
	_AnimationBrush(&DefaultAnimationBrush),
	_AutoFit(true),
	_ValueRangeBegin(-1),
	_ValueRangeEnd(1) {}
		SLATE_ATTRIBUTE(int64, Frame)
		SLATE_ATTRIBUTE(int64, TimelineRangeBegin)
		SLATE_ATTRIBUTE(int64, TimelineRangeEnd)
		SLATE_ATTRIBUTE(float, ValueRangeBegin)
		SLATE_ATTRIBUTE(float, ValueRangeEnd)
		SLATE_ATTRIBUTE(int64, AnimationStart)
		SLATE_ATTRIBUTE(int64, AnimationEnd)
		SLATE_ATTRIBUTE(const FSlateBrush*, AnimationBrush)
		SLATE_ARGUMENT_DEFAULT(bool, AutoFit) = false;
		SLATE_ARGUMENT(TArray<FFICEditorAttributeBase*>, Attributes)
		SLATE_EVENT(FFICGraphViewTimelineRangeChanged, OnTimelineRangedChanged)
		SLATE_EVENT(FFICGraphViewValueRangeChanged, OnValueRangeChanged)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	static FSlateColorBrush DefaultAnimationBrush;
	
	TSlotlessChildren<SFICKeyframeControl> Children;

	TAttribute<int64> ActiveFrame;
	TAttribute<int64> TimelineRangeBegin;
	TAttribute<int64> TimelineRangeEnd;
	TAttribute<float> ValueRangeBegin;
	TAttribute<float> ValueRangeEnd;
	TAttribute<int64> AnimationStart;
	TAttribute<int64> AnimationEnd;
	TAttribute<const FSlateBrush*> AnimationBrush;
	FFICGraphViewTimelineRangeChanged OnTimelineRangeChanged;
	FFICGraphViewValueRangeChanged OnValueRangeChanged;

	TArray<FFICEditorAttributeBase*> Attributes;
	TMap<FFICEditorAttributeBase*, FDelegateHandle> DelegateHandles;

public:
	SFICGraphView();
	virtual ~SFICGraphView() override;
	
	// Begin SWidget
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
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SWidget

	void SetAttributes(const TArray<FFICEditorAttributeBase*>& InAttributes);
	void Update();
	void FitAll();

	void SetValueRange(float InBegin, float InEnd) { OnValueRangeChanged.ExecuteIfBound(InBegin, InEnd); }
	void GetValueRange(float& OutBegin, float& OutEnd) { OutBegin = ValueRangeBegin.IsSet() ? ValueRangeBegin.Get() : 0; OutEnd = ValueRangeEnd.IsSet() ? ValueRangeEnd.Get() : 0; }
	void SetTimeRange(int64 InBegin, int64 InEnd) { OnTimelineRangeChanged.ExecuteIfBound(InBegin, InEnd); }
	void GetTimeRange(int64& OutBegin, int64& OutEnd) { OutBegin = TimelineRangeBegin.IsSet() ? TimelineRangeBegin.Get() : 0; OutEnd = TimelineRangeEnd.IsSet() ? TimelineRangeEnd.Get() : 0; }

	int64 LocalToFrame(float Local) const;
	float LocalToValue(float Local) const;
	float FrameToLocal(int64 InFrame) const;
	float ValueToLocal(float Value) const;
	float GetFramePerLocal() const;
	float GetValuePerLocal() const;
	FVector2D FrameAttributeToLocal(const FFICEditorAttributeBase* InAttribute, int64 InFrame) const;

	TSharedPtr<SFICKeyframeControl> FindKeyframeControl(const FFICEditorAttributeBase* InAttribute, int64 InFrame);
};
