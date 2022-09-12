#include "Editor/UI/FICKeyframeIcon.h"

#include "SlateBasics.h"

const FName FFICKeyframeIconStyle::TypeName = TEXT("FFICKeyframeIconStyle");

const FFICKeyframeIconStyle& FFICKeyframeIconStyle::GetDefault() {
	static FFICKeyframeIconStyle* Default = nullptr;
	if (!Default) {
		Default = new FFICKeyframeIconStyle();
		*Default = FFICEditorStyles::Get().GetWidgetStyle<FFICKeyframeIconStyle>("KeyframeIcon");
		Default->Icons = FFICNumericKeyframeIcons::GetDefault();
	}
	return *Default;
}

void SFICKeyframeIcon::Construct(const FArguments& InArgs) {
	Style = InArgs._Style;
	Keyframe = InArgs._Keyframe;
	IsSelected = InArgs._IsSelected;
	
	ChildSlot[
		SNew(SBox)
		.ToolTipText_Lambda([this]() {
			if (Keyframe.Get()) {
				switch (Keyframe.Get()->KeyframeType) {
				case FIC_KF_EASE:
					return FText::FromString("Ease");
				case FIC_KF_EASEINOUT:
					return FText::FromString("Ease-In/Out");
				case FIC_KF_LINEAR:
					return FText::FromString("Linear");
				case FIC_KF_STEP:
					return FText::FromString("Step");
				default:
					break;
				}
			}
			return FText();
		})
		.Content()[
			SNew(SBox)
			.Padding(2)
			.WidthOverride(20)
			.HeightOverride(20)
			.Content()[
				SNew(SImage)
				.ColorAndOpacity_Lambda([this]() {
					if (IsSelected.Get()) return Style->SelectedColor;
					return Style->UnselectedColor;
				})
				.Image_Lambda([this]() {
					if (!Keyframe.Get()) {
						return &Style->Icons.DefaultBrush;
					}
					switch (Keyframe.Get()->GetType()) {
					case FIC_KF_EASE:
						return &Style->Icons.AutoBrush;
					case FIC_KF_EASEINOUT:
						return &Style->Icons.EaseInOutBrush;
					case FIC_KF_MIRROR:
						return &Style->Icons.MirrorBrush;
					case FIC_KF_CUSTOM:
						return &Style->Icons.CustomBrush;
					case FIC_KF_LINEAR:
						return &Style->Icons.LinearBrush;
					case FIC_KF_STEP:
						return &Style->Icons.StepBrush;
					default:
						return &Style->Icons.DefaultBrush;
					}
				})
			]
		]
	];
}
