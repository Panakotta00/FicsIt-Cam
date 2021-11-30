#include "FICGraphView.h"

#include "FICDragDrop.h"

FSlateColorBrush SFICGraphView::DefaultAnimationBrush = FSlateColorBrush(FColor::FromHex("050505"));

void SFICGraphView::Construct(const FArguments& InArgs) {
	AnimationBrush = InArgs._AnimationBrush;
	ActiveFrame = InArgs._Frame;
	TimelineRangeBegin = InArgs._TimelineRangeBegin;
	TimelineRangeEnd = InArgs._TimelineRangeEnd;
	ValueRangeBegin = InArgs._ValueRangeBegin;
	ValueRangeEnd = InArgs._ValueRangeEnd;
	AnimationStart = InArgs._AnimationStart;
	AnimationEnd = InArgs._AnimationEnd;
	OnTimelineRangeChanged = InArgs._OnTimelineRangedChanged;
	OnValueRangeChanged = InArgs._OnValueRangeChanged;

	if (!OnTimelineRangeChanged.IsBound()) OnTimelineRangeChanged.BindLambda([this](int64 Start, int64 End) {
		if (!TimelineRangeBegin.IsBound()) TimelineRangeBegin.Set(Start);
		if (!TimelineRangeEnd.IsBound()) TimelineRangeEnd.Set(End);
	});
	if (!OnValueRangeChanged.IsBound()) OnValueRangeChanged.BindLambda([this](float Start, float End) {
		if (!ValueRangeBegin.IsBound()) ValueRangeBegin.Set(Start);
		if (!ValueRangeEnd.IsBound()) ValueRangeEnd.Set(End);
	});

	SetAttributes(InArgs._Attributes);
	Update();

	if (InArgs._AutoFit) {
		FitAll();
	}
}

SFICGraphView::SFICGraphView() : Children(this) {
	Clipping = EWidgetClipping::ClipToBoundsAlways;
}

FVector2D SFICGraphView::ComputeDesiredSize(float) const {
	return FVector2D(1000, 1000);
}

int32 SFICGraphView::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	// Draw Background
	FVector2D AnimationLocalStart = FVector2D(FrameToLocal(AnimationStart.Get()), 0);
	FVector2D AnimationLocalEnd = FVector2D(FrameToLocal(AnimationEnd.Get()), AllottedGeometry.GetLocalSize().Y);
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(AnimationLocalEnd - AnimationLocalStart, FSlateLayoutTransform(AnimationLocalStart)), AnimationBrush.Get(), ESlateDrawEffect::None, AnimationBrush.Get()->TintColor.GetSpecifiedColor());
	
	// Draw Grid
	FVector2D Distance = FVector2D(10,10);
	FVector2D Start = FVector2D(LocalToFrame(0), LocalToValue(0)) / Distance;
	FVector2D RenderOffset = FVector2D(FMath::Fractional(Start.X) * Distance.X, FMath::Fractional(Start.Y) * Distance.Y);
	int GridOffsetX = FMath::FloorToInt(Start.X) * FMath::RoundToInt(Distance.X);
	int GridOffsetY = FMath::FloorToInt(Start.Y) * FMath::RoundToInt(Distance.Y);
	//Distance *= Zoom;
	//Start *= Zoom;
	//RenderOffset *= Zoom;
	float Zoom = 1;
	FLinearColor GridColor = FLinearColor(FColor::FromHex("444444"));
	int64 FrameStart = TimelineRangeBegin.Get();
	int64 FrameEnd = TimelineRangeEnd.Get();
	int64 Steps = 1;
	while ((FrameEnd - FrameStart) / Steps > 30) Steps *= 10;
	for (float x = FrameStart - FrameStart % Steps; x <= FrameEnd; x += Steps) {
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(FrameToLocal(x), 0), FVector2D(FrameToLocal(x), AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None, GridColor, true, 1);
	}
	for (float y = 0; y <= AllottedGeometry.GetLocalSize().Y; y += Distance.Y) {
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(0, y + RenderOffset.Y), FVector2D(AllottedGeometry.GetLocalSize().X, y + RenderOffset.Y)}, ESlateDrawEffect::None, GridColor, true, 1);
	}
	FLinearColor FrameColor = FLinearColor(FColor::FromHex("666600"));
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(FrameToLocal(ActiveFrame.Get()), 0), FVector2D(FrameToLocal(ActiveFrame.Get()), AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None, FrameColor, true, 2);
	
	// Draw Plots
	for (FFICEditorAttributeBase* const& Attribute : Attributes) {
		int64 StartFrame = TimelineRangeBegin.Get();
		int64 EndFrame = TimelineRangeEnd.Get();
		TArray<FVector2D> PlotPoints;
		for (int64 Frame = StartFrame; Frame <= EndFrame; ++Frame) {
			FVector2D PlotPoint = FrameAttributeToLocal(Attribute, Frame);
			PlotPoints.Add(PlotPoint);
		}
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), PlotPoints, ESlateDrawEffect::None, Attribute->GraphColor, true, 2);
	}

	// Draw Handles
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFICKeyframeControl> Child = StaticCastSharedRef<SFICKeyframeControl>(const_cast<TSlotlessChildren<SFICKeyframeControl>*>(&Children)->GetChildAt(i));
		FFICKeyframe* Keyframe = Child->GetAttribute()->GetKeyframe(Child->GetFrame())->Get();
		if (FIC_KF_NONE < Keyframe->KeyframeType && Keyframe->KeyframeType <= FIC_KF_CUSTOM) {
			int64 Frame = Child->GetFrame();
			float Value = Keyframe->GetValueAsFloat();
			float Handle1Frame;
			float Handle1Value;
			float Handle2Frame;
			float Handle2Value;
			Keyframe->GetInControlAsFloat(Handle1Frame, Handle1Value);
			Keyframe->GetOutControlAsFloat(Handle2Frame, Handle2Value);
			TArray<FVector2D> PlotPoints;
			PlotPoints.Add(FVector2D(FrameToLocal(Frame - Handle1Frame), ValueToLocal(Value - Handle1Value)));
			PlotPoints.Add(FVector2D(FrameToLocal(Frame), ValueToLocal(Value)));
			PlotPoints.Add(FVector2D(FrameToLocal(Frame + Handle2Frame), ValueToLocal(Value + Handle2Value)));
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), PlotPoints, ESlateDrawEffect::None, FLinearColor::White, true, 1);
		}
	}

	LayerId = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId+1, InWidgetStyle, bParentEnabled);
	
	return LayerId;
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
	}
	return FReply::Unhandled();
}

