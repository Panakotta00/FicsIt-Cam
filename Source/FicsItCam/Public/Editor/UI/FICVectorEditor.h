#pragma once

#include "Data/FICTypes.h"
#include "Data/Attributes/FICAttributeFloat.h"
#include "Editor/Data/FICEditorAttributeBase.h"

class UFICEditorContext;
class FFICEditorAttributeBase;

class SFICVectorEditor : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICVectorEditor) :
		_Style(&FFICVectorEditStyle::GetDefault()),
		_ShowKeyframeControls(false) {}
		SLATE_STYLE_ARGUMENT(FFICVectorEditStyle, Style)
		SLATE_ARGUMENT(bool, ShowKeyframeControls)
		SLATE_ATTRIBUTE(FICFrame, Frame)
		SLATE_ATTRIBUTE(bool, AutoKeyframe)
	SLATE_END_ARGS()
	
public:
	SFICVectorEditor();
	
	void Construct(FArguments InArgs, UFICEditorContext* Context, TSharedRef<TFICEditorAttribute<FFICFloatAttribute>> XAttr, TSharedRef<TFICEditorAttribute<FFICFloatAttribute>> YAttr, TSharedRef<TFICEditorAttribute<FFICFloatAttribute>> ZAttr);

private:
	UFICEditorContext* Context = nullptr;
	const FFICVectorEditStyle* Style = nullptr;
	
	TSharedPtr<TFICEditorAttribute<FFICFloatAttribute>> XAttr;
	TSharedPtr<TFICEditorAttribute<FFICFloatAttribute>> YAttr;
	TSharedPtr<TFICEditorAttribute<FFICFloatAttribute>> ZAttr;

	TAttribute<FICFrame> Frame;
	TAttribute<bool> AutoKeyframe;
};
