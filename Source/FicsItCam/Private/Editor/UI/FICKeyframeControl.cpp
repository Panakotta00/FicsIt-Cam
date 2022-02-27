#include "Editor/UI/FICKeyframeControl.h"

#include "Brushes/SlateColorBrush.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICDragDrop.h"
#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"

const FName FFICKeyframeControlStyle::TypeName = TEXT("KeyframeControlStyle");

const FFICKeyframeControlStyle& FFICKeyframeControlStyle::GetDefault() {
	static FFICKeyframeControlStyle* Style = nullptr;
	if (!Style) Style = new FFICKeyframeControlStyle();
	return *Style;
}

FFICKeyframeControlStyle::FFICKeyframeControlStyle() {
	UnsetColor = FSlateColor(FColor::FromHex("5A5A5A"));
	SetColor = FSlateColor(FColor::FromHex("FFAA00"));
	ChangedColor = FSlateColor(FColor::FromHex("AAAAFF"));
	AnimatedColor = FSlateColor(FColor::FromHex("AA8800"));

	UTexture2D* Texture = LoadObject<UTexture2D>(NULL, TEXT("/Game/FicsItCam/Ease.Ease"));
	if (Texture) {
		Texture->AddToRoot();
		DefaultBrush.DrawAs = ESlateBrushDrawType::Image;
		DefaultBrush.ImageType = ESlateBrushImageType::FullColor;
		DefaultBrush.SetResourceObject(Texture);
		DefaultBrush.ImageSize.X = Texture->GetSizeX();
		DefaultBrush.ImageSize.Y = Texture->GetSizeY();
	} else {
		DefaultBrush = static_cast<FSlateBrush>(FSlateColorBrush(FColor::White));
	}
	StepBrush = LinearBrush = EaseInOutBrush = MirrorBrush = CustomBrush = AutoBrush = DefaultBrush;

	Texture = LoadObject<UTexture2D>(NULL, TEXT("/Game/FicsItCam/Ease_InOut.Ease_InOut"));
	if (Texture) {
		Texture->AddToRoot();
		EaseInOutBrush.SetResourceObject(Texture);
		EaseInOutBrush.ImageSize.X = Texture->GetSizeX();
		EaseInOutBrush.ImageSize.Y = Texture->GetSizeY();
	}

	Texture = LoadObject<UTexture2D>(NULL, TEXT("/Game/FicsItCam/Linear.Linear"));
	if (Texture) {
		Texture->AddToRoot();
		LinearBrush.SetResourceObject(Texture);
		LinearBrush.ImageSize.X = Texture->GetSizeX();
		LinearBrush.ImageSize.Y = Texture->GetSizeY();
	}

	Texture = LoadObject<UTexture2D>(NULL, TEXT("/Game/FicsItCam/Step.Step"));
	if (Texture) {
		Texture->AddToRoot();
		StepBrush.SetResourceObject(Texture);
		StepBrush.ImageSize.X = Texture->GetSizeX();
		StepBrush.ImageSize.Y = Texture->GetSizeY();
	}

	Texture = LoadObject<UTexture2D>(NULL, TEXT("/Game/FicsItCam/Handle.Handle"));
	if (Texture) {
		Texture->AddToRoot();
		HandleBrush.SetResourceObject(Texture);
		HandleBrush.ImageSize.X = Texture->GetSizeX();
		HandleBrush.ImageSize.Y = Texture->GetSizeY();
	}
}

void SFICKeyframeControl::Construct(FArguments InArgs, UFICEditorContext* InContext, TSharedRef<FFICEditorAttributeBase> InAttribute) {
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
						return &Style->DefaultBrush;
					}
					switch (KF->KeyframeType) {
					case FIC_KF_EASE:
						return &Style->AutoBrush;
					case FIC_KF_EASEINOUT:
						return &Style->EaseInOutBrush;
					case FIC_KF_MIRROR:
						return &Style->MirrorBrush;
					case FIC_KF_CUSTOM:
						return &Style->CustomBrush;
					case FIC_KF_LINEAR:
						return &Style->LinearBrush;
					case FIC_KF_STEP:
						return &Style->StepBrush;
					default:
						return &Style->DefaultBrush;
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
        BEGIN_QUICK_ATTRIB_CHANGE(Context, Attrib, GetFrame(), GetFrame())
		if (Attribute->GetKeyframe(GetFrame()) && (!Attribute->HasChanged(GetFrame()))) Attribute->RemoveKeyframe(GetFrame());
		else Attribute->SetKeyframe(GetFrame());
		Attrib.RecalculateAllKeyframes();
		END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
		return FReply::Handled();
	} else 	if (Event.GetEffectingButton() == EKeys::RightMouseButton) {
        TSharedPtr<FFICKeyframe> KF = Attribute->GetKeyframe(GetFrame());
		if (KF) {
			TSharedPtr<IMenu> MenuHandle;
			FMenuBuilder MenuBuilder(true, NULL);
			MenuBuilder.AddMenuEntry(
                FText::FromString("Ease"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                	BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute->GetAttribute(), GetFrame(), GetFrame())
                    Attribute->SetKeyframe(FFICValueTime(GetFrame(), Attribute->GetKeyframe(GetFrame())->GetValue()), FIC_KF_EASE, false);
                	Attribute->GetAttribute().RecalculateAllKeyframes();
                	END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
			MenuBuilder.AddMenuEntry(
                FText::FromString("Ease-In/Out"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                	BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute->GetAttribute(), GetFrame(), GetFrame())
                    Attribute->SetKeyframe(FFICValueTime(GetFrame(), Attribute->GetKeyframe(GetFrame())->GetValue()), FIC_KF_EASEINOUT, false);
                	Attribute->GetAttribute().RecalculateAllKeyframes();
                	END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
			MenuBuilder.AddMenuEntry(
                FText::FromString("Linear"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                	BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute->GetAttribute(), GetFrame(), GetFrame())
                    Attribute->SetKeyframe(FFICValueTime(GetFrame(), Attribute->GetKeyframe(GetFrame())->GetValue()), FIC_KF_LINEAR, false);
                	Attribute->GetAttribute().RecalculateAllKeyframes();
                	END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
			MenuBuilder.AddMenuEntry(
                FText::FromString("Step"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                	BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute->GetAttribute(), GetFrame(), GetFrame())
                    Attribute->SetKeyframe(FFICValueTime(GetFrame(), Attribute->GetKeyframe(GetFrame())->GetValue()), FIC_KF_STEP, false);
                	Attribute->GetAttribute().RecalculateAllKeyframes();
                	END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
		
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
		for (const TPair<FICFrame, TSharedRef<FFICKeyframe>>& KF : Keyframes) Attribute->RemoveKeyframe(KF.Key);
		Attribute->GetAttribute().OnUpdate.Broadcast();
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
