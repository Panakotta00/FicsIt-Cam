#include "FICTimelineScrubber.h"


FSlateColorBrush SFICTimelineScrubber::DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex("030303"));
FLinearColor SFICTimelineScrubber::DefaultIncrementColor = FLinearColor(FColor::FromHex("404040"));
FLinearColor SFICTimelineScrubber::DefaultFrameColor = FLinearColor(FColor::FromHex("FF8500"));

void SFICTimelineScrubber::Construct(const FArguments& InArgs) {
	RangeStart = InArgs._RangeStart;
	RangeEnd = InArgs._RangeEnd;
	BackgroundBrush = InArgs._BackgroundBrush;
	IncrementColor = InArgs._IncrementColor;
	FrameColor = InArgs._FrameColor;
	FrameChanged = InArgs._FrameChanged;

	Frame = FrameAttr.Get();

	SetCursor(EMouseCursor::ResizeLeftRight);
}

void SFICTimelineScrubber::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	if (FrameAttr.IsBound()) Frame = FrameAttr.Get();
}

int32 SFICTimelineScrubber::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), BackgroundBrush.Get(), ESlateDrawEffect::None, BackgroundBrush.Get()->TintColor.GetSpecifiedColor());

	// Draw Range Increments
	int64 Start = RangeStart.Get();
	int64 End = RangeEnd.Get();
	Start += 10 - (Start % 10);
	FLinearColor IncColor = IncrementColor.Get();
	for (int64 i = Start; i <= End; i += 10) {
		float IX = RangePosToLocalPos(AllottedGeometry, i);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(IX, 0), FVector2D(IX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  IncColor, true, 5);
	}
	++LayerId;

	// Draw Highlight
	float FrameX = RangePosToLocalPos(AllottedGeometry, Frame);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId+1, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(FrameX, 0), FVector2D(FrameX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  FrameColor.Get(), true, 5);

	return LayerId;
}

FVector2D SFICTimelineScrubber::ComputeDesiredSize(float) const {
	return FVector2D(200, 200);
}

FReply SFICTimelineScrubber::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	SetFrame(LocalPosToRangePos(MyGeometry, MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X));
	bDrag = true;
	return FReply::Handled().CaptureMouse(SharedThis(this));
}

FReply SFICTimelineScrubber::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (bDrag) {
		bDrag = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply SFICTimelineScrubber::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (bDrag) {
		SetFrame(LocalPosToRangePos(MyGeometry, MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X));
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SFICTimelineScrubber::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	SetFrame(GetFrame() + MouseEvent.GetWheelDelta());
	return FReply::Handled();
}

bool SFICTimelineScrubber::IsInteractable() const {
	return true;
}

TSharedPtr<IToolTip> SFICTimelineScrubber::GetToolTip() {
	return SLeafWidget::GetToolTip();
}

void SFICTimelineScrubber::OnToolTipClosing() {
	SLeafWidget::OnToolTipClosing();
}

float SFICTimelineScrubber::RangePosToLocalPos(const FGeometry& MyGeometry, int64 RangePos) const {
	return MyGeometry.GetLocalPositionAtCoordinates(FVector2D(FMath::GetRangePct(RangeStart.Get(), RangeEnd.Get(), RangePos), 0)).X;
}

int64 SFICTimelineScrubber::LocalPosToRangePos(const FGeometry& MyGeometry, float LocalPos) const {
	return FMath::RoundToInt(FMath::Lerp((float)RangeStart.Get(), (float)RangeEnd.Get(), FMath::GetRangePct(0.0f, MyGeometry.GetLocalPositionAtCoordinates(FVector2D(1,1)).X, LocalPos)));
}

void SFICTimelineScrubber::SetFrame(int64 inFrame) {
	inFrame = FMath::Clamp(inFrame, RangeStart.Get(), RangeEnd.Get());
	if (Frame == inFrame) return;
	bool _ = FrameChanged.ExecuteIfBound(Frame, inFrame);
	Frame = inFrame;
}

int64 SFICTimelineScrubber::GetFrame() const {
	return Frame;
}
