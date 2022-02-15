#include "Editor/UI/FICKeyframeControl.h"

#include "Brushes/SlateColorBrush.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICDragDrop.h"
#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"

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

void SFICKeyframeHandle::Construct(const FArguments& InArgs, SFICKeyframeControl* InKeyframeControl) {
	KeyframeControl = InKeyframeControl;
	
	bIsOutHandle = InArgs._IsOutHandle;
	Style = InArgs._Style;

	ChildSlot[
		SNew(SBox)
		.HeightOverride(20)
		.WidthOverride(20)
		.Padding(5)[
			SNew(SImage)
			.Image_Lambda([this]() {
				return &Style.Get()->HandleBrush;
			})
			.ColorAndOpacity_Lambda([this]() {
				return Style.Get()->SetColor.GetSpecifiedColor();
			})
		]
	];
}

FReply SFICKeyframeHandle::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled().DetectDrag(AsShared(), EKeys::LeftMouseButton);
}

FReply SFICKeyframeHandle::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		return FReply::Handled().BeginDragDrop(MakeShared<FFICGraphKeyframeHandleDragDrop>(SharedThis(this), MouseEvent));
	}
	return FReply::Unhandled();
}

FCursorReply SFICKeyframeHandle::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const {
	return FCursorReply::Cursor(EMouseCursor::GrabHand);
}

FFICKeyframeControlStyle* SFICKeyframeControl::DefaultStyle() {
	static FFICKeyframeControlStyle* Style = nullptr;
	if (!Style) Style = new FFICKeyframeControlStyle();
	return Style;
}

SFICKeyframeControl::SFICKeyframeControl() : Children(this) {}

void SFICKeyframeControl::Construct(FArguments InArgs, UFICEditorContext* InContext) {
	Attribute = InArgs._Attribute;
	Style = InArgs._Style;
	Frame = InArgs._Frame;
	GraphView = InArgs._GraphView;
	Context = InContext;
	
	Children.Add(
		SAssignNew(MainHandle, SBox)
		.ToolTipText_Lambda([this]() {
			TSharedPtr<FFICKeyframe> KF = Attribute.Get()->GetKeyframe(GetFrame());
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
					FFICEditorAttributeBase* Attr = Attribute.Get();
					if (!Attr) return Style.Get()->UnsetColor;
					TSharedPtr<FFICKeyframe> KF = Attr->GetKeyframe(GetFrame());
					if (KF) {
						if (Attr->HasChanged(GetFrame())) return Style.Get()->ChangedColor;
						else return Style.Get()->SetColor;
					} else if (Attr->IsAnimated()) return Style.Get()->AnimatedColor;
					else return Style.Get()->UnsetColor;
				})
				.Image_Lambda([this]() {
					TSharedPtr<FFICKeyframe> KF = Attribute.Get()->GetKeyframe(GetFrame());
					if (!KF) {
						return &Style.Get()->DefaultBrush;
					}
					switch (KF->KeyframeType) {
					case FIC_KF_EASE:
						return &Style.Get()->AutoBrush;
					case FIC_KF_EASEINOUT:
						return &Style.Get()->EaseInOutBrush;
					case FIC_KF_MIRROR:
						return &Style.Get()->MirrorBrush;
					case FIC_KF_CUSTOM:
						return &Style.Get()->CustomBrush;
					case FIC_KF_LINEAR:
						return &Style.Get()->LinearBrush;
					case FIC_KF_STEP:
						return &Style.Get()->StepBrush;
					default:
						return &Style.Get()->DefaultBrush;
					}
				})
			]
		]
	);

	if (!InArgs._GraphView) InArgs._ShowHandles = false;
	if (InArgs._ShowHandles) {
		TSharedPtr<FFICKeyframe> Keyframe = GetAttribute()->GetKeyframe(GetFrame());
		TSharedPtr<FFICKeyframe> NextKeyframe, PrevKeyframe;
		int64 NextKeyframeTime, PrevKeyframeTime;
		NextKeyframe = Attribute.Get()->GetAttribute().GetNextKeyframe(Frame.Get().GetValue(), NextKeyframeTime);
		PrevKeyframe = Attribute.Get()->GetAttribute().GetPrevKeyframe(Frame.Get().GetValue(), PrevKeyframeTime);
		if (PrevKeyframe && PrevKeyframe->KeyframeType & FIC_KF_HANDLES) {
			FromHandle = SNew(SFICKeyframeHandle, this).Style(Style);
			Children.Add(FromHandle.ToSharedRef());
		}
		if (Keyframe && (Keyframe->KeyframeType & FIC_KF_HANDLES)) {
			ToHandle = SNew(SFICKeyframeHandle, this).IsOutHandle(true).Style(Style);
			Children.Add(ToHandle.ToSharedRef());
		}
	}
}

