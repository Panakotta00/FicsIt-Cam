#include "Editor/UI/FICKeyframeControl.h"

#include "Brushes/SlateColorBrush.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICDragDrop.h"
#include "Editor/UI/FICUIUtil.h"
#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"

void SFICKeyframeControl::Construct(const FArguments& InArgs, UFICEditorContext* InContext, TSharedRef<FFICEditorAttributeBase> InAttribute) {
	Attribute = InAttribute;
	Style = InArgs._Style;
	Frame = InArgs._Frame;
	Context = InContext;
	
	ChildSlot[
		SNew(SBox)
		.ToolTipText_Lambda([this]() {
			TSharedPtr<FFICKeyframe> KF = Attribute->GetKeyframe(GetFrame());
			if (KF) {
				switch (KF->KeyframeType) {
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
					TSharedPtr<FFICKeyframe> KF = Attribute->GetKeyframe(GetFrame());
					if (KF) {
						if (Attribute->HasChanged(GetFrame())) return Style->ChangedColor;
						else return Style->SetColor;
					} else if (Attribute->IsAnimated()) return Style->AnimatedColor;
					else return Style->UnsetColor;
				})
				.Image_Lambda([this]() {
					TSharedPtr<FFICKeyframe> KF = Attribute->GetKeyframe(GetFrame());
					if (!KF) {
						return &Style->NumericKeyframeIcons.DefaultBrush;
					}
					switch (KF->GetType()) {
					case FIC_KF_EASE:
						return &Style->NumericKeyframeIcons.AutoBrush;
					case FIC_KF_EASEINOUT:
						return &Style->NumericKeyframeIcons.EaseInOutBrush;
					case FIC_KF_MIRROR:
						return &Style->NumericKeyframeIcons.MirrorBrush;
					case FIC_KF_CUSTOM:
						return &Style->NumericKeyframeIcons.CustomBrush;
					case FIC_KF_LINEAR:
						return &Style->NumericKeyframeIcons.LinearBrush;
					case FIC_KF_STEP:
						return &Style->NumericKeyframeIcons.StepBrush;
					default:
						return &Style->NumericKeyframeIcons.DefaultBrush;
					}
				})
			]
		]
	];
}

FReply SFICKeyframeControl::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled();
}

FReply SFICKeyframeControl::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& Event) {
	if (Event.GetEffectingButton() == EKeys::LeftMouseButton) {
		FFICAttribute& Attrib = Attribute->GetAttribute();
		Attrib.LockUpdateEvent();
		BEGIN_QUICK_ATTRIB_CHANGE(Context, Attrib, GetFrame(), GetFrame())
		if (Attribute->GetKeyframe(GetFrame()) && (!Attribute->HasChanged(GetFrame()))) Attribute->RemoveKeyframe(GetFrame());
		else Attribute->SetKeyframe(GetFrame());
		Attrib.RecalculateAllKeyframes();
		END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
		Attrib.UnlockUpdateEvent();
		return FReply::Handled();
	} else 	if (Event.GetEffectingButton() == EKeys::RightMouseButton) {
        TSharedPtr<FFICKeyframe> KF = Attribute->GetKeyframe(GetFrame());
		if (KF) {
			FMenuBuilder MenuBuilder = FICCreateKeyframeTypeChangeMenu(Context, [this] {
				return TSet{TPair<FFICAttribute*, FICFrame>(&Attribute->GetAttribute(), GetFrame())};
			});
			FSlateApplication::Get().PushMenu(SharedThis(this), *Event.GetEventPath(), MenuBuilder.MakeWidget(), Event.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
		}
		return FReply::Handled();
	}
	return FReply::Handled();
}

FReply SFICKeyframeControl::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& Event) {
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = Attribute->GetAttribute().GetKeyframes();
	if (Keyframes.Num() > 0) {
		BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute->GetAttribute(), GetFrame(), GetFrame())
		Attribute->GetAttribute().LockUpdateEvent();
		for (const TPair<FICFrame, TSharedRef<FFICKeyframe>>& KF : Keyframes) Attribute->RemoveKeyframe(KF.Key);
		Attribute->GetAttribute().UnlockUpdateEvent();
		END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
	}
	return FReply::Handled();
}

FCursorReply SFICKeyframeControl::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const {
	return FCursorReply::Cursor(EMouseCursor::GrabHand);
}

FICFrame SFICKeyframeControl::GetFrame() const {
	TOptional<int64> F = Frame.Get();
	if (F.IsSet()) return F.GetValue();
	return Context->GetCurrentFrame();
}

TSharedRef<FFICEditorAttributeBase> SFICKeyframeControl::GetAttribute() const {
	return Attribute.ToSharedRef();
}
