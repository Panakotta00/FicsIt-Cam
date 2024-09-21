#include "Editor/UI/FICSequencer.h"

#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICDragDrop.h"
#include "Editor/UI/FICEditorStyle.h"
#include "Editor/UI/FICSequencerTreeView.h"

void SFICSequencer::Construct(const FArguments& InArgs, UFICEditorContext* InContext, SFICSequencerTreeView* InTreeView) {
	Context = InContext;
	TreeView = InTreeView;

	Style = InArgs._Style;
	ActiveFrame = InArgs._Frame;
	FrameRange = InArgs._FrameRange;
	FrameHighlightRange = InArgs._FrameHighlightRange;
		
	ActiveFrameDelegate = Context->OnCurrentFrameChanged.AddRaw(this, &SFICSequencer::ActiveFrameChanged);

	UpdateRows();
}

SFICSequencer::SFICSequencer() : Children(this) {
	Clipping = EWidgetClipping::ClipToBoundsAlways;

	SelectionManager.OnSelectionChanged.BindLambda([this]() {
		Invalidate(EInvalidateWidgetReason::Layout);
	});
	SelectionManager.OnHandleBoxSelection.BindLambda([this](const FBox2D& InBox, const FModifierKeysState& InModifiers) {
		for (TSharedPtr<ITableRow> Row : LinearRows) {
			TSharedPtr<FFICSequencerRowMeta> RowMeta = *TreeView->ItemFromWidget(Row.Get());
			TSharedPtr<SFICSequencerRow> Widget = MetaToWidget[RowMeta];
			
			FGeometry Geometry = Widget->GetCachedGeometry();
			for (const TTuple<FFICAttribute&, FICFrame>& Keyframe : Widget->GetKeyframesInBox(InBox)) {
				SelectionManager.ToggleKeyframeSelection(Keyframe.Get<0>(), Keyframe.Get<1>(), &InModifiers);
			}
		}
	});
}

SFICSequencer::~SFICSequencer() {
	Context->OnCurrentFrameChanged.Remove(ActiveFrameDelegate);
}

void SFICSequencer::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	if (OldFrameRange != FrameRange.Get()) {
		OldFrameRange = FrameRange.Get();
		FrameRangeChanged();
	}
	
	SPanel::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

FVector2D SFICSequencer::ComputeDesiredSize(float) const {
	return FVector2D(10, 10);
}

int32 SFICSequencer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	OutDrawElements.PushClip(FSlateClippingZone(AllottedGeometry));
	
	FFICFrameRange Range = FrameRange.Get();
	FFICFrameRange Highlight = FrameHighlightRange.Get();
		
	// Draw Highlighted Frame Range Background
	FVector2D AnimationLocalStart = FVector2D(FrameToLocal(Highlight.Begin), 0);
	FVector2D AnimationLocalEnd = FVector2D(FrameToLocal(Highlight.End), AllottedGeometry.GetLocalSize().Y);
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(AnimationLocalEnd - AnimationLocalStart, FSlateLayoutTransform(AnimationLocalStart)), &Style->HighlightRangeBrush, ESlateDrawEffect::None, Style->HighlightRangeBrush.TintColor.GetSpecifiedColor());
	
	// Draw Grid
	FVector2D Distance = FVector2D(10,10);
	FVector2D Start = FVector2D(LocalToFrame(0), 0) / Distance;
	FVector2D RenderOffset = FVector2D(FMath::Fractional(Start.X) * Distance.X, FMath::Fractional(Start.Y) * Distance.Y);
	int64 Steps = 1;
	int64 SafetyCounter = 0;
	while (Range.Length() / Steps > 30) Steps *= 10;
	for (float x = Range.Begin - Range.Begin % Steps; x <= Range.End; x += Steps) {
		if (SafetyCounter++ > 1000) break;
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(FrameToLocal(x), 0), FVector2D(FrameToLocal(x), AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None, Style->GridColor, true, 1);
	}
		
	// Draw Active Frame
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), {FVector2D(FrameToLocal(ActiveFrame.Get()), 0), FVector2D(FrameToLocal(ActiveFrame.Get()), AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None, Style->ActiveFrameColor, true, 2);

	// Draw Box Selection
	FBox2D BoxSelection = SelectionManager.GetSelectionBox();
	if (BoxSelection.bIsValid) {
		float BeginTime = BoxSelection.Min.X;
		float EndTime = BoxSelection.Max.X;
		float BeginValue = BoxSelection.Min.Y;
		float EndValue = BoxSelection.Max.Y;
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(FVector2D(EndTime - BeginTime, EndValue - BeginValue), FSlateLayoutTransform(FVector2D(BeginTime, BeginValue))), &Style->SelectionBoxBrush, ESlateDrawEffect::None, Style->SelectionBoxBrush.TintColor.GetSpecifiedColor());
	}
	
	LayerId = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	OutDrawElements.PopClip();

	return LayerId+20;
}