FReply SFICGraphView::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
	return SPanel::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FReply SFICGraphView::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) {
		return FReply::Handled().BeginDragDrop(MakeShared<FFICGraphPanDragDrop>(SharedThis(this)));
	}
	return FReply::Unhandled();
}

FReply SFICGraphView::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return SPanel::OnMouseMove(MyGeometry, MouseEvent);
}

FReply SFICGraphView::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	float Delta = MouseEvent.GetWheelDelta() * -10.0f;
	FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	int64 StartFrame = LocalToFrame(LocalPos.X);
	float StartValue = LocalToValue(LocalPos.Y);
	float FramePerLocal = GetFramePerLocal();
	float ValuePerLocal = GetValuePerLocal();

	bool bMoveToCursor = false;
	if (MouseEvent.IsControlDown() || MouseEvent.IsShiftDown()) {
		int64 FrameBegin, FrameEnd;
		GetTimeRange(FrameBegin, FrameEnd);
		SetTimeRange(FrameBegin - Delta * FramePerLocal, FrameEnd + Delta * FramePerLocal);
		bMoveToCursor = true;
	}
	if (MouseEvent.IsControlDown() || MouseEvent.IsAltDown()) {
		float ValueBegin, ValueEnd;
		GetValueRange(ValueBegin, ValueEnd);
		SetValueRange(ValueBegin - Delta * ValuePerLocal, ValueEnd + Delta * ValuePerLocal);
		bMoveToCursor = true;
	}

	if (bMoveToCursor) {
		int64 StopFrame = LocalToFrame(LocalPos.X);
		float StopValue = LocalToValue(LocalPos.Y);
	
		int64 FrameDiff = StartFrame - StopFrame;
		float ValueDiff = StartValue - StopValue;

		GetTimeRange(StartFrame, StopFrame);
		SetTimeRange(StartFrame + FrameDiff, StopFrame + FrameDiff);
		GetValueRange(StartValue, StopValue);
		SetValueRange(StartValue + ValueDiff, StopValue + ValueDiff);

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
	for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex) {
		const TSharedRef<SFICKeyframeControl>& Child = Children[ChildIndex];
		float Frame = FrameToLocal(Child->GetFrame());
		float Value = ValueToLocal(Child->GetAttribute()->GetKeyframe(Child->GetFrame())->Get()->GetValueAsFloat());
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Child, FVector2D(Frame, Value) /* - Child->GetDesiredSize()/2.0f */, Child->GetDesiredSize(), 1));
	}
}

