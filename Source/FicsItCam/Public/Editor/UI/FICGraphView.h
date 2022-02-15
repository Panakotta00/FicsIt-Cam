#pragma once

#include "FICKeyframeControl.h"
#include "Brushes/SlateColorBrush.h"
#include "Data/FICTypes.h"
#include "FICEvents.h"

class SFICGraphView : public SPanel {
	SLATE_BEGIN_ARGS(SFICGraphView) :
	_AnimationBrush(&DefaultAnimationBrush),
	_AutoFit(true) {}
		SLATE_ATTRIBUTE(FICFrame, Frame)
		SLATE_ATTRIBUTE(FFICFrameRange, FrameRange)
		SLATE_ATTRIBUTE(FFICFrameRange, FrameHighlightRange)
		SLATE_ATTRIBUTE(FFICValueRange, ValueRange)

		SLATE_ARGUMENT_DEFAULT(bool, AutoFit) = false;
		SLATE_ARGUMENT(TArray<TSharedRef<FFICEditorAttributeBase>>, Attributes)

		SLATE_EVENT(FFICFrameRangeChanged, OnFrameRangeChanged)
		SLATE_EVENT(FFICValueRangeChanged, OnValueRangeChanged)
	
		SLATE_ATTRIBUTE(const FSlateBrush*, AnimationBrush)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* Context);

private:
	static FSlateColorBrush DefaultAnimationBrush;
	
	TSlotlessChildren<SFICKeyframeControl> Children;

	TAttribute<FICFrame> ActiveFrame;
	TAttribute<FFICFrameRange> FrameRange;
	TAttribute<FFICFrameRange> FrameHighlightRange;
	TAttribute<FFICValueRange> ValueRange;
	
	TAttribute<const FSlateBrush*> AnimationBrush;

	FFICFrameRangeChanged OnFrameRangeChanged;
	FFICValueRangeChanged OnValueRangeChanged;

	TArray<TSharedRef<FFICEditorAttributeBase>> Attributes;
	TMap<TSharedRef<FFICEditorAttributeBase>, FDelegateHandle> DelegateHandles;

public:
	UFICEditorContext* Context = nullptr;
	
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

	void SetAttributes(const TArray<TSharedRef<FFICEditorAttributeBase>>& InAttributes);
	void Update();
	void FitAll();

	void SetValueRange(const FFICValueRange& InRange) {
		bool _ = OnValueRangeChanged.ExecuteIfBound(InRange);
	}
	FFICValueRange GetValueRange() {
		return ValueRange.Get();
	}
	void SetFrameRange(const FFICFrameRange& InRange) {
		bool _ = OnFrameRangeChanged.ExecuteIfBound(InRange);
	}
	FFICFrameRange GetFrameRange() {
		return FrameRange.Get();
	}

	FICFrame LocalToFrame(float Local) const;
	FICValue LocalToValue(float Local) const;
	float FrameToLocal(FICFrame InFrame) const;
	float ValueToLocal(FICValue Value) const;
	float GetFramePerLocal() const;
	float GetValuePerLocal() const;
	FVector2D FrameAttributeToLocal(TSharedRef<FFICEditorAttributeBase> InAttribute, FICFrame InFrame) const;

	TSharedPtr<SFICKeyframeControl> FindKeyframeControl(TSharedRef<FFICEditorAttributeBase> InAttribute, FICFrame InFrame);
};