FReply SFICSequencer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled().DetectDrag(AsShared(), EKeys::RightMouseButton);
}

FReply SFICSequencer::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return SPanel::OnMouseButtonUp(MyGeometry, MouseEvent);
}

FReply SFICSequencer::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
	return SPanel::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FReply SFICSequencer::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) {
		return FReply::Handled().BeginDragDrop(MakeShared<FFICSequencerPanDragDrop>(SharedThis(this), MouseEvent));
	} else if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		return FReply::Handled().BeginDragDrop(MakeShared<FFICSequencerSelectionDragDrop>(SharedThis(this), MouseEvent));
	}
	return FReply::Unhandled();
}

FReply SFICSequencer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return SPanel::OnMouseMove(MyGeometry, MouseEvent);
}

FReply SFICSequencer::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FFICFrameRange Range = FrameRange.Get();
	if (MouseEvent.IsControlDown()) {
		FICFrameFloat Delta = MouseEvent.GetWheelDelta() / 10.0f;
		FICFrameFloat CursorFrame = LocalToFrameF(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X);
		Context->SetActiveRange(FFICFrameRange(Range.Begin + Delta * (CursorFrame - Range.Begin), Range.End + Delta * (CursorFrame - Range.End)));
	} else if (MouseEvent.IsShiftDown()) {
		FICFrameFloat Delta = MouseEvent.GetWheelDelta() * (Range.End - Range.Begin) / 10.0f;
		Context->SetActiveRange(Range + Delta);
	}
	return SPanel::OnMouseWheel(MyGeometry, MouseEvent);
}

FReply SFICSequencer::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Delete) {
		for (TTuple<FFICAttribute*, FICFrame> Keyframe : SelectionManager.GetSelection()) {
			Keyframe.Key->RemoveKeyframe(Keyframe.Value);
		}
		SelectionManager.SetSelection({});
		return FReply::Handled();
	} else if (InKeyEvent.GetKey() == EKeys::C && InKeyEvent.GetModifierKeys().IsControlDown()) {
		CopiedKeyframes.Empty();
		CopiedKeyframesSceneObject = FSoftObjectPtr(Context->GetSelectedSceneObject());
		if (CopiedKeyframesSceneObject.IsValid()) {
			FICFrame MaxFrame = TNumericLimits<FICFrame>::Min();
			FICFrame MinFrame = TNumericLimits<FICFrame>::Max();
			for (TTuple<FFICAttribute*, FICFrame> Keyframe : SelectionManager.GetSelection()) {
				MaxFrame = FMath::Max(MaxFrame, Keyframe.Value);
				MinFrame = FMath::Min(MinFrame, Keyframe.Value);
				TSharedRef<FFICKeyframe> KF = Keyframe.Key->GetKeyframes()[Keyframe.Value];
				CopiedKeyframes.Add(TTuple<FICFrame, FFICAttribute*, FFICKeyframeData>{Keyframe.Value, Keyframe.Key, KF->GetKeyframeData()});
			}
			FICFrame Offset = MinFrame + (MaxFrame - MinFrame)/2;
			for (TTuple<FICFrame, FFICAttribute*, FFICKeyframeData>& Keyframe : CopiedKeyframes) {
				Keyframe.Get<0>() -= Offset;
			}
		}
		return FReply::Handled();
	} else if (InKeyEvent.GetKey() == EKeys::V && InKeyEvent.GetModifierKeys().IsControlDown()) {
		UObject* CopiedKeyframesFromSceneObject = CopiedKeyframesSceneObject.Get();
		if (CopiedKeyframesFromSceneObject && CopiedKeyframesFromSceneObject == Context->GetSelectedSceneObject()) {
			FICFrame Offset = Context->GetCurrentFrame();
			SelectionManager.SetSelection({});
			for (const TTuple<FICFrame, FFICAttribute*, FFICKeyframeData>& Keyframe : CopiedKeyframes) {
				FICFrame Time = Keyframe.Get<0>() + Offset;
				TSharedRef<FFICKeyframe> KF = Keyframe.Get<1>()->AddKeyframe(Time);
				KF->SetKeyframeData(Keyframe.Get<2>());
				SelectionManager.AddKeyframeToSelection(*Keyframe.Get<1>(), Time , false);
			}
			auto _ = SelectionManager.OnSelectionChanged.ExecuteIfBound();
		}
		return FReply::Handled();
	}
	return SPanel::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SFICSequencer::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	return SPanel::OnKeyUp(MyGeometry, InKeyEvent);
}

