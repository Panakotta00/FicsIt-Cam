#include "Editor/UI/FICRangeSelector.h"

FSlateColorBrush SFICRangeSelector::DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex("030303"));
FLinearColor SFICRangeSelector::DefaultRangeIncrementColor = FLinearColor(FColor::FromHex("404040"));
FSlateColorBrush SFICRangeSelector::DefaultSelectBrush = FSlateColorBrush(FColor::FromHex("80808088"));
FLinearColor SFICRangeSelector::DefaultSelectHandleColor = FLinearColor(FColor::FromHex("A0A0A0"));
FLinearColor SFICRangeSelector::DefaultHighlightColor = FLinearColor(FColor::FromHex("FF8500"));

void SFICRangeSelector::Construct(const FArguments& InArgs) {
	FullRange = InArgs._FullRange;
	ActiveRange = InArgs._ActiveRange;
	ActiveFrame = InArgs._ActiveFrame;
	ActiveFrameEnabled = InArgs._ActiveFrameEnabled;

	OnActiveRangeChanged = InArgs._OnActiveRangeChanged;
	OnActiveFrameChanged = InArgs._OnActiveFrameChanged;

	BackgroundBrush = InArgs._BackgroundBrush;
	RangeIncrementColor = InArgs._RangeIncrementColor;
	SelectBrush = InArgs._SelectBrush;
	SelectHandleColor = InArgs._SelectHandleColor;
	HighlightColor = InArgs._HighlightColor;
}

int32 SFICRangeSelector::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), BackgroundBrush.Get(), ESlateDrawEffect::None, BackgroundBrush.Get()->TintColor.GetSpecifiedColor());
	
	// Draw Range Increments
	FFICFrameRange Range = FullRange.Get();
	FICFrame Increment = 10;
	while (Range.Length() / Increment > 30) Increment *= 10;
	Range.Begin += Increment - (Range.Begin % Increment);
	FLinearColor IncrementColor = RangeIncrementColor.Get();
	for (FICFrame i = Range.Begin; i <= Range.End; i += Increment) {
		float IX = FrameToLocalPos(i);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(IX, 0), FVector2D(IX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  IncrementColor, true, 5);
	}
	++LayerId;

	// Draw Highlight
	if (ActiveFrameEnabled.Get()) {
		float HighlightX = FrameToLocalPos(GetActiveFrame());
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId+1, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(HighlightX, 0), FVector2D(HighlightX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  HighlightColor.Get(), true, 5);
	}
	
	// Draw Selection Box & Handles
	FFICFrameRange Active = ActiveRange.Get();
	float StartX = FrameToLocalPos(Active.Begin);
	float EndX = FrameToLocalPos(Active.End);
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(FVector2D(StartX, 0), FVector2D(EndX - StartX, AllottedGeometry.GetLocalSize().Y)), SelectBrush.Get(), ESlateDrawEffect::None, SelectBrush.Get()->TintColor.GetSpecifiedColor());
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(StartX, 0), FVector2D(StartX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  SelectHandleColor.Get(), true, 5);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(EndX, 0), FVector2D(EndX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  SelectHandleColor.Get(), true, 5);

	return LayerId;
}

FVector2D SFICRangeSelector::ComputeDesiredSize(float) const {
	return FVector2D(20, 20);
}

FReply SFICRangeSelector::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled().DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
}

FReply SFICRangeSelector::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	float LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition()).X;
	FICFrame Frame = LocalPosToFrame(LocalPos);
	SetActiveFrame(Frame);
	return FReply::Handled();
}

FReply SFICRangeSelector::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	float LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition()).X;
	FICFrame Frame = LocalPosToFrame(LocalPos);
	FFICFrameRange Range = GetActiveRange();
	EFICRangeSelectorDragDropType DragDropType;
	if (IsLocalPosNearFrame(LocalPos, Range.Begin)) {
		DragDropType = FIC_RangeSelect_Begin;
	} else if (IsLocalPosNearFrame(LocalPos, Range.End)) {
		DragDropType = FIC_RangeSelect_End;
	} else if (IsLocalPosNearFrame(LocalPos, GetActiveFrame())) {
		DragDropType = FIC_RangeSelect_Frame;
	} else if (Range.IsInRange(Frame)) {
		DragDropType = FIC_RangeSelect_Both;
	} else {
		DragDropType = FIC_RangeSelect_Frame;
	}
	return FReply::Handled().BeginDragDrop(MakeShared<FFICRangeSelectorDragDrop>(MouseEvent, SharedThis(this), DragDropType));
}

