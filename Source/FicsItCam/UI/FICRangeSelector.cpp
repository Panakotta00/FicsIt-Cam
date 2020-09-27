#include "FICRangeSelector.h"

FSlateColorBrush SFICRangeSelector::DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex("030303"));
FLinearColor SFICRangeSelector::DefaultRangeIncrementColor = FLinearColor(FColor::FromHex("404040"));
FSlateColorBrush SFICRangeSelector::DefaultSelectBrush = FSlateColorBrush(FColor::FromHex("80808088"));
FLinearColor SFICRangeSelector::DefaultSelectHandleColor = FLinearColor(FColor::FromHex("A0A0A0"));
FLinearColor SFICRangeSelector::DefaultHighlightColor = FLinearColor(FColor::FromHex("FF8500"));

void SFICRangeSelector::Construct(const FArguments& InArgs) {
	RangeStart = InArgs._RangeStart;
	RangeEnd = InArgs._RangeEnd;
	SelectStartAttr = InArgs._SelectStart;
	SelectEndAttr = InArgs._SelectEnd;
	Highlight = InArgs._Highlight;
	HighlightEnabled = InArgs._HighlightEnabled;
	SelectStartChanged = InArgs._SelectStartChanged;
	SelectEndChanged = InArgs._SelectEndChanged;
	BackgroundBrush = InArgs._BackgroundBrush;
	RangeIncrementColor = InArgs._RangeIncrementColor;
	SelectBrush = InArgs._SelectBrush;
	SelectHandleColor = InArgs._SelectHandleColor;
	HighlightColor = InArgs._HighlightColor;

	SelectStart = SelectStartAttr.Get();
	SelectEnd = SelectEndAttr.Get();
	
	PrevSelectStart = SelectStart;
	PrevSelectEnd = SelectEnd;
}

void SFICRangeSelector::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	if (SelectStartAttr.IsBound()) SelectStart = SelectStartAttr.Get();
	if (SelectEndAttr.IsBound()) SelectEnd = SelectEndAttr.Get();
}

int32 SFICRangeSelector::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), BackgroundBrush.Get(), ESlateDrawEffect::None, BackgroundBrush.Get()->TintColor.GetSpecifiedColor());

	// Draw Range Increments
	int64 Start = RangeStart.Get();
	int64 End = RangeEnd.Get();
	Start += 10 - (Start % 10);
	FLinearColor IncrementColor = RangeIncrementColor.Get();
	for (int64 i = Start; i <= End; i += 10) {
		float IX = RangePosToLocalPos(AllottedGeometry, i);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(IX, 0), FVector2D(IX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  IncrementColor, true, 5);
	}
	++LayerId;

	// Draw Highlight
	if (HighlightEnabled.Get()) {
		float HighlightX = RangePosToLocalPos(AllottedGeometry, Highlight.Get());
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId+1, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(HighlightX, 0), FVector2D(HighlightX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  HighlightColor.Get(), true, 5);
	}
	
	// Draw Selection Box & Handles
	float StartX = RangePosToLocalPos(AllottedGeometry, SelectStart);
	float EndX = RangePosToLocalPos(AllottedGeometry, SelectEnd);
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(FVector2D(StartX, 0), FVector2D(EndX - StartX, AllottedGeometry.GetLocalSize().Y)), SelectBrush.Get(), ESlateDrawEffect::None, SelectBrush.Get()->TintColor.GetSpecifiedColor());
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(StartX, 0), FVector2D(StartX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  SelectHandleColor.Get(), true, 5);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(EndX, 0), FVector2D(EndX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  SelectHandleColor.Get(), true, 5);

	return LayerId;
}

FVector2D SFICRangeSelector::ComputeDesiredSize(float) const {
	return FVector2D(100, 50);
}