bool SFICSequencer::IsInteractable() const {
	return SPanel::IsInteractable();
}

FChildren* SFICSequencer::GetChildren() {
	return &Children;
}

void SFICSequencer::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	for (int32 i = 0; i < Children.Num(); ++i) {
		const TSharedPtr<SFICSequencerRow>& Child = Children[i];
		const TSharedPtr<ITableRow>& Row = LinearRows[i];
		FGeometry TableRowGeometry = Row->AsWidget()->GetPaintSpaceGeometry();
		FVector2D Size = FVector2D(AllottedGeometry.GetLocalSize().X, TableRowGeometry.GetLocalSize().Y);
		FVector2D ParentOffset = FVector2D(0, AllottedGeometry.AbsoluteToLocal(TableRowGeometry.GetAbsolutePosition()).Y);
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Child.ToSharedRef(), ParentOffset, Size, 1));
	}
}

FSelectionManager& SFICSequencer::GetSelectionManager() {
	return SelectionManager;
}

const FSelectionManager& SFICSequencer::GetSelectionManager() const {
	return SelectionManager;
}

void SFICSequencer::UpdateRows() {
	TArray<TSharedPtr<ITableRow>> Rows = TreeView->GetVisibleTableRows();

	TSet<int32> ToRemove;
	int32 i = 0;
	for (; i < Rows.Num(); ++i) {
		const TSharedPtr<ITableRow>& Row = Rows[i];
		TSharedPtr<FFICSequencerRowMeta> Meta = *TreeView->ItemFromWidget(Row.Get());
		
		// Skip if Item already exists
		if (LinearRows.Num() > i && LinearRows[i] == Row) continue;

		if (LinearRows.Contains(Row)) {
			// If Current Item already exists, but was not at same index -> Item to delete in LinearRows
			while (LinearRows[i] != Row) {
				TSharedPtr<SFICSequencerRow> Widget = Children[i];
				Children.RemoveAt(i);
				LinearRows.RemoveAt(i);
				MetaToWidget.Remove(WidgetToMeta[Widget]);
				WidgetToMeta.Remove(Widget);
			}
		} else {
			// If Current Item does not exist -> Create and Add Item
			TSharedPtr<SFICSequencerRow> Widget = Meta->Provider->CreateRow(this);
			Children.Insert(Widget.ToSharedRef(), i);
			LinearRows.Insert(Row, i);
			MetaToWidget.Add(Meta, Widget);
			WidgetToMeta.Add(Widget, Meta);

			Widget->UpdateActiveFrame(ActiveFrame.Get());
			Widget->UpdateFrameRange(FrameRange.Get());
		}
	}
	for (; i < LinearRows.Num();) {
		TSharedPtr<SFICSequencerRow> Widget = Children[i];
		Children.RemoveAt(i);
		LinearRows.RemoveAt(i);
		MetaToWidget.Remove(WidgetToMeta[Widget]);
		WidgetToMeta.Remove(Widget);
	}

	Invalidate(EInvalidateWidgetReason::Layout);
}

int32 SFICSequencer::GetRowIndexByWidget(TSharedRef<SFICSequencerRow> InWidget) {
	return TreeView->GetRowIndex(WidgetToMeta[InWidget]);
}

FICFrame SFICSequencer::LocalToFrame(float Local) const {
	return FMath::RoundToInt(LocalToFrameF(Local));
}


double SFICSequencer::LocalToFrameF(float Local) const {
	FFICFrameRange Frames = FrameRange.Get();
	return (int64) FMath::Lerp(
		(double)Frames.Begin,
		(double)Frames.End,
		Local / GetCachedGeometry().GetLocalSize().X);
}

float SFICSequencer::FrameToLocal(FICFrame InFrame) const {
	FFICFrameRange Frames = FrameRange.Get();
	return FMath::Lerp(
		0.0f,
		GetCachedGeometry().GetLocalSize().X,
		FMath::GetRangePct(
			(double)Frames.Begin,
			(double)Frames.End,
			(double)InFrame));
}

float SFICSequencer::GetFramePerLocal() const {
	return FrameRange.Get().Length() / GetCachedGeometry().Size.X;
}

void SFICSequencer::FrameRangeChanged() {
	for (int i = 0; i < Children.Num(); ++i) {
		Children[i]->UpdateFrameRange(FrameRange.Get());
	}
}

void SFICSequencer::ActiveFrameChanged() {
	for (int i = 0; i < Children.Num(); ++i) {
		Children[i]->UpdateActiveFrame(ActiveFrame.Get());
	}
}