int SFICKeyframeControl::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	TSharedPtr<FFICKeyframe> Keyframe = GetAttribute()->GetKeyframe(GetFrame());
	if ((FromHandle || ToHandle) && Keyframe.IsValid()) {
		FFICValueTimeFloat InControl = Keyframe->GetInControl();
		FFICValueTimeFloat OutControl = Keyframe->GetOutControl();
		TArray<FVector2D> PlotPoints;
		if (FromHandle) PlotPoints.Add(FVector2D((1.0 / GraphView->GetFramePerLocal()) * -InControl.Frame, (1.0 / GraphView->GetValuePerLocal()) * InControl.Value));
		PlotPoints.Add(FVector2D(0, 0));
		if (ToHandle) PlotPoints.Add(FVector2D((1.0 / GraphView->GetFramePerLocal()) * OutControl.Frame, (1.0 / GraphView->GetValuePerLocal()) * -OutControl.Value));
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId+5, AllottedGeometry.ToPaintGeometry(), PlotPoints, ESlateDrawEffect::None, Style.Get()->SetColor.GetSpecifiedColor(), true, 2);
	}
	
	int NewLayerId = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId+10, InWidgetStyle, bParentEnabled);

	return NewLayerId+10;
}

FVector2D SFICKeyframeControl::ComputeDesiredSize(float F) const {
	return MainHandle->GetDesiredSize();
}

FChildren* SFICKeyframeControl::GetChildren() {
	return &Children;
}

void SFICKeyframeControl::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(MainHandle.ToSharedRef(), (FromHandle || ToHandle) ? -MainHandle->GetDesiredSize()/2.0f : FVector2D::ZeroVector, MainHandle->GetDesiredSize(), 1));
	if (FromHandle || ToHandle) {
		float TimelinePerLocal, ValuePerLocal;
		FFICFrameRange FrameRange = GraphView->GetFrameRange();
		FFICValueRange ValueRange = GraphView->GetValueRange();
		
		ValuePerLocal = (float)(ValueRange.Length()) / GraphView->GetCachedGeometry().Size.Y;
		TimelinePerLocal = (float)(FrameRange.Length()) / GraphView->GetCachedGeometry().Size.X;

		TSharedPtr<FFICKeyframe> Keyframe = Attribute.Get()->GetKeyframe(Frame.Get().GetValue());

		if (Keyframe) {
			if (FromHandle) {
				FFICValueTimeFloat InControl = Keyframe->GetInControl();
				FVector2D FromHandleOffset(-InControl.Frame, InControl.Value);
				FromHandleOffset /= FVector2D(TimelinePerLocal, ValuePerLocal);
				FromHandleOffset -= FromHandle->GetDesiredSize()/2.0f;
				ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(FromHandle.ToSharedRef(), FromHandleOffset, FromHandle->GetDesiredSize(), 1));
			}

			if (ToHandle) {
				FFICValueTimeFloat OutControl = Keyframe->GetOutControl();
				FVector2D ToHandleOffset(OutControl.Frame, -OutControl.Value);
				ToHandleOffset /= FVector2D(TimelinePerLocal, ValuePerLocal);
				ToHandleOffset -= ToHandle->GetDesiredSize()/2.0f;
				ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(ToHandle.ToSharedRef(), ToHandleOffset, ToHandle->GetDesiredSize(), 1));
			}
		}
	}
}

FReply SFICKeyframeControl::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	bWasDoubleClick = false;
	if (GraphView) return FReply::Handled().DetectDrag(AsShared(), EKeys::LeftMouseButton);
	return FReply::Handled();
}

