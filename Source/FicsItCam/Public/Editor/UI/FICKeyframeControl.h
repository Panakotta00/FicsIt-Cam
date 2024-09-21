#pragma once

#include "FICEditorStyle.h"
#include "SCompoundWidget.h"
#include "Data/FICTypes.h"
#include "FICKeyframeControl.generated.h"

class FFICEditorAttributeBase;
class UFICEditorContext;
class SFICGraphView;

class SFICKeyframeControl : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SFICKeyframeControl) :
		_Style(&FFICEditorStyles::Get().GetWidgetStyle<FFICKeyframeControlStyle>(TEXT("KeyframeControl"))) {}
		SLATE_ATTRIBUTE(FICFrame, Frame)
		SLATE_STYLE_ARGUMENT(FFICKeyframeControlStyle, Style)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute);

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