FReply SFICRangeSelector::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	int64 RangePos = LocalPosToRangePos(MyGeometry, MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition()).X);
	if (!(bStartDrag || bEndDrag || bSelectDrag) && RangePos >= SelectStart && RangePos <= SelectEnd) {
		if (RangePos == SelectStart) {
			bStartDrag = true;
		} else if (RangePos == SelectEnd) {
			bEndDrag = true;
		} else {
			bSelectDrag = true;
			SelectDragStartPos = RangePos;
			SelectDragStart = SelectStart;
			SelectDragEnd = SelectEnd;
		}
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	return FReply::Unhandled();
}

FReply SFICRangeSelector::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (bStartDrag || bEndDrag || bSelectDrag) {
		bStartDrag = false;
		bEndDrag = false;
		bSelectDrag = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply SFICRangeSelector::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	int64 RangePos = LocalPosToRangePos(MyGeometry, MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition()).X);
	if (RangePos >= SelectStart && RangePos <= SelectEnd) {
		SetCursor(EMouseCursor::ResizeLeftRight);
	} else {
		SetCursor(EMouseCursor::Default);
	}
	
	if (bStartDrag) {
		SetSelectStart(RangePos);
		return FReply::Handled();
	} else if (bEndDrag) {
		SetSelectEnd(RangePos);
		return FReply::Handled();
	} else if (bSelectDrag) {
		int64 SelectDiff = RangePos - SelectDragStartPos;
		SetSelectStart(SelectDragStart + SelectDiff);
		SetSelectEnd(SelectDragEnd + SelectDiff);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SFICRangeSelector::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetWheelDelta() > 0) {
		SetSelectEnd(SelectEnd + MouseEvent.GetWheelDelta());
		SetSelectStart(SelectStart + MouseEvent.GetWheelDelta());
	} else {
		SetSelectStart(SelectStart + MouseEvent.GetWheelDelta());
		SetSelectEnd(SelectEnd + MouseEvent.GetWheelDelta());
	}
	return FReply::Handled();
}

FReply SFICRangeSelector::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
	if (SelectStart == RangeStart.Get() && SelectEnd == RangeEnd.Get()) {
		SetSelectStart(PrevSelectStart);
		SetSelectEnd(PrevSelectEnd);
	} else {
		PrevSelectStart = SelectStart;
		PrevSelectEnd = SelectEnd;
		SetSelectStart(RangeStart.Get());
		SetSelectEnd(RangeEnd.Get());
	}
	return FReply::Handled();
}

bool SFICRangeSelector::IsInteractable() const {
	return true;
}

TSharedPtr<IToolTip> SFICRangeSelector::GetToolTip() {
	return SLeafWidget::GetToolTip();
}

void SFICRangeSelector::OnToolTipClosing() {
	SLeafWidget::OnToolTipClosing();
}

float SFICRangeSelector::RangePosToLocalPos(const FGeometry& MyGeometry, int64 RangePos) const {
	return MyGeometry.GetLocalPositionAtCoordinates(FVector2D(FMath::GetRangePct(RangeStart.Get(), RangeEnd.Get(), RangePos), 0)).X;
}

int64 SFICRangeSelector::LocalPosToRangePos(const FGeometry& MyGeometry, float LocalPos) const {
	return FMath::RoundToFloat(FMath::Lerp((float)RangeStart.Get(), (float)RangeEnd.Get(), FMath::GetRangePct(0.0f, MyGeometry.GetLocalPositionAtCoordinates(FVector2D(1,1)).X, LocalPos)));
}

void SFICRangeSelector::SetSelectStart(int64 Start) {
	Start = FMath::Clamp(Start, RangeStart.Get(), SelectEnd-1);
	if (SelectStart == Start) return;
	bool _ = SelectStartChanged.ExecuteIfBound(SelectStart, Start);
	SelectStart = Start;
}

void SFICRangeSelector::SetSelectEnd(int64 End) {
	End = FMath::Clamp(End, SelectStart+1, RangeEnd.Get());
	if (SelectEnd == End) return;
	bool _ = SelectEndChanged.ExecuteIfBound(SelectEnd, End);
	SelectEnd = End;
}

int64 SFICRangeSelector::GetSelectStart() const {
	return SelectStart;
}

int64 SFICRangeSelector::GetSelectEnd() const {
	return SelectEnd;
}
