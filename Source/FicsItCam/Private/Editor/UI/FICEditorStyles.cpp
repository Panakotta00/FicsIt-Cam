#include "Editor/UI/FICEditorStyle.h"
#include "Slate/SlateGameResources.h"

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
	static FFICNumericKeyframeIcons* Default = nullptr;
	if (!Default) {
		Default = new FFICNumericKeyframeIcons();
		*Default = FFICEditorStyles::Get().GetWidgetStyle<FFICNumericKeyframeIcons>("NumericKeyframeIcons");
	}
	return *Default;
}
