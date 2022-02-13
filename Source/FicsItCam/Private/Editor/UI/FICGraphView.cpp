#include "Editor/UI/FICGraphView.h"

#include "Editor/FICChangeList.h"
#include "Editor/FICEditorAttributeBase.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICDragDrop.h"

FSlateColorBrush SFICGraphView::DefaultAnimationBrush = FSlateColorBrush(FColor::FromHex("050505"));

void SFICGraphView::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	AnimationBrush = InArgs._AnimationBrush;
	ActiveFrame = InArgs._Frame;
	FrameRange = InArgs._FrameRange;
	ValueRange = InArgs._ValueRange;
	FrameHighlightRange = InArgs._FrameHighlightRange;
	OnFrameRangeChanged = InArgs._OnFrameRangeChanged;
	OnValueRangeChanged = InArgs._OnValueRangeChanged;
	Context = InContext;

	SetAttributes(InArgs._Attributes);
	Update();

	if (InArgs._AutoFit) {
		FitAll();
	}
}

SFICGraphView::SFICGraphView() : Children(this) {
	Clipping = EWidgetClipping::ClipToBoundsAlways;
}

SFICGraphView::~SFICGraphView() {
	for (FFICEditorAttributeBase* Attribute : Attributes) {
		Attribute->GetAttribute()->OnUpdate.Remove(DelegateHandles[Attribute]);
	}
}

FVector2D SFICGraphView::ComputeDesiredSize(float) const {
	return FVector2D(0, 0);
}

int32 SFICGraphView::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	OutDrawElements.PushClip(FSlateClippingZone(MyCullingRect));

	FFICFrameRange Range = FrameRange.Get();
	FFICFrameRange Highlight = FrameHighlightRange.Get();
	FFICValueRange Value = ValueRange.Get();
	
	// Draw Highlighted Frame Range Background
	FVector2D AnimationLocalStart = FVector2D(FrameToLocal(Highlight.Begin), 0);
	FVector2D AnimationLocalEnd = FVector2D(FrameToLocal(Highlight.End), AllottedGeometry.GetLocalSize().Y);
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(AnimationLocalEnd - AnimationLocalStart, FSlateLayoutTransform(AnimationLocalStart)), AnimationBrush.Get(), ESlateDrawEffect::None, AnimationBrush.Get()->TintColor.GetSpecifiedColor());
	
	// Draw Grid
	FVector2D Distance = FVector2D(10,10);
	FVector2D Start = FVector2D(LocalToFrame(0), LocalToValue(0)) / Distance;
	FVector2D RenderOffset = FVector2D(FMath::Fractional(Start.X) * Distance.X, FMath::Fractional(Start.Y) * Distance.Y);
	FLinearColor GridColor = FLinearColor(FColor::FromHex("444444"));
	int64 Steps = 1;
	int64 SafetyCounter = 0;
	while (Range.Length() / Steps > 30) Steps *= 10;
	for (float x = Range.Begin - Range.Begin % Steps; x <= Range.End; x += Steps) {
		if (SafetyCounter++ > 1000) break;
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(FrameToLocal(x), 0), FVector2D(FrameToLocal(x), AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None, GridColor, true, 1);
	}
	SafetyCounter = 0;
	for (float y = 0; y <= AllottedGeometry.GetLocalSize().Y; y += Distance.Y) {
		if (SafetyCounter++ > 1000) break;
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(0, y + RenderOffset.Y), FVector2D(AllottedGeometry.GetLocalSize().X, y + RenderOffset.Y)}, ESlateDrawEffect::None, GridColor, true, 1);
	}
	FLinearColor FrameColor = FLinearColor(FColor::FromHex("666600"));
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(FrameToLocal(ActiveFrame.Get()), 0), FVector2D(FrameToLocal(ActiveFrame.Get()), AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None, FrameColor, true, 2);
	
	// Draw Plots
	for (FFICEditorAttributeBase* const& Attribute : Attributes) {
		TArray<FVector2D> PlotPoints;
		for (FICFrame PlotFrame : Range) {
			FVector2D PlotPoint = FrameAttributeToLocal(Attribute, PlotFrame);
			PlotPoints.Add(PlotPoint);
		}
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), PlotPoints, ESlateDrawEffect::None, Attribute->GraphColor, true, 2);
	}

	LayerId = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId+10, InWidgetStyle, bParentEnabled);

	OutDrawElements.PopClip();
	
	return LayerId+20;
}

FReply SFICGraphView::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Unhandled().DetectDrag(AsShared(), EKeys::RightMouseButton);
}

