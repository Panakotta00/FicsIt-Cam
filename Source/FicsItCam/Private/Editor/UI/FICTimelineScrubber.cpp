#include "Editor/UI/FICTimelineScrubber.h"

#include "FicsItCamModule.h"

FSlateColorBrush SFICTimelineScrubber::DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex("030303"));
FSlateColorBrush SFICTimelineScrubber::DefaultAnimationBrush = FSlateColorBrush(FColor::FromHex("050505"));
FLinearColor SFICTimelineScrubber::DefaultIncrementColor = FLinearColor(FColor::FromHex("404040"));
FLinearColor SFICTimelineScrubber::DefaultFrameColor = FLinearColor(FColor::FromHex("FF8500"));

void SFICTimelineScrubber::Construct(const FArguments& InArgs) {
	FullRange = InArgs._FullRange;
	ActiveRange = InArgs._ActiveRange;
	ActiveFrame = InArgs._ActiveFrame;

	OnActiveFrameChanged = InArgs._OnActiveFrameChanged;
	
	BackgroundBrush = InArgs._BackgroundBrush;
	IncrementColor = InArgs._IncrementColor;
	FrameColor = InArgs._FrameColor;
	AnimationBrush = InArgs._AnimationBrush;
}

int32 SFICTimelineScrubber::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	FFICFrameRange Full = FullRange.Get();
	FFICFrameRange Active = ActiveRange.Get();
	
	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), BackgroundBrush.Get(), ESlateDrawEffect::None, BackgroundBrush.Get()->TintColor.GetSpecifiedColor());
	FVector2D AnimationLocalStart = FVector2D(FrameToLocalPos(Full.Begin), 0);
	FVector2D AnimationLocalEnd = FVector2D(FrameToLocalPos(Full.End), AllottedGeometry.GetLocalSize().Y);
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(AnimationLocalEnd - AnimationLocalStart, FSlateLayoutTransform(AnimationLocalStart)), AnimationBrush.Get(), ESlateDrawEffect::None, AnimationBrush.Get()->TintColor.GetSpecifiedColor());

	// Draw Range Increments
	FICFrame Increment = 10;
	while (Active.Length() / Increment > 30) Increment *= 10;
	Active.Begin += Increment - Active.Begin % Increment;
	if (Active.Begin <= 0) Active.Begin -= Increment;
	FLinearColor IncColor = IncrementColor.Get();
	int SafetyIncrement = 0;
	for (int64 i = Active.Begin; i <= Active.End; i += Increment) {
		if (++SafetyIncrement > 100) {
			UE_LOG(LogFicsItCam, Error, TEXT("Safety Increment of TimelineScrubber reached!"));
			break;
		}
		float IX = FrameToLocalPos(i);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(IX, 0), FVector2D(IX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  IncColor, true, 5);
	}
	++LayerId;

	// Draw Highlight
	float FrameX = FrameToLocalPos(GetActiveFrame());
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId+1, AllottedGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(FrameX, 0), FVector2D(FrameX, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None,  FrameColor.Get(), true, 5);

	return LayerId;
}

FVector2D SFICTimelineScrubber::ComputeDesiredSize(float) const {
	return FVector2D(30, 30);
}

FReply SFICTimelineScrubber::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled().BeginDragDrop(MakeShared<FFICTimelineScrubberDragDrop>(MouseEvent, SharedThis(this)));
}

FCursorReply SFICTimelineScrubber::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const {
	FICFrame Frame = LocalPosToFrame(MyGeometry.AbsoluteToLocal(CursorEvent.GetScreenSpacePosition()).X);
	if (GetActiveFrame() == Frame) {
		return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
	} else {
		return FCursorReply::Cursor(EMouseCursor::Default);
	}
}

FReply SFICTimelineScrubber::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	float Delta = MouseEvent.GetWheelDelta();
	if (!MouseEvent.GetModifierKeys().IsControlDown()) {
		int64 Range = ActiveRange.Get().Length();
		while (Range > 300) {
			Range /= 10;
			Delta *= 10;
		}
	}
	SetActiveFrame(GetActiveFrame() + Delta);
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

float SFICTimelineScrubber::FrameToLocalPos(FICFrame InFrame) const {
	const FFICFrameRange Range = ActiveRange.Get();
	return (GetCachedGeometry().GetLocalSize() * FVector2D(FMath::GetRangePct<float>((float)Range.Begin, (float)Range.End, InFrame), 0)).X;
}

FICFrame SFICTimelineScrubber::LocalPosToFrame(float LocalPos) const {
	const FFICFrameRange Range = ActiveRange.Get();
	return FMath::RoundToFloat(FMath::Lerp((float)Range.Begin, (float)Range.End, FMath::GetRangePct(0.0, GetCachedGeometry().GetLocalSize().X, LocalPos)));
}

void SFICTimelineScrubber::SetActiveFrame(FICFrame InFrame) {
	const FFICFrameRange Range = ActiveRange.Get();
	InFrame = FMath::Clamp(InFrame, Range.Begin, Range.End);
	const FICFrame Frame = ActiveFrame.Get();
	if (Frame == InFrame) return;
	bool _ = OnActiveFrameChanged.ExecuteIfBound(InFrame);
}

FICFrame SFICTimelineScrubber::GetActiveFrame() const {
	return ActiveFrame.Get();
}

void FFICTimelineScrubberDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FICFrame Frame = TimelineScrubber->LocalPosToFrame(TimelineScrubber->GetCachedGeometry().AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition()).X);
	TimelineScrubber->SetActiveFrame(Frame);
}
