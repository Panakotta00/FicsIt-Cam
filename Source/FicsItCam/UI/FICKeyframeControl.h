#pragma once

#include "FICAnimation.h"
#include "FICEditorAttributeBase.h"
#include "SlateBasics.h"

struct FFICKeyframeControlStyle {
	static FSlateColorBrush DefaultKFBrush;
	static FSlateColor DefaultSetColor;
	static FSlateColor DefaultUnsetColor;
	static FSlateColor DefaultChangedColor;
	static FSlateColor DefaultAnimatedColor;
	
	FSlateBrush* AutoBrush = &DefaultKFBrush;
	FSlateBrush* MirrorBrush = &DefaultKFBrush;
	FSlateBrush* CustomBrush = &DefaultKFBrush;
	FSlateBrush* LinearBrush = &DefaultKFBrush;
	FSlateBrush* StepBrush = &DefaultKFBrush;
	FSlateBrush* EaseInOutBrush = &DefaultKFBrush;
	FSlateBrush* DefaultBrush = &DefaultKFBrush;
	
	FSlateColor UnsetColor = DefaultUnsetColor;
	FSlateColor SetColor = DefaultSetColor;
	FSlateColor ChangedColor = DefaultChangedColor;
	FSlateColor AnimatedColor = DefaultAnimatedColor;
};

class SFICKeyframeControl : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICKeyframeControl) :
		_Style(FFICKeyframeControlStyle()) {}
		SLATE_ATTRIBUTE(FFICEditorAttributeBase*, Attribute)
		SLATE_ATTRIBUTE(TOptional<int64>, Frame)
		SLATE_ATTRIBUTE(FFICKeyframeControlStyle, Style)
	SLATE_END_ARGS()

public:
	void Construct(FArguments InArgs);

private:
	TAttribute<TOptional<int64>> Frame;
	TAttribute<FFICEditorAttributeBase*> Attribute;
	TAttribute<FFICKeyframeControlStyle> Style;

public:
	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	// End SWidget

	int64 GetFrame();
};