FReply SFICGraphView::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton) {
		TSharedPtr<IMenu> MenuHandle;
		FMenuBuilder MenuBuilder(true, NULL);
		MenuBuilder.AddMenuEntry(
			FText::FromString("FitAll"),
			FText(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this]() {
				FitAll();
			}), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
				
		FSlateApplication::Get().PushMenu(SharedThis(this), *MouseEvent.GetEventPath(), MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);

		return FReply::Handled();
	} else if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		FVector2D LocalMousePos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		for (FFICEditorAttributeBase* const& Attribute : Attributes) {
			FFICFrameRange Range = FrameRange.Get();
			for (FICFrame Frame : Range) {
				FVector2D PlotPoint = FrameAttributeToLocal(Attribute, Frame);
				FVector2D Difference = PlotPoint - LocalMousePos;
				if (Difference.Size() < 5) {
					BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute->GetAttribute(), TNumericLimits<int64>::Min(), Frame)
					Attribute->SetKeyframe(FFICValueTime(Frame, LocalToValue(LocalMousePos.Y)));
					Attribute->GetAttribute()->RecalculateAllKeyframes();
					END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
					return FReply::Handled();
				}
			}
		}
	}
	return FReply::Unhandled();
}

FReply SFICGraphView::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
	return SPanel::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FReply SFICGraphView::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) {
		return FReply::Handled().BeginDragDrop(MakeShared<FFICGraphPanDragDrop>(SharedThis(this), MouseEvent));
	}
	return FReply::Unhandled();
}

FReply SFICGraphView::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return SPanel::OnMouseMove(MyGeometry, MouseEvent);
}

FReply SFICGraphView::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	float Delta = MouseEvent.GetWheelDelta() * -10.0f;
	FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	FICFrame StartFrame = LocalToFrame(LocalPos.X);
	FICValue StartValue = LocalToValue(LocalPos.Y);
	float FramePerLocal = GetFramePerLocal();
	float ValuePerLocal = GetValuePerLocal();

	bool bMoveToCursor = false;
	if (MouseEvent.IsControlDown() || MouseEvent.IsShiftDown()) {
		FFICFrameRange Range = GetFrameRange();
		SetFrameRange(FFICFrameRange(Range.Begin - Delta * FramePerLocal, Range.End + Delta * FramePerLocal));
		bMoveToCursor = true;
	}
	if (MouseEvent.IsControlDown() || MouseEvent.IsAltDown()) {
		FFICValueRange Range = GetValueRange();
		SetValueRange(FFICValueRange(Range.Begin - Delta * ValuePerLocal, Range.End + Delta * ValuePerLocal));
		bMoveToCursor = true;
	}

	if (bMoveToCursor) {
		FICFrame StopFrame = LocalToFrame(LocalPos.X);
		FICValue StopValue = LocalToValue(LocalPos.Y);
	
		FICFrame FrameDiff = StartFrame - StopFrame;
		FICValue ValueDiff = StartValue - StopValue;

		GetFrameRange().GetRange(StartFrame, StopFrame);
		SetFrameRange(FFICFrameRange(StartFrame + FrameDiff, StopFrame + FrameDiff));
		GetValueRange().GetRange(StartValue, StopValue);
		SetValueRange(FFICValueRange(StartValue + ValueDiff, StopValue + ValueDiff));

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SFICGraphView::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	return SPanel::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SFICGraphView::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	return SPanel::OnKeyUp(MyGeometry, InKeyEvent);
}

bool SFICGraphView::IsInteractable() const {
	return true;
}

FChildren* SFICGraphView::GetChildren() {
	return &Children;
}

void SFICGraphView::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	for (int ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex) {
		TSharedRef<SFICKeyframeControl> Child = Children[ChildIndex];
		float Frame = FrameToLocal(Child->GetFrame());
		float Value = ValueToLocal(Child->GetAttribute()->GetKeyframe(Child->GetFrame())->GetValue());
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Child, FVector2D(Frame, Value), Child->GetDesiredSize(), 1));
	}
}

void SFICGraphView::SetAttributes(const TArray<FFICEditorAttributeBase*>& InAttributes) {
	for (FFICEditorAttributeBase* Attribute : Attributes) {
		Attribute->GetAttribute()->OnUpdate.Remove(DelegateHandles[Attribute]);
	}
	
	Attributes = InAttributes;

	for (FFICEditorAttributeBase* Attribute : Attributes) {
		DelegateHandles.Add(Attribute, Attribute->GetAttribute()->OnUpdate.AddRaw(this, &SFICGraphView::Update));
	}
	
	Update();
}


void SFICGraphView::Update() {
	Children.Empty();
	
	for (FFICEditorAttributeBase* Attribute : Attributes) {
		for (const TPair<FICFrame, TSharedRef<FFICKeyframe>>& Keyframe : Attribute->GetAttribute()->GetKeyframes()) {
			TSharedRef<SFICKeyframeControl> Child =
				SNew(SFICKeyframeControl, Context)
				.Attribute(Attribute)
				.Frame(Keyframe.Key)
				.GraphView(this)
				.ShowHandles(true);
			Children.Add(Child);
		}
	}
}

