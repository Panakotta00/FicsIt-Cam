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
	HighlightAttr = InArgs._Highlight;
	HighlightEnabled = InArgs._HighlightEnabled;
	SelectStartChanged = InArgs._SelectStartChanged;
	SelectEndChanged = InArgs._SelectEndChanged;
	HighlightChanged = InArgs._HighlightChanged;
	BackgroundBrush = InArgs._BackgroundBrush;
	RangeIncrementColor = InArgs._RangeIncrementColor;
	SelectBrush = InArgs._SelectBrush;
	SelectHandleColor = InArgs._SelectHandleColor;
	HighlightColor = InArgs._HighlightColor;

	SelectStart = SelectStartAttr.Get();
	SelectEnd = SelectEndAttr.Get();
	Highlight = HighlightAttr.Get();
	
	PrevSelectStart = SelectStart;
	PrevSelectEnd = SelectEnd;
}

void SFICRangeSelector::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	if (SelectStartAttr.IsBound()) SelectStart = SelectStartAttr.Get();
	if (SelectEndAttr.IsBound()) SelectEnd = SelectEndAttr.Get();
	if (HighlightAttr.IsBound()) Highlight = HighlightAttr.Get();
}

int32 SFICRangeSelector::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), BackgroundBrush.Get(), ESlateDrawEffect::None, BackgroundBrush.Get()->TintColor.GetSpecifiedColor());

	// Draw Range Increments
	int64 Start = RangeStart.Get();
	int64 End = RangeEnd.Get();
	int64 Increment = 10;
	while ((End - Start) / Increment > 30) Increment *= 10;
	Start += Increment - (Start % Increment);
	FLinearColor IncrementColor = RangeIncrementColor.Get();
	for (int64 i = Start; i <= End; i += Increment) {
		float IX = RangePosToLocalPos(AllottedGeometry, i);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(IX, 0), FVector2D(IX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  IncrementColor, true, 5);
	}
	++LayerId;

	// Draw Highlight
	if (HighlightEnabled.Get()) {
		float HighlightX = RangePosToLocalPos(AllottedGeometry, Highlight);
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
	return FVector2D(20, 20);
}

FReply SFICRangeSelector::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	float LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition()).X;
	int64 RangePos = LocalPosToRangePos(MyGeometry, LocalPos);
	if (!(bStartDrag || bEndDrag || bSelectDrag || bHighlightDrag)) {
		if (IsLocalPosNearRangePos(MyGeometry, LocalPos, SelectStart)) {
			bStartDrag = true;
			LastDragPos = DragStartPos = RangePos;
			DragDelta = 0;
		} else if (IsLocalPosNearRangePos(MyGeometry, LocalPos, SelectEnd)) {
			bEndDrag = true;
			LastDragPos = DragStartPos = RangePos;
			DragDelta = 0;
		} else if (IsLocalPosNearRangePos(MyGeometry, LocalPos, Highlight)) {
			bHighlightDrag = true;
			LastDragPos = DragStartPos = RangePos;
			DragDelta = 0;
		} else if (RangePos > SelectStart && RangePos < SelectEnd) {
			bSelectDrag = true;
			LastDragPos = DragStartPos = RangePos;
			DragDelta = 0;
			SelectDragStart = SelectStart;
			SelectDragEnd = SelectEnd;
		} else {
			bHighlightDrag = true;
			LastDragPos = DragStartPos = RangePos;
			DragDelta = 0;
		}
		SetCursor(EMouseCursor::ResizeLeftRight);
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	return FReply::Unhandled();
}

FReply SFICRangeSelector::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	int64 RangePos = LocalPosToRangePos(MyGeometry, MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition()).X);
	if (bStartDrag || bEndDrag || bSelectDrag || bHighlightDrag) {
		if (DragDelta == 0) {
			SetHighlight(RangePos);
		}
		bStartDrag = false;
		bEndDrag = false;
		bSelectDrag = false;
		bHighlightDrag = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply SFICRangeSelector::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	int64 RangePos = LocalPosToRangePos(MyGeometry, MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition()).X);

	if (!(bStartDrag || bEndDrag || bSelectDrag || bHighlightDrag)) {
		if ((RangePos >= SelectStart && RangePos <= SelectEnd) || RangePos == Highlight) {
			SetCursor(EMouseCursor::ResizeLeftRight);
		} else {
			SetCursor(EMouseCursor::Default);
		}
	}
	
	DragDelta += FMath::Abs(RangePos - LastDragPos);
	LastDragPos = RangePos;
	if (bStartDrag) {
		SetSelectStart(RangePos);
		return FReply::Handled();
	} else if (bEndDrag) {
		SetSelectEnd(RangePos);
		return FReply::Handled();
	} else if (bSelectDrag) {
		int64 SelectDiff = RangePos - DragStartPos;
		SetSelectStart(SelectDragStart + SelectDiff);
		SetSelectEnd(SelectDragEnd + SelectDiff);
		return FReply::Handled();
	} else if (bHighlightDrag) {
		SetHighlight(RangePos);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SFICRangeSelector::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	float Delta = MouseEvent.GetWheelDelta();
	if (!MouseEvent.GetModifierKeys().IsControlDown()) {
		int64 Range = RangeEnd.Get() - RangeStart.Get();
		while (Range > 300) {
			Range /= 10;
			Delta *= 10;
		}
	}
	int64 NewEnd = SelectEnd;
	int64 NewStart = SelectStart;
	NewEnd += MouseEvent.GetWheelDelta();
	if (MouseEvent.GetModifierKeys().IsControlDown()) {
		NewStart -= Delta;
	} else {
		NewStart += Delta;
	}
	if (Delta > 0) {
		SetSelectEnd(NewEnd);
		SetSelectStart(NewStart);
	} else {
		SetSelectStart(NewStart);
		SetSelectEnd(NewEnd);
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
	return (MyGeometry.GetLocalSize() * FVector2D(FMath::GetRangePct<float>((float)RangeStart.Get(), (float)RangeEnd.Get(), RangePos), 0)).X;
}

int64 SFICRangeSelector::LocalPosToRangePos(const FGeometry& MyGeometry, float LocalPos) const {
	return FMath::RoundToFloat(FMath::Lerp((float)RangeStart.Get(), (float)RangeEnd.Get(), FMath::GetRangePct(0.0f, MyGeometry.GetLocalSize().X, LocalPos)));
}

bool SFICRangeSelector::IsLocalPosNearRangePos(const FGeometry& MyGeometry, float LocalPos, int64 RangePos) const {
	float Pos = RangePosToLocalPos(MyGeometry, RangePos);
	return FMath::Abs(LocalPos - Pos) < 10.0;
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

void SFICRangeSelector::SetHighlight(int64 inHighlight) {
	if (Highlight == inHighlight) return;
	bool _ = HighlightChanged.ExecuteIfBound(Highlight, inHighlight);
	Highlight = inHighlight;
}

int64 SFICRangeSelector::GetSelectStart() const {
	return SelectStart;
}

int64 SFICRangeSelector::GetSelectEnd() const {
	return SelectEnd;
}

int64 SFICRangeSelector::GetHighlight() const {
	return Highlight;
}
