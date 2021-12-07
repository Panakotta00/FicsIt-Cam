#include "FICKeyframeControl.h"

#include "FICDragDrop.h"
#include "FICGraphView.h"
#include "Brushes/SlateColorBrush.h"
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

void SFICKeyframeControl::Construct(FArguments InArgs) {
	Attribute = InArgs._Attribute;
	Style = InArgs._Style;
	Frame = InArgs._Frame;
	GraphView = InArgs._GraphView;
	
	Children.Add(
		SAssignNew(MainHandle, SBox)
		.ToolTipText_Lambda([this]() {
			TSharedPtr<FFICKeyframeRef> KF = Attribute.Get()->GetKeyframe(GetFrame());
	        if (KF && *KF.Get()) {
	            switch (KF.Get()->Get()->KeyframeType) {
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
					TSharedPtr<FFICKeyframeRef> KF = Attr->GetKeyframe(GetFrame());
					if (KF && *KF) {
						if (Attr->HasChanged(GetFrame())) return Style.Get()->ChangedColor;
						else return Style.Get()->SetColor;
					} else if (Attr->IsAnimated()) return Style.Get()->AnimatedColor;
					else return Style.Get()->UnsetColor;
				})
				.Image_Lambda([this]() {
					TSharedPtr<FFICKeyframeRef> KF = Attribute.Get()->GetKeyframe(GetFrame());
					if (!KF || !*KF) {
						return &Style.Get()->DefaultBrush;
					}
					switch ((*KF)->KeyframeType) {
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
		FFICKeyframe* Keyframe = GetAttribute()->GetKeyframe(GetFrame())->Get();
		TSharedPtr<FFICKeyframeRef> NextKeyframe, PrevKeyframe;
		int64 NextKeyframeTime, PrevKeyframeTime;
		Attribute.Get()->GetAttribute()->GetNextKeyframe(Frame.Get().GetValue(), NextKeyframeTime, NextKeyframe);
		Attribute.Get()->GetAttribute()->GetPrevKeyframe(Frame.Get().GetValue(), PrevKeyframeTime, PrevKeyframe);
		if (PrevKeyframe && PrevKeyframe->Get()->KeyframeType & FIC_KF_HANDLES) {
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
	TSharedPtr<FFICKeyframeRef> KeyframeRef = GetAttribute()->GetKeyframe(GetFrame());
	if ((FromHandle || ToHandle) && KeyframeRef.IsValid()) {
		FFICKeyframe* Keyframe = KeyframeRef->Get();
		float Handle1Frame;
		float Handle1Value;
		float Handle2Frame;
		float Handle2Value;
		Keyframe->GetInControlAsFloat(Handle1Frame, Handle1Value);
		Keyframe->GetOutControlAsFloat(Handle2Frame, Handle2Value);
		TArray<FVector2D> PlotPoints;
		if (FromHandle) PlotPoints.Add(FVector2D((1.0 / GraphView->GetFramePerLocal()) * -Handle1Frame, (1.0 / GraphView->GetValuePerLocal()) * Handle1Value));
		PlotPoints.Add(FVector2D(0, 0));
		if (ToHandle) PlotPoints.Add(FVector2D((1.0 / GraphView->GetFramePerLocal()) * Handle2Frame, (1.0 / GraphView->GetValuePerLocal()) * -Handle2Value));
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
		int64 TimelineBegin, TimelineEnd;
		float ValueBegin, ValueEnd;
		GraphView->GetTimeRange(TimelineBegin, TimelineEnd);
		GraphView->GetValueRange(ValueBegin, ValueEnd);

		ValuePerLocal = (float)(ValueEnd - ValueBegin) / GraphView->GetCachedGeometry().Size.Y;
		TimelinePerLocal = (float)(TimelineEnd - TimelineBegin) / GraphView->GetCachedGeometry().Size.X;

		FFICKeyframe* Keyframe = Attribute.Get()->GetKeyframe(Frame.Get().GetValue())->Get();

		if (Keyframe) {
			if (FromHandle) {
				float FromFrame, FromValue;
				Keyframe->GetInControlAsFloat(FromFrame, FromValue);
				FVector2D FromHandleOffset(-FromFrame, FromValue);
				FromHandleOffset /= FVector2D(TimelinePerLocal, ValuePerLocal);
				FromHandleOffset -= FromHandle->GetDesiredSize()/2.0f;
				ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(FromHandle.ToSharedRef(), FromHandleOffset, FromHandle->GetDesiredSize(), 1));
			}

			if (ToHandle) {
				float ToFrame, ToValue;
				Keyframe->GetOutControlAsFloat(ToFrame, ToValue);
				FVector2D ToHandleOffset(ToFrame, -ToValue);
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
	else return FReply::Unhandled();
}

FReply SFICKeyframeControl::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& Event) {
	if (Event.GetEffectingButton() == EKeys::LeftMouseButton && !bWasDoubleClick) {
		FFICAttribute* Attrib = Attribute.Get()->GetAttribute();
		if (Attribute.Get()->GetKeyframe(GetFrame()) && (ToHandle || FromHandle || !Attribute.Get()->HasChanged(GetFrame()))) Attribute.Get()->RemoveKeyframe(GetFrame());
		else Attribute.Get()->SetKeyframe(GetFrame());
		Attrib->OnUpdate.Broadcast();
		return FReply::Handled();
	} else 	if (Event.GetEffectingButton() == EKeys::RightMouseButton) {
        TSharedPtr<FFICKeyframeRef> KF = Attribute.Get()->GetKeyframe(GetFrame());
		if (KF) {
			TSharedPtr<IMenu> MenuHandle;
			FMenuBuilder MenuBuilder(true, NULL);
			MenuBuilder.AddMenuEntry(
                FText::FromString("Ease"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                    Attribute.Get()->SetKeyframeFromFloat(GetFrame(), Attribute.Get()->GetKeyframe(GetFrame())->Get()->GetValueAsFloat(), FIC_KF_EASE, false);
                	Attribute.Get()->GetAttribute()->RecalculateAllKeyframes();
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
			MenuBuilder.AddMenuEntry(
                FText::FromString("Ease-In/Out"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                    Attribute.Get()->SetKeyframeFromFloat(GetFrame(), Attribute.Get()->GetKeyframe(GetFrame())->Get()->GetValueAsFloat(), FIC_KF_EASEINOUT, false);
                	Attribute.Get()->GetAttribute()->RecalculateAllKeyframes();
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
			MenuBuilder.AddMenuEntry(
                FText::FromString("Linear"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                    Attribute.Get()->SetKeyframeFromFloat(GetFrame(), Attribute.Get()->GetKeyframe(GetFrame())->Get()->GetValueAsFloat(), FIC_KF_LINEAR, false);
                	Attribute.Get()->GetAttribute()->RecalculateAllKeyframes();
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
			MenuBuilder.AddMenuEntry(
                FText::FromString("Step"),
                FText(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([KF, this]() {
                    Attribute.Get()->SetKeyframeFromFloat(GetFrame(), Attribute.Get()->GetKeyframe(GetFrame())->Get()->GetValueAsFloat(), FIC_KF_STEP, false);
                	Attribute.Get()->GetAttribute()->RecalculateAllKeyframes();
                }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
		
			FSlateApplication::Get().PushMenu(SharedThis(this), *Event.GetEventPath(), MenuBuilder.MakeWidget(), Event.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
		}
		return FReply::Handled();
	}
	return SPanel::OnMouseButtonUp(MyGeometry, Event);
}

FReply SFICKeyframeControl::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& Event) {
	bWasDoubleClick = true;

	if (!GraphView) {
		FFICEditorAttributeBase* Attr = Attribute.Get();
		TMap<int64, TSharedPtr<FFICKeyframeRef>> Keyframes = Attr->GetAttribute()->GetKeyframes();
		if (Keyframes.Num() > 0) {
			for (const TPair<int64, TSharedPtr<FFICKeyframeRef>>& KF : Keyframes) Attr->RemoveKeyframe(KF.Key);
			Attr->GetAttribute()->OnUpdate.Broadcast();
			return FReply::Handled();
		}
	}
	return SPanel::OnMouseButtonDoubleClick(MyGeometry, Event);
}

FReply SFICKeyframeControl::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		if (GraphView) {
			return FReply::Handled().BeginDragDrop(MakeShared<FFICGraphKeyframeDragDrop>(SharedThis(GraphView), SharedThis(this), MouseEvent));
		}
	}
	
	return SPanel::OnDragDetected(MyGeometry, MouseEvent);
}

FCursorReply SFICKeyframeControl::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const {
	return FCursorReply::Cursor(EMouseCursor::GrabHand);
}

int64 SFICKeyframeControl::GetFrame() const {
	TOptional<int64> F = Frame.Get();
	if (F.IsSet()) return F.GetValue();
	return Attribute.Get()->GetFrame();
}

FFICEditorAttributeBase* SFICKeyframeControl::GetAttribute() const {
	return Attribute.Get();
}