void SFICGraphView::SetAttributes(const TArray<FFICEditorAttributeBase*>& InAttributes) {
	Attributes = InAttributes;

	Update();
}


void SFICGraphView::Update() {
	Children.Empty();
	
	for (FFICEditorAttributeBase* Attribute : Attributes) {
		for (const TPair<int64, TSharedPtr<FFICKeyframeRef>>& Keyframe : Attribute->GetAttribute()->GetKeyframes()) {
			TSharedRef<SFICKeyframeControl> Child =
				SNew(SFICKeyframeControl)
				.Attribute(Attribute)
				.Frame(Keyframe.Key)
				.GraphView(this)
				.ShowHandles(true);
			Children.Add(Child);
		}
	}
}

void SFICGraphView::FitAll() {
	int64 FrameMin = 0;
	int64 FrameMax = 0;
	float ValueMin = 0;
	float ValueMax = 0;
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFICKeyframeControl> Child = StaticCastSharedRef<SFICKeyframeControl>(Children.GetChildAt(i));
		int64 Frame = Child->GetFrame();
		float Value = Child->GetAttribute()->GetValueAsFloat(Frame);
		if (i == 0) {
			FrameMin = FrameMax = Frame;
			ValueMin = ValueMax = Value;
		} else {
			if (Frame < FrameMin) FrameMin = Frame;
			if (FrameMax < Frame) FrameMax = Frame;
			if (Value < ValueMin) ValueMin = Value;
			if (ValueMax < Value) ValueMax = Value;
		}
	}
	OnValueRangeChanged.Execute(ValueMin, ValueMax);
	OnTimelineRangeChanged.Execute(FrameMin, FrameMax);
}

int64 SFICGraphView::LocalToFrame(float Local) const {
	return (int64) FMath::Lerp(
		(double)TimelineRangeBegin.Get(),
		(double)TimelineRangeEnd.Get(),
		Local / GetCachedGeometry().GetLocalSize().X);
}

float SFICGraphView::LocalToValue(float Local) const {
	return FMath::Lerp(
		ValueRangeEnd.Get(),
		ValueRangeBegin.Get(),
		Local / GetCachedGeometry().GetLocalSize().Y);
}

float SFICGraphView::FrameToLocal(int64 InFrame) const {
	return FMath::Lerp(
		0.0f,
		GetCachedGeometry().GetLocalSize().X,
		FMath::GetRangePct(
			(double)TimelineRangeBegin.Get(),
			(double)TimelineRangeEnd.Get(),
			(double)InFrame));
}

float SFICGraphView::ValueToLocal(float Value) const {
	return FMath::Lerp(
		GetCachedGeometry().GetLocalSize().Y,
		0.0f,
		FMath::GetRangePct(
		ValueRangeBegin.Get(),
		ValueRangeEnd.Get(),
		Value));
}

float SFICGraphView::GetFramePerLocal() const {
	return (float)(TimelineRangeEnd.Get() - TimelineRangeBegin.Get()) / GetCachedGeometry().Size.X;
}
float SFICGraphView::GetValuePerLocal() const {
	return (float)(ValueRangeEnd.Get() - ValueRangeBegin.Get()) / GetCachedGeometry().Size.Y;
}

FVector2D SFICGraphView::FrameAttributeToLocal(const FFICEditorAttributeBase* InAttribute, int64 InFrame) const {
	return FVector2D(
		FrameToLocal(InFrame),
		ValueToLocal(InAttribute->GetValueAsFloat(InFrame)));
}

TSharedPtr<SFICKeyframeControl> SFICGraphView::FindKeyframeControl(const FFICEditorAttributeBase* InAttribute, int64 InFrame) {
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFICKeyframeControl> Child = StaticCastSharedRef<SFICKeyframeControl>(Children.GetChildAt(i));
		if (Child->GetAttribute() == InAttribute && Child->GetFrame() == InFrame) {
			return Child;
		}
	}
	return nullptr;
}