#pragma optimize("", off)
void SFICGraphView::FitAll() {
	FFICFrameRange Frames = FrameRange.Get();
	FFICValueRange Values;
	Values.Begin = TNumericLimits<FICValue>::Max();
	Values.End = TNumericLimits<FICValue>::Lowest();
	int MaxKeyframeCountOfAnyAttribute = 0;
	for (FFICEditorAttributeBase* Attribute : Attributes) {
		TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = Attribute->GetAttribute()->GetKeyframes();
		for (TTuple<FICFrame, TSharedRef<FFICKeyframe>> Keyframe : Keyframes) {
			TSharedRef<FFICKeyframe> KF = Keyframe.Value;
			FICValue Value = KF->GetValue();
			FICFrame Frame = Keyframe.Key;
			FFICValueTimeFloat InControl = KF->GetInControl();
			FFICValueTimeFloat OutControl = KF->GetOutControl();
			Frames.Begin = FMath::Min3(Frames.Begin, Frame, (int64)FMath::Min(Frame + InControl.Frame, Frame + OutControl.Frame));
			Frames.End = FMath::Max3(Frames.End, Frame, (int64)FMath::Max(Frame + InControl.Frame, Frame + OutControl.Frame));
			Values.Begin = FMath::Min3(Values.Begin, Value, FMath::Min(Value - InControl.Value, Value + OutControl.Value));
			Values.End = FMath::Max3(Values.End, Value, FMath::Max(Value - InControl.Value, Value + OutControl.Value));
		}
		MaxKeyframeCountOfAnyAttribute = FMath::Max(MaxKeyframeCountOfAnyAttribute, Keyframes.Num());
	}
	FICFrame FrameSpan = FMath::Max(Frames.Length(), 10ll);
	FICValue ValueSpan = FMath::Max(Values.Length(), 1.0f);
	FrameSpan = FrameSpan/10;
	ValueSpan = ValueSpan/10.0;
	if (MaxKeyframeCountOfAnyAttribute > 1) {
		Frames.Begin -= FrameSpan;
		Frames.End += FrameSpan;
	}
	OnFrameRangeChanged.Execute(Frames);
	if (MaxKeyframeCountOfAnyAttribute > 0) {
		Values.Begin -= ValueSpan;
		Values.End += ValueSpan;
	} else {
		Values.Begin = -1;
		Values.End = 1;
	}
	OnValueRangeChanged.Execute(Values);
}
#pragma optimize("", on)

FICFrame SFICGraphView::LocalToFrame(float Local) const {
	FFICFrameRange Frames = FrameRange.Get();
	return (int64) FMath::Lerp(
		(double)Frames.Begin,
		(double)Frames.End,
		Local / GetCachedGeometry().GetLocalSize().X);
}

FICValue SFICGraphView::LocalToValue(float Local) const {
	FFICValueRange Values = ValueRange.Get();
	return FMath::Lerp(
		Values.Begin,
		Values.End,
		Local / GetCachedGeometry().GetLocalSize().Y);
}

float SFICGraphView::FrameToLocal(FICFrame InFrame) const {
	FFICFrameRange Frames = FrameRange.Get();
	return FMath::Lerp(
		0.0f,
		GetCachedGeometry().GetLocalSize().X,
		FMath::GetRangePct(
			(double)Frames.Begin,
			(double)Frames.End,
			(double)InFrame));
}

float SFICGraphView::ValueToLocal(FICValue Value) const {
	FFICValueRange Values = ValueRange.Get();
	return FMath::Lerp(
		GetCachedGeometry().GetLocalSize().Y,
		0.0f,
		FMath::GetRangePct(
		Values.Begin,
		Values.End,
		Value));
}

float SFICGraphView::GetFramePerLocal() const {
	return (float)(FrameRange.Get().Length()) / GetCachedGeometry().Size.X;
}
float SFICGraphView::GetValuePerLocal() const {
	return (float)(ValueRange.Get().Length()) / GetCachedGeometry().Size.Y;
}

FVector2D SFICGraphView::FrameAttributeToLocal(const FFICEditorAttributeBase* InAttribute, FICFrame InFrame) const {
	return FVector2D(
		FrameToLocal(InFrame),
		ValueToLocal(InAttribute->GetValue(InFrame)));
}

TSharedPtr<SFICKeyframeControl> SFICGraphView::FindKeyframeControl(const FFICEditorAttributeBase* InAttribute, FICFrame InFrame) {
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFICKeyframeControl> Child = StaticCastSharedRef<SFICKeyframeControl>(Children.GetChildAt(i));
		if (Child->GetAttribute() == InAttribute && Child->GetFrame() == InFrame) {
			return Child;
		}
	}
	return nullptr;
}
