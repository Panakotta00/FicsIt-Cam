#include "Editor/UI/FICSequencerRow.h"

#include "FicsItCamModule.h"
#include "Async/ParallelFor.h"
#include "Editor/UI/FICDragDrop.h"
#include "Editor/UI/FICKeyframeIcon.h"
#include "Editor/UI/FICSequencer.h"
#include "Editor/UI/FICUIUtil.h"

TArray<TSharedPtr<FFICSequencerRowMeta>> FFICSequencerRowMeta::GetChildren() {
	if (!CachedChildren.IsSet()) {
		CachedChildren = Provider->GetChildRows();
	}

	return *CachedChildren;
}

void FFICSequencerRowMeta::ClearCachedChildren() {
	CachedChildren.Reset();
}

void SFICSequencerRow::Construct(const FArguments& InArgs, SFICSequencer* InSequencer) {
	Sequencer = InSequencer;
	Context = Sequencer->Context;
	
	Style = InArgs._Style;
	if (!Style) Style = &FFICSequencerStyle::GetDefault();
	BackgroundColor = InArgs._BackgroundColor;
}

SFICSequencerRow::SFICSequencerRow() {
	Clipping = EWidgetClipping::ClipToBoundsAlways;
}

int32 SFICSequencerRow::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	const double StartSeconds = FPlatformTime::Seconds();

	OutDrawElements.PushClip(FSlateClippingZone(MyCullingRect));
	
	const FSlateBrush* BackgroundBrush;
	FLinearColor Color;
	GetRowBrushAndColor(Sequencer->GetRowIndexByWidget(ConstCastSharedRef<SFICSequencerRow>(SharedThis(this))), BackgroundColor, &Style->RowBackgroundEven, &Style->RowBackgroundOdd, InWidgetStyle, BackgroundBrush, Color);
	
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), BackgroundBrush, ESlateDrawEffect::None, Color);

	
	LayerId = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	
	OutDrawElements.PopClip();

	const double ElapsedMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
	if (ElapsedMs > 100) UE_LOG(LogFicsItCam, Warning, TEXT("SFICSequencerRow::OnPaint took %.3f ms"), ElapsedMs);

	return LayerId + 20;
}

void SFICSequencerRow::UpdateFrameRange(FFICFrameRange InFrameRange) {
	FrameRange = InFrameRange;
}

void SFICSequencerRow::UpdateActiveFrame(FICFrame InFrame) {
	ActiveFrame = InFrame;
}

TSharedRef<SFICSequencer> SFICSequencerRow::GetSequencer() const {
	return SharedThis(Sequencer);
}

void SFICSequencerRow::GetRowBrushAndColor(int32 InIndex, const TAttribute<FLinearColor>& InColorAttribute, const FSlateBrush* InBrushEven, const FSlateBrush* InBrushOdd, const FWidgetStyle& InWidgetStyle, const FSlateBrush*& OutBrush, FLinearColor& OutColor) {
	OutBrush = (InIndex % 2 == 0) ? InBrushEven : InBrushOdd;

	OutColor = FLinearColor::White;
	FLinearColor Tint;
	
	if (InColorAttribute.IsSet() || InColorAttribute.IsBound()) {
		OutColor = InColorAttribute.Get();
		if (OutColor == FLinearColor::White) OutBrush = InBrushOdd;
	}

	Tint = OutBrush->GetTint(InWidgetStyle);
	OutColor.A = Tint.A;
}

void SFICSequencerRowAttribute::Construct(const FArguments& InArgs, SFICSequencer* InSequencer, TSharedRef<FFICEditorAttributeBase> InAttribute) {
	SFICSequencerRow::FArguments SuperArgs;
	SuperArgs._Style = InArgs._Style;
	SuperArgs._BackgroundColor = InArgs._BackgroundColor;
	SFICSequencerRow::Construct(SuperArgs, InSequencer);
	
	Attribute = InAttribute;
}

SFICSequencerRowAttribute::SFICSequencerRowAttribute() : Children(this) {
	//TestColor = FLinearColor::MakeRandomColor();
}

FVector2D SFICSequencerRowAttribute::ComputeDesiredSize(float) const {
	return FVector2D(1, 1);
}

int32 SFICSequencerRowAttribute::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	FICFrameFloat start = LocalToFrameF(0.0);
	FICFrameFloat end = LocalToFrameF(AllottedGeometry.GetLocalSize().X);
	float height = AllottedGeometry.GetLocalSize().Y;

	auto keyframes = Attribute->GetAttribute().GetKeyframes().Array();
	Algo::SortBy(keyframes, [](const auto& Keyframe) { return Keyframe.Key; });

	for (const auto& [frame, keyframe] : keyframes) {
		if (frame < start || frame > end) continue;

		float localX = FrameToLocal(frame) - height / 2.0f;

		const FSlateBrush* icon = Style->KeyframeIcon.Icons.BrushByKeyframeType(keyframe->GetType());

		bool bIsSelected = GetSequencer()->GetSelectionManager().IsKeyframeSelected(Attribute->GetAttribute(), frame);

		FSlateDrawElement::MakeBox(OutDrawElements, LayerId++,
			AllottedGeometry.ToPaintGeometry(FVector2D(localX, 0), FVector2D(height, height)),
			icon, ESlateDrawEffect::None, (bIsSelected ? Style->KeyframeIcon.SelectedColor : Style->KeyframeIcon.UnselectedColor).GetColor(InWidgetStyle));
	}

	return SFICSequencerRow::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

