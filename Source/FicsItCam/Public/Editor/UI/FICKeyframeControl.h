#pragma once

#include "Data/FICTypes.h"

class FFICEditorAttributeBase;
class UFICEditorContext;
class SFICGraphView;

struct FFICKeyframeControlStyle : public FSlateWidgetStyle {
public:
	static const FFICKeyframeControlStyle& GetDefault();
	
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		OutBrushes.Add(&AutoBrush);
		OutBrushes.Add(&MirrorBrush);
		OutBrushes.Add(&CustomBrush);
		OutBrushes.Add(&LinearBrush);
		OutBrushes.Add(&StepBrush);
		OutBrushes.Add(&EaseInOutBrush);
		OutBrushes.Add(&DefaultBrush);
		OutBrushes.Add(&HandleBrush);
	}
	
	FSlateBrush AutoBrush;
	FSlateBrush MirrorBrush;
	FSlateBrush CustomBrush;
	FSlateBrush LinearBrush;
	FSlateBrush StepBrush;
	FSlateBrush EaseInOutBrush;
	FSlateBrush DefaultBrush;
	
	FSlateBrush HandleBrush;
	
	FSlateColor UnsetColor;
	FSlateColor SetColor;
	FSlateColor ChangedColor;
	FSlateColor AnimatedColor;

	FFICKeyframeControlStyle();
};

class SFICKeyframeControl : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SFICKeyframeControl) : _Style(&FFICKeyframeControlStyle::GetDefault()) {}
		SLATE_ATTRIBUTE(FICFrame, Frame)
		SLATE_STYLE_ARGUMENT(FFICKeyframeControlStyle, Style)
	SLATE_END_ARGS()

public:
	void Construct(FArguments InArgs, UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute);

private:
	TAttribute<FICFrame> Frame;
	const FFICKeyframeControlStyle* Style = nullptr;

	UFICEditorContext* Context = nullptr;
	TSharedPtr<FFICEditorAttributeBase> Attribute;
	
public:
	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	// End SWidget

	FICFrame GetFrame() const;
	TSharedRef<FFICEditorAttributeBase> GetAttribute() const;
};
