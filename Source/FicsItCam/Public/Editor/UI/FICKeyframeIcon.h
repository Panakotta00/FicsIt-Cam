#pragma once

#include "FICKeyframeControl.h"
#include "SCompoundWidget.h"
#include "Data/Attributes/FICKeyframe.h"
#include "FICKeyframeIcon.generated.h"

class SFICKeyframeIcon : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICKeyframeIcon) : _Style(&FFICEditorStyles::Get().GetWidgetStyle<FFICKeyframeIconStyle>("KeyframeIcon")) {}
	SLATE_STYLE_ARGUMENT(FFICKeyframeIconStyle, Style)
	SLATE_ATTRIBUTE(TSharedPtr<FFICKeyframe>, Keyframe)
	SLATE_ATTRIBUTE(bool, IsSelected)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	const FFICKeyframeIconStyle* Style = nullptr;
	TAttribute<TSharedPtr<FFICKeyframe>> Keyframe;
	TAttribute<bool> IsSelected;
};
