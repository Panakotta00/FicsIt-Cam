#pragma once

#include "FicsItCam/FICEditorAttributeBase.h"

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

class SFICKeyframeControl : public SCompoundWidget {
	static FFICKeyframeControlStyle* DefaultStyle();
	
	SLATE_BEGIN_ARGS(SFICKeyframeControl) :
		_Style(TAttribute<FFICKeyframeControlStyle*>::Create(TFunction<FFICKeyframeControlStyle*()>([](){ return DefaultStyle(); }))) {}
		SLATE_ATTRIBUTE(FFICEditorAttributeBase*, Attribute)
		SLATE_ATTRIBUTE(TOptional<int64>, Frame)
		SLATE_ATTRIBUTE(FFICKeyframeControlStyle*, Style)
	SLATE_END_ARGS()

public:
	void Construct(FArguments InArgs);

private:
	TAttribute<TOptional<int64>> Frame;
	TAttribute<FFICEditorAttributeBase*> Attribute;
	TAttribute<FFICKeyframeControlStyle*> Style;

public:
	// Begin SWidget
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	// End SWidget

	int64 GetFrame();
	FFICEditorAttributeBase* GetAttribute();
};