FChildren* SFICSequencerRowAttribute::GetChildren() {
	return &Children;
}

void SFICSequencerRowAttribute::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {}

FReply SFICSequencerRowAttribute::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FICFrame clickedFrame = LocalToFrame(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X);
	FICFrame maxDistance = FMath::Abs(LocalToFrame(0) - LocalToFrame(MyGeometry.GetLocalSize().Y))/2;
	TOptional<FICFrame> frame = Attribute->GetAttribute().GetClosestKeyframe(clickedFrame, maxDistance);
	ClickedFrame = frame;
	if (frame) {
		return FReply::Handled().DetectDrag(AsShared(), EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

FReply SFICSequencerRowAttribute::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	ClickedFrame = {};
	FSelectionManager& SelectionManager = GetSequencer()->GetSelectionManager();
	FICFrame clickedFrame = LocalToFrame(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X);
	FICFrame maxDistance = FMath::Abs(LocalToFrame(0) - LocalToFrame(MyGeometry.GetLocalSize().Y))/2;
	TOptional<FICFrame> frame = Attribute->GetAttribute().GetClosestKeyframe(clickedFrame, maxDistance);
	if (frame && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		SelectionManager.ToggleKeyframeSelection(Attribute->GetAttribute(), *frame, &MouseEvent.GetModifierKeys());
		return FReply::Handled();
	} else if (frame && MouseEvent.GetEffectingButton() == EKeys::RightMouseButton) {
		TPair<FFICAttribute*, FICFrame> KF_Selection(GetAttribute(), *frame);
		if (!SelectionManager.GetSelection().Contains(KF_Selection)) {
			SelectionManager.SetSelection({KF_Selection});
		}
		FMenuBuilder MenuBuilder = FICCreateKeyframeTypeChangeMenu(GetSequencer()->Context, [this] {
			return GetSequencer()->GetSelectionManager().GetSelection();
		});
		FSlateApplication::Get().PushMenu(SharedThis(this), *MouseEvent.GetEventPath(), MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
		return FReply::Handled();
	}
	return SFICSequencerRow::OnMouseButtonUp(MyGeometry, MouseEvent);
}

FReply SFICSequencerRowAttribute::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		if (ClickedFrame) {
			FICFrame frame = *ClickedFrame;
			ClickedFrame = {};
			if (!GetSequencer()->GetSelectionManager().IsKeyframeSelected(Attribute->GetAttribute(), frame)) GetSequencer()->GetSelectionManager().SetSelection({TPair<FFICAttribute*, FICFrame>(&Attribute->GetAttribute(), frame)});
			return FReply::Handled().BeginDragDrop(MakeShared<FFICSequencerKeyframeDragDrop>(GetSequencer(), MouseEvent));
		}
	}
	return FReply::Unhandled();
}

void SFICSequencerRowAttribute::UpdateFrameRange(FFICFrameRange InFrameRange) {
	SFICSequencerRow::UpdateFrameRange(InFrameRange);
}

TArray<TTuple<FFICAttribute&, FICFrame>> SFICSequencerRowAttribute::GetKeyframesInBox(const FBox2D& InBox) {
	TArray<TTuple<FFICAttribute&, FICFrame>> Keyframes;
	for (TTuple<FICFrame, TSharedRef<FFICKeyframe>> Keyframe : Attribute->GetAttribute().GetKeyframes()) {
		FGeometry Geometry = GetCachedGeometry();
		FGeometry SeqGeometry = Sequencer->GetCachedGeometry();
		bool isInBox = true;
		FBox2D Box = InBox;
		Box.Min = SeqGeometry.LocalToAbsolute(Box.Min);
		Box.Max = SeqGeometry.LocalToAbsolute(Box.Max);
		isInBox = isInBox && LocalToFrameF(Geometry.AbsoluteToLocal(Box.Min).X) <= Keyframe.Key;
		isInBox = isInBox && LocalToFrameF(Geometry.AbsoluteToLocal(Box.Max).X) >= Keyframe.Key;
		isInBox = isInBox && Box.Min.Y <= Geometry.GetAbsolutePositionAtCoordinates(FVector2D(1.0)).Y;
		isInBox = isInBox && Box.Max.Y >= Geometry.GetAbsolutePositionAtCoordinates(FVector2D(0.0)).Y;
		if (isInBox) {
			Keyframes.Add(TTuple<FFICAttribute&, FICFrame>(Attribute->GetAttribute(), Keyframe.Key));
		}
	}
	return Keyframes;
}

FFICAttribute* SFICSequencerRowAttribute::GetAttribute() const {
	return &Attribute->GetAttribute();
}

FICFrame SFICSequencerRowAttribute::LocalToFrame(float Local) const {
	return FMath::RoundToInt(LocalToFrameF(Local));
}

double SFICSequencerRowAttribute::LocalToFrameF(float Local) const {
	FFICFrameRange Frames = FrameRange;
	return (int64) FMath::Lerp(
		(double)Frames.Begin,
		(double)Frames.End,
		Local / GetCachedGeometry().GetLocalSize().X);
}

float SFICSequencerRowAttribute::FrameToLocal(FICFrame InFrame) const {
	FFICFrameRange Frames = FrameRange;
	return FMath::Lerp(
		0.0f,
		GetCachedGeometry().GetLocalSize().X,
		FMath::GetRangePct(
			(double)Frames.Begin,
			(double)Frames.End,
			(double)InFrame));
}

float SFICSequencerRowAttribute::GetFramePerLocal() const {
	return FrameRange.Length() / GetCachedGeometry().Size.X;
}
