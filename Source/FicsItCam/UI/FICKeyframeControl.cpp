#include "FICKeyframeControl.h"

FSlateColorBrush FFICKeyframeControlStyle::DefaultKFBrush = FSlateColorBrush(FColor::White);
FSlateColor FFICKeyframeControlStyle::DefaultUnsetColor = FSlateColor(FColor::FromHex("5A5A5A"));
FSlateColor FFICKeyframeControlStyle::DefaultSetColor = FSlateColor(FColor::FromHex("FFAA00"));
FSlateColor FFICKeyframeControlStyle::DefaultChangedColor = FSlateColor(FColor::FromHex("AAAAFF"));
FSlateColor FFICKeyframeControlStyle::DefaultAnimatedColor = FSlateColor(FColor::FromHex("AA8800"));

void SFICKeyframeControl::Construct(FArguments InArgs) {
	Attribute = InArgs._Attribute;
	Style = InArgs._Style;
	Frame = InArgs._Frame;
	
	ChildSlot[
		SNew(SBox)
		.WidthOverride(10)
		.HeightOverride(10)
		.Content()[
			SNew(SImage)
			.ColorAndOpacity_Lambda([this]() {
				FFICEditorAttributeBase* Attr = Attribute.Get();
				if (!Attr) return Style.Get().UnsetColor;
				TSharedPtr<FFICKeyframeRef> KF = Attr->GetKeyframe(GetFrame());
				if (KF && *KF) {
					if (Attr->HasChanged(GetFrame())) return Style.Get().ChangedColor;
					else return Style.Get().SetColor;
				} else if (Attr->IsAnimated()) return Style.Get().AnimatedColor;
				else return Style.Get().UnsetColor;
			})
			.Image_Lambda([this]() {
				TSharedPtr<FFICKeyframeRef> KF = Attribute.Get()->GetKeyframe(GetFrame());
				if (!KF || !*KF) {
					return Style.Get().DefaultBrush;
				}
				switch ((*KF)->KeyframeType) {
				case FIC_KF_EASE:
					return Style.Get().AutoBrush;
				case FIC_KF_EASEINOUT:
					return Style.Get().EaseInOutBrush;
				case FIC_KF_MIRROR:
					return Style.Get().MirrorBrush;
				case FIC_KF_CUSTOM:
					return Style.Get().CustomBrush;
				case FIC_KF_LINEAR:
					return Style.Get().LinearBrush;
				case FIC_KF_STEP:
					return Style.Get().StepBrush;
				default:
					return Style.Get().DefaultBrush;
				}
			})
		]
	];
}

FReply SFICKeyframeControl::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& Event) {
	if (Event.GetEffectingButton() == EKeys::LeftMouseButton) {
		if (Attribute.Get()->GetKeyframe(GetFrame()) && !Attribute.Get()->HasChanged(GetFrame())) Attribute.Get()->RemoveKeyframe(GetFrame());
		else Attribute.Get()->SetKeyframe(GetFrame());
		return FReply::Handled();
	}
	return SCompoundWidget::OnMouseButtonDown(MyGeometry, Event);
}

FReply SFICKeyframeControl::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& Event) {
	FFICEditorAttributeBase* Attr = Attribute.Get();
	TMap<int64, TSharedPtr<FFICKeyframeRef>> Keyframes = Attr->GetAttribute()->GetKeyframes();
	if (Keyframes.Num() > 0) {
		for (const TPair<int64, TSharedPtr<FFICKeyframeRef>>& KF : Keyframes) Attr->RemoveKeyframe(KF.Key);
		return FReply::Handled();
	}
	return SCompoundWidget::OnMouseButtonDoubleClick(MyGeometry, Event);
}

int64 SFICKeyframeControl::GetFrame() {
	TOptional<int64> F = Frame.Get();
	if (F.IsSet()) return F.GetValue();
	return Attribute.Get()->GetFrame();
}
