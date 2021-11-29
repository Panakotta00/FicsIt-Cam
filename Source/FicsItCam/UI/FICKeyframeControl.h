#pragma once

#include "FicsItCam/FICEditorAttributeBase.h"

class SFICGraphView;

struct FFICKeyframeControlStyle {
	FSlateBrush AutoBrush;
	FSlateBrush MirrorBrush;
	FSlateBrush CustomBrush;
	FSlateBrush LinearBrush;
	FSlateBrush StepBrush;
	FSlateBrush EaseInOutBrush;
	FSlateBrush DefaultBrush;
	
	FSlateColor UnsetColor;
	FSlateColor SetColor;
	FSlateColor ChangedColor;
	FSlateColor AnimatedColor;

	FFICKeyframeControlStyle();
};

class SFICKeyframeHandle : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICKeyframeHandle) : _IsOutHandle(false) {}
	SLATE_ARGUMENT(bool, IsOutHandle)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, class SFICKeyframeControl* InKeyframeControl);

	class SFICKeyframeControl* KeyframeControl;
	bool bIsOutHandle;

	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	// End SWidget
};

class SFICKeyframeControl : public SPanel {
	static FFICKeyframeControlStyle* DefaultStyle();
	
	SLATE_BEGIN_ARGS(SFICKeyframeControl) :
		_Style(TAttribute<FFICKeyframeControlStyle*>::Create(TFunction<FFICKeyframeControlStyle*()>([](){ return DefaultStyle(); }))),
		_GraphView(nullptr),
		_ShowHandles(false) {}
		SLATE_ATTRIBUTE(FFICEditorAttributeBase*, Attribute)
		SLATE_ATTRIBUTE(TOptional<int64>, Frame)
		SLATE_ATTRIBUTE(FFICKeyframeControlStyle*, Style)
		SLATE_ARGUMENT(SFICGraphView*, GraphView)
		SLATE_ARGUMENT(bool, ShowHandles)
	SLATE_END_ARGS()

public:
	SFICKeyframeControl();
	
	void Construct(FArguments InArgs);

private:
	TAttribute<TOptional<int64>> Frame;
	TAttribute<FFICEditorAttributeBase*> Attribute;
	TAttribute<FFICKeyframeControlStyle*> Style;

	TSlotlessChildren<SWidget> Children;
	TSharedPtr<SBox> MainHandle;
	TSharedPtr<SFICKeyframeHandle> FromHandle;
	TSharedPtr<SFICKeyframeHandle> ToHandle;

public:
	SFICGraphView* GraphView = nullptr;
	
	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget

	int64 GetFrame();
	FFICEditorAttributeBase* GetAttribute() const;
};
