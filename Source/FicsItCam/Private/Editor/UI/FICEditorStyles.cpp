#include "CoreStyle.h"
#include "Editor/UI/FICEditorStyle.h"
#include "Slate/SlateGameResources.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FFICEditorStyles::StyleInstance = NULL;

void FFICEditorStyles::Initialize() {
	if (!StyleInstance.IsValid()) {
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FFICEditorStyles::Shutdown() {
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique()); 
	StyleInstance.Reset();
}

FName FFICEditorStyles::GetStyleSetName() {
	static FName StyleSetName(TEXT("FICEditorStyles"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FFICEditorStyles::Create() {
	TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(FFICEditorStyles::GetStyleSetName(), "/FicsItCam/Styles", "/FicsItCam/Styles");
	return StyleRef;
}

const ISlateStyle& FFICEditorStyles::Get() {
	return *StyleInstance;
}

const FName FFICNumericKeyframeIcons::TypeName = TEXT("FFICNumericKeyframeIcons");

const FFICNumericKeyframeIcons& FFICNumericKeyframeIcons::GetDefault() {
	static FFICNumericKeyframeIcons Style;
	return Style;
}

const FName FFICKeyframeIconStyle::TypeName = TEXT("FFICKeyframeIcon");

const FFICKeyframeIconStyle& FFICKeyframeIconStyle::GetDefault() {
	static FFICKeyframeIconStyle Style;
	return Style;
}

const FName FFICKeyframeControlStyle::TypeName = TEXT("FFICKeyframeControlStyle");

const FFICKeyframeControlStyle& FFICKeyframeControlStyle::GetDefault() {
	static FFICKeyframeControlStyle Style;
	return Style;
}

const FName FFICSequencerStyle::TypeName = TEXT("FFICSequencerStyle");

const FFICSequencerStyle& FFICSequencerStyle::GetDefault() {
	static FFICSequencerStyle Style;
	return Style;
}

const FName FFICGraphViewStyle::TypeName = TEXT("FFICGraphViewStyle");

const FFICGraphViewStyle& FFICGraphViewStyle::GetDefault() {
	static FFICGraphViewStyle Style;
	return Style;
}

const FName FFICRangeSelectorStyle::TypeName = TEXT("FFICRangeSelectorStyle");

const FFICRangeSelectorStyle& FFICRangeSelectorStyle::GetDefault() {
	static FFICRangeSelectorStyle Style;
	return Style;
}

const FName FFICVectorEditStyle::TypeName = TEXT("FFICVectorEditStyle");

const FFICVectorEditStyle& FFICVectorEditStyle::GetDefault() {
	static FFICVectorEditStyle Style;
	return Style;
}

FFICVectorEditStyle::FFICVectorEditStyle() {
	SpinBoxStyle = FCoreStyle::Get().GetWidgetStyle<FSpinBoxStyle>("NumericEntrySpinBox");
	SpinBoxStyle.ActiveFillBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
	SpinBoxStyle.InactiveFillBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
}

const FName FFICTimelineStyle::TypeName = TEXT("FFICTimelineStyle");

const FFICTimelineStyle& FFICTimelineStyle::GetDefault() {
	static FFICTimelineStyle Style;
	return Style;
}

const FName FFICEditorStyle::TypeName = TEXT("FFICEditorStyle");

const FFICEditorStyle& FFICEditorStyle::GetDefault() {
	static FFICEditorStyle Style;
	return Style;
}
