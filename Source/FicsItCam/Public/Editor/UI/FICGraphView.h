#pragma once

#include "FICEditorStyle.h"
#include "Data/FICTypes.h"
#include "FICEvents.h"
#include "SelectionManager.h"
#include "Data/Attributes/FICKeyframe.h"
#include "Editor/Data/FICEditorAttributeBase.h"

class UFICEditorContext;
struct FFICAttribute;
class SFICGraphView;

class SFICGraphViewKeyframeHandle : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICGraphViewKeyframeHandle) : _Style(&FFICGraphViewStyle::GetDefault()), _IsOutHandle(false) {}
	SLATE_STYLE_ARGUMENT(FFICGraphViewStyle, Style)
	SLATE_ARGUMENT(bool, IsOutHandle)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, class SFICGraphViewKeyframe* InKeyframe);

private:
	class SFICGraphViewKeyframe* GraphKeyframe = nullptr;
	bool bIsOutHandle = false;
	const FFICGraphViewStyle* Style = nullptr;

public:
	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	// End SWidget

	SFICGraphViewKeyframe* GetGraphKeyframe() { return GraphKeyframe; }
	bool IsOutHandle() { return bIsOutHandle; }
};

class SFICGraphViewKeyframe : public SPanel {
	SLATE_BEGIN_ARGS(SFICGraphViewKeyframe) : _Style(&FFICGraphViewStyle::GetDefault()) {}
	SLATE_STYLE_ARGUMENT(FFICGraphViewStyle, Style)
	SLATE_END_ARGS()

public:
	SFICGraphViewKeyframe();
	
	void Construct(const FArguments& InArgs, SFICGraphView* InGraphView, FFICAttribute* InAttribute, FICFrame InFrame);

private:
	SFICGraphView* GraphView = nullptr;
	FFICAttribute* Attribute = nullptr;
	FICFrame Frame = 0;
	const FFICGraphViewStyle* Style = nullptr;

	TSlotlessChildren<SWidget> Children;
	TSharedPtr<SBox> MainHandle;
	TSharedPtr<SFICGraphViewKeyframeHandle> InHandle;
	TSharedPtr<SFICGraphViewKeyframeHandle> OutHandle;

public:
	// Begin SWidget
	virtual int OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget

	SFICGraphView* GetGraphView() { return GraphView; }
	UFICEditorContext* GetContext();
	FICFrame GetFrame() const { return Frame; }
	FFICAttribute& GetAttribute() { return *Attribute; }
	TSharedPtr<FFICKeyframe> GetKeyframe() const;
};

class SFICGraphView : public SPanel {
	SLATE_BEGIN_ARGS(SFICGraphView) : _Style(&FFICGraphViewStyle::GetDefault()) {}
	SLATE_STYLE_ARGUMENT(FFICGraphViewStyle, Style)
	SLATE_ATTRIBUTE(FICFrame, Frame)
	SLATE_ATTRIBUTE(FFICFrameRange, FrameRange)
	SLATE_ATTRIBUTE(FFICFrameRange, FrameHighlightRange)
	SLATE_ATTRIBUTE(FFICValueRange, ValueRange)

	SLATE_ARGUMENT_DEFAULT(bool, AutoFit) = true;
	SLATE_ARGUMENT(TArray<TSharedRef<FFICEditorAttributeBase>>, Attributes)

	SLATE_EVENT(FFICFrameRangeChanged, OnFrameRangeChanged)
	SLATE_EVENT(FFICValueRangeChanged, OnValueRangeChanged)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* Context);

private:
	TSlotlessChildren<SFICGraphViewKeyframe> Children;

	const FFICGraphViewStyle* Style = nullptr;

	TAttribute<FICFrame> ActiveFrame;
	TAttribute<FFICFrameRange> FrameRange;
	TAttribute<FFICFrameRange> FrameHighlightRange;
	TAttribute<FFICValueRange> ValueRange;
	
	FFICFrameRangeChanged OnFrameRangeChanged;
	FFICValueRangeChanged OnValueRangeChanged;

	TArray<TSharedRef<FFICEditorAttributeBase>> Attributes;
	TMap<TSharedRef<FFICEditorAttributeBase>, FDelegateHandle> DelegateHandles;

	FSelectionManager SelectionManager;

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
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SWidget

	FSelectionManager& GetSelectionManager();
	const FSelectionManager& GetSelectionManager() const;

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

	TSharedPtr<SFICGraphViewKeyframe> FindKeyframeControl(TSharedRef<FFICEditorAttributeBase> InAttribute, FICFrame InFrame);
	TSharedPtr<SFICGraphViewKeyframe> FindKeyframeControl(FFICAttribute* InAttribute, FICFrame InFrame);
};