FCursorReply SFICRangeSelector::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const {
	float LocalPos = MyGeometry.AbsoluteToLocal(CursorEvent.GetLastScreenSpacePosition()).X;
	FICFrame Frame = LocalPosToFrame(LocalPos);
	FFICFrameRange Range = GetActiveRange();
	if (IsLocalPosNearFrame(LocalPos, Range.Begin) ||
		IsLocalPosNearFrame(LocalPos, Range.End) ||
		IsLocalPosNearFrame(LocalPos, GetActiveFrame())) {
		return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
	}
	return SLeafWidget::OnCursorQuery(MyGeometry, CursorEvent);
}

FReply SFICRangeSelector::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	float Delta = MouseEvent.GetWheelDelta();
	if (!MouseEvent.GetModifierKeys().IsControlDown()) {
		int64 Range = FullRange.Get().Length();
		while (Range > 300) {
			Range /= 10;
			Delta *= 10;
		}
	}

	FFICFrameRange NewRange = ActiveRange.Get();
	NewRange.End += MouseEvent.GetWheelDelta();
	if (MouseEvent.GetModifierKeys().IsControlDown()) {
		NewRange.Begin -= Delta;
	} else {
		NewRange.Begin += Delta;
	}
	SetActiveRange(NewRange);
	return FReply::Handled();
}

FReply SFICRangeSelector::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
	FFICFrameRange Full = FullRange.Get();
	FFICFrameRange Active = ActiveRange.Get();
	if (Full.Begin == Active.Begin && Full.End == Active.End) {
		SetActiveRange(PrevActiveRange, false);
	} else {
		SetActiveRange(Full, false);
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

float SFICRangeSelector::FrameToLocalPos(FICFrame Frame) const {
	FFICFrameRange Range = FullRange.Get();
	return (GetCachedGeometry().GetLocalSize() * FVector2D(FMath::GetRangePct<float>((float)Range.Begin, Range.End, Frame), 0)).X;
}

FICFrame SFICRangeSelector::LocalPosToFrame(float LocalPos) const {
	FFICFrameRange Range = FullRange.Get();
	return FMath::RoundToFloat(FMath::Lerp((float)Range.Begin, (float)Range.End, FMath::GetRangePct(0.0, GetCachedGeometry().GetLocalSize().X, LocalPos)));
}

bool SFICRangeSelector::IsLocalPosNearFrame(float LocalPos, FICFrame Frame) const {
	float Pos = FrameToLocalPos(Frame);
	return FMath::Abs(LocalPos - Pos) < 10.0;
}

void FFICRangeSelectorDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FDragDropOperation::OnDragged(DragDropEvent);

	FVector2D Local = RangeSelector->GetCachedGeometry().AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());
	FICFrame Frame = RangeSelector->LocalPosToFrame(Local.X);
	
	FFICFrameRange NewRange = InitRange;
	FICFrame NewFrame = InitFrame;
	bool bNewRange = false, bNewFrame = false;
	switch (ChangeType) {
	case FIC_RangeSelect_Begin:
		NewRange.Begin = Frame;
		bNewRange = true;
		break;
	case FIC_RangeSelect_End:
		NewRange.End = Frame;
		bNewRange = true;
		break;
	case FIC_RangeSelect_Both:
		NewRange.Begin += Frame - InitFrame;
		NewRange.End += Frame - InitFrame;
		bNewRange = true;
		break;
	case FIC_RangeSelect_Frame:
		NewFrame = Frame;
		bNewFrame = true;
		break;
	default: ;
	}
	if (bNewRange && NewRange != RangeSelector->GetActiveRange()) RangeSelector->SetActiveRange(FFICFrameRange(NewRange.Begin, NewRange.End));
	if (bNewFrame && NewFrame != RangeSelector->GetActiveFrame()) RangeSelector->SetActiveFrame(NewFrame);
}
