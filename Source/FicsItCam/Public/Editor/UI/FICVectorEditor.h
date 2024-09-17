#pragma once

#include "Data/FICTypes.h"
#include "Data/Attributes/FICAttributeFloat.h"
#include "Editor/Data/FICEditorAttributeBase.h"

class UFICEditorContext;
class FFICEditorAttributeBase;

class SFICVectorEditor : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICVectorEditor) :
		_ShowKeyframeControls(false) {}
		SLATE_ARGUMENT(bool, ShowKeyframeControls)
		SLATE_ATTRIBUTE(FICFrame, Frame)
		SLATE_ATTRIBUTE(bool, AutoKeyframe)
	SLATE_END_ARGS()
	
public:
	SFICVectorEditor();
	
	void Construct(FArguments InArgs, UFICEditorContext* Context, TSharedRef<TFICEditorAttribute<FFICFloatAttribute>> XAttr, TSharedRef<TFICEditorAttribute<FFICFloatAttribute>> YAttr, TSharedRef<TFICEditorAttribute<FFICFloatAttribute>> ZAttr);

private:
	UFICEditorContext* Context = nullptr;
	
	TSharedPtr<TFICEditorAttribute<FFICFloatAttribute>> XAttr;
	TSharedPtr<TFICEditorAttribute<FFICFloatAttribute>> YAttr;
	TSharedPtr<TFICEditorAttribute<FFICFloatAttribute>> ZAttr;

	TAttribute<FICFrame> Frame;
	TAttribute<bool> AutoKeyframe;
	
	FSlateColorBrush XBrush = FSlateColorBrush(FLinearColor::Red);
	FSlateColorBrush YBrush = FSlateColorBrush(FLinearColor::Green);
	FSlateColorBrush ZBrush = FSlateColorBrush(FLinearColor::Blue);

	FSpinBoxStyle SpinBoxStyle;
};
