#include "Editor/UI/FICSequencerRow.h"

#include "Editor/UI/FICDragDrop.h"
#include "Editor/UI/FICKeyframeIcon.h"
#include "Editor/UI/FICSequencer.h"

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
	OutDrawElements.PushClip(FSlateClippingZone(MyCullingRect));
	
	const FSlateBrush* BackgroundBrush;
	FLinearColor Color;
	GetRowBrushAndColor(Sequencer->GetRowIndexByWidget(ConstCastSharedRef<SFICSequencerRow>(SharedThis(this))), BackgroundColor, &Style->RowBackgroundEven, &Style->RowBackgroundOdd, InWidgetStyle, BackgroundBrush, Color);
	
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), BackgroundBrush, ESlateDrawEffect::None, Color);

	
	LayerId = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	
	OutDrawElements.PopClip();

	return LayerId + 20;
}

void SFICSequencerRow::UpdateFrameRange(FFICFrameRange InFrameRange) {
	FrameRange = InFrameRange;
}

void SFICSequencerRow::UpdateActiveFrame(FICFrame InFrame) {
	ActiveFrame = InFrame;
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

void SFICSequencerRowAttributeKeyframe::Construct(const FArguments& InArgs, SFICSequencerRowAttribute* InRowAttribute, UFICEditorContext* InContext, FFICAttribute* InAttribute, FICFrame InFrame) {
	RowAttribute = InRowAttribute;
	Context = InContext;
	Attribute = InAttribute;
	Frame = InFrame;

	Style = InArgs._Style;
	if (!Style) Style = &FFICSequencerStyle::GetDefault();
	ActiveFrame = InArgs._Frame;
	FrameRange = InArgs._FrameRange;

	ChildSlot[
		SNew(SFICKeyframeIcon)
		.Style(&Style->KeyframeIcon)
		.Keyframe_Lambda([this]() {
			TSharedPtr<FFICKeyframe> Keyframe;
			TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = Attribute->GetKeyframes();
			if (const TSharedRef<FFICKeyframe>* KeyframePtr = Keyframes.Find(Frame)) Keyframe = *KeyframePtr;
			return Keyframe;
		})
		.IsSelected_Lambda([this]() {
			return false;
		})
	];
}

FReply SFICSequencerRowAttributeKeyframe::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Unhandled().DetectDrag(AsShared(), EKeys::LeftMouseButton);
}

FReply SFICSequencerRowAttributeKeyframe::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return SCompoundWidget::OnMouseButtonUp(MyGeometry, MouseEvent);
}

FReply SFICSequencerRowAttributeKeyframe::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled().BeginDragDrop(MakeShared<FFICSequencerKeyframeDragDrop>(SharedThis(RowAttribute), SharedThis(this), MouseEvent));
}

void SFICSequencerRowAttribute::Construct(const FArguments& InArgs, SFICSequencer* InSequencer, TSharedRef<FFICEditorAttributeBase> InAttribute) {
	SFICSequencerRow::FArguments SuperArgs;
	SuperArgs._Style = InArgs._Style;
	SuperArgs._BackgroundColor = InArgs._BackgroundColor;
	SFICSequencerRow::Construct(SuperArgs, InSequencer);
	
	Attribute = InAttribute;
	
	DelegateHandle = Attribute->GetAttribute().OnUpdate.AddSP(SharedThis(this), &SFICSequencerRowAttribute::UpdateKeyframes);

	UpdateKeyframes();
}

SFICSequencerRowAttribute::SFICSequencerRowAttribute() : Children(this) {
	//TestColor = FLinearColor::MakeRandomColor();
}

SFICSequencerRowAttribute::~SFICSequencerRowAttribute() {
	Attribute->GetAttribute().OnUpdate.Remove(DelegateHandle);
}

FVector2D SFICSequencerRowAttribute::ComputeDesiredSize(float) const {
	return FVector2D(1, 1);
}

int32 SFICSequencerRowAttribute::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	return SFICSequencerRow::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

FChildren* SFICSequencerRowAttribute::GetChildren() {
	return &Children;
}

void SFICSequencerRowAttribute::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	for (int i = 0; i < Children.Num(); ++i) {
		const TSharedRef<SFICSequencerRowAttributeKeyframe>& Child = Children[i];
		FVector2D Size = Child->GetDesiredSize();
		FVector2D Offset = -Child->GetDesiredSize() / 2.0f;
		Offset.Y += AllottedGeometry.Size.Y / 2.0f;
		Offset.X += FrameToLocal(Child->GetFrame());
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Child, Offset, Size));
	}
}

void SFICSequencerRowAttribute::UpdateKeyframes() {
	Children.Empty();

	for (TTuple<FICFrame, TSharedRef<FFICKeyframe>> Keyframe : Attribute->GetAttribute().GetKeyframes()) {
		Children.Add(
			SNew(SFICSequencerRowAttributeKeyframe, this, Context, &Attribute->GetAttribute(), Keyframe.Key)
			.Style(Style)
			.Frame_Lambda([this]() {
				return ActiveFrame;
			})
		);
	}
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