FReply SFICKeyframeControl::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& Event) {
	if (Event.GetEffectingButton() == EKeys::LeftMouseButton && !bWasDoubleClick) {
		FFICAttribute& Attrib = Attribute.Get()->GetAttribute();
        BEGIN_QUICK_ATTRIB_CHANGE(Context, Attrib, GetFrame(), GetFrame())
		if (Attribute.Get()->GetKeyframe(GetFrame()) && (ToHandle || FromHandle || !Attribute.Get()->HasChanged(GetFrame()))) Attribute.Get()->RemoveKeyframe(GetFrame());
		else Attribute.Get()->SetKeyframe(GetFrame());
		Attrib.RecalculateAllKeyframes();
		END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
		return FReply::Handled();
	} else 	if (Event.GetEffectingButton() == EKeys::RightMouseButton) {
        TSharedPtr<FFICKeyframe> KF = Attribute.Get()->GetKeyframe(GetFrame());
		if (KF) {
			TSharedPtr<IMenu> MenuHandle;
			FMenuBuilder MenuBuilder(true, NULL);
			MenuBuilder.AddMenuEntry(
                FText::FromString("Ease"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                	BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute.Get()->GetAttribute(), GetFrame(), GetFrame())
                    Attribute.Get()->SetKeyframe(FFICValueTime(GetFrame(), Attribute.Get()->GetKeyframe(GetFrame())->GetValue()), FIC_KF_EASE, false);
                	Attribute.Get()->GetAttribute().RecalculateAllKeyframes();
                	END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
			MenuBuilder.AddMenuEntry(
                FText::FromString("Ease-In/Out"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                	BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute.Get()->GetAttribute(), GetFrame(), GetFrame())
                    Attribute.Get()->SetKeyframe(FFICValueTime(GetFrame(), Attribute.Get()->GetKeyframe(GetFrame())->GetValue()), FIC_KF_EASEINOUT, false);
                	Attribute.Get()->GetAttribute().RecalculateAllKeyframes();
                	END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
			MenuBuilder.AddMenuEntry(
                FText::FromString("Linear"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                	BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute.Get()->GetAttribute(), GetFrame(), GetFrame())
                    Attribute.Get()->SetKeyframe(FFICValueTime(GetFrame(), Attribute.Get()->GetKeyframe(GetFrame())->GetValue()), FIC_KF_LINEAR, false);
                	Attribute.Get()->GetAttribute().RecalculateAllKeyframes();
                	END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
			MenuBuilder.AddMenuEntry(
                FText::FromString("Step"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                	BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute.Get()->GetAttribute(), GetFrame(), GetFrame())
                    Attribute.Get()->SetKeyframe(FFICValueTime(GetFrame(), Attribute.Get()->GetKeyframe(GetFrame())->GetValue()), FIC_KF_STEP, false);
                	Attribute.Get()->GetAttribute().RecalculateAllKeyframes();
                	END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
		
			FSlateApplication::Get().PushMenu(SharedThis(this), *Event.GetEventPath(), MenuBuilder.MakeWidget(), Event.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
		}
		return FReply::Handled();
	}
	return FReply::Handled();
}

FReply SFICKeyframeControl::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& Event) {
	bWasDoubleClick = true;

	if (!GraphView) {
		FFICEditorAttributeBase* Attr = Attribute.Get();
		TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = Attr->GetAttribute().GetKeyframes();
		if (Keyframes.Num() > 0) {
			BEGIN_QUICK_ATTRIB_CHANGE(Context, Attr->GetAttribute(), GetFrame(), GetFrame())
			for (const TPair<FICFrame, TSharedRef<FFICKeyframe>>& KF : Keyframes) Attr->RemoveKeyframe(KF.Key);
			Attr->GetAttribute().OnUpdate.Broadcast();
			END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
		}
	}
	return FReply::Handled();
}

FReply SFICKeyframeControl::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		if (GraphView) {
			return FReply::Handled().BeginDragDrop(MakeShared<FFICGraphKeyframeDragDrop>(SharedThis(GraphView), SharedThis(this), MouseEvent));
		}
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

FFICEditorAttributeBase* SFICKeyframeControl::GetAttribute() const {
	return Attribute.Get();
}
