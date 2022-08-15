#include "Editor/UI/FICGraphView.h"

#include "Editor/FICChangeList.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICDragDrop.h"

const FName FFICGraphViewStyle::TypeName = TEXT("FFICGraphViewStyle");

const FFICGraphViewStyle& FFICGraphViewStyle::GetDefault() {
	static FFICGraphViewStyle* Default = nullptr;
	if (!Default) {
		Default = new FFICGraphViewStyle();
		*Default = FFICEditorStyles::Get().GetWidgetStyle<FFICGraphViewStyle>("GraphView");
		Default->NumericKeyframeIcons = FFICNumericKeyframeIcons::GetDefault();
	}
	return *Default;
}

void SFICGraphViewKeyframeHandle::Construct(const FArguments& InArgs, SFICGraphViewKeyframe* InKeyframe) {
	GraphKeyframe = InKeyframe;
	
	bIsOutHandle = InArgs._IsOutHandle;
	Style = InArgs._Style;

	ChildSlot[
		SNew(SBox)
		.HeightOverride(20)
		.WidthOverride(20)
		.Padding(5)[
			SNew(SImage)
			.Image_Lambda([this]() {
				return &Style->NumericKeyframeIcons.HandleBrush;
			})
			.ColorAndOpacity_Lambda([this]() {
				return Style->KeyframeSelectedColor.GetSpecifiedColor();
			})
		]
	];
}

FReply SFICGraphViewKeyframeHandle::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled().DetectDrag(AsShared(), EKeys::LeftMouseButton);
}

FReply SFICGraphViewKeyframeHandle::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		return FReply::Handled().BeginDragDrop(MakeShared<FFICGraphKeyframeHandleDragDrop>(SharedThis(this), MouseEvent));
	}
	return FReply::Unhandled();
}

FCursorReply SFICGraphViewKeyframeHandle::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const {
	return FCursorReply::Cursor(EMouseCursor::GrabHand);
}

SFICGraphViewKeyframe::SFICGraphViewKeyframe() : Children(this) {}

void SFICGraphViewKeyframe::Construct(const FArguments& InArgs, SFICGraphView* InGraphView, FFICAttribute* InAttribute, FICFrame InFrame) {
	GraphView = InGraphView;
	Attribute = InAttribute;
	Frame = InFrame;
	Style = InArgs._Style;
	
	Children.Add(
		SAssignNew(MainHandle, SBox)
		.ToolTipText_Lambda([this]() {
			if (GetKeyframe()) {
				switch (GetKeyframe()->KeyframeType) {
				case FIC_KF_EASE:
					return FText::FromString("Ease");
				case FIC_KF_EASEINOUT:
					return FText::FromString("Ease-In/Out");
				case FIC_KF_LINEAR:
					return FText::FromString("Linear");
				case FIC_KF_STEP:
					return FText::FromString("Step");
				default:
					break;
				}
			}
			return FText();
		})
		.Content()[
			SNew(SBox)
			.Padding(2)
			.WidthOverride(20)
			.HeightOverride(20)
			.Content()[
				SNew(SImage)
				.ColorAndOpacity_Lambda([this]() {
					TSharedPtr<FFICKeyframe> KF = GetKeyframe();
					if (GraphView->IsKeyframeSelected(*Attribute, Frame))return Style->KeyframeSelectedColor;
					return Style->KeyframeUnselectedColor;
				})
				.Image_Lambda([this]() {
					if (!GetKeyframe()) {
						return &Style->NumericKeyframeIcons.DefaultBrush;
					}
					switch (GetKeyframe()->GetType()) {
					case FIC_KF_EASE:
						return &Style->NumericKeyframeIcons.AutoBrush;
					case FIC_KF_EASEINOUT:
						return &Style->NumericKeyframeIcons.EaseInOutBrush;
					case FIC_KF_MIRROR:
						return &Style->NumericKeyframeIcons.MirrorBrush;
					case FIC_KF_CUSTOM:
						return &Style->NumericKeyframeIcons.CustomBrush;
					case FIC_KF_LINEAR:
						return &Style->NumericKeyframeIcons.LinearBrush;
					case FIC_KF_STEP:
						return &Style->NumericKeyframeIcons.StepBrush;
					default:
						return &Style->NumericKeyframeIcons.DefaultBrush;
					}
				})
			]
		]);

	int64 _;
	TSharedPtr<FFICKeyframe> PrevKeyframe = Attribute->GetPrevKeyframe(Frame, _);
	if (PrevKeyframe && PrevKeyframe->KeyframeType & FIC_KF_HANDLES) {
		Children.Add(
			SAssignNew(InHandle, SFICGraphViewKeyframeHandle, this)
			.Style(Style)
			.Visibility_Lambda([this]() {
				return GraphView->IsKeyframeSelected(*Attribute, GetFrame()) ? EVisibility::All : EVisibility::Collapsed;
			}));
	}
	if (GetKeyframe() && GetKeyframe()->KeyframeType & FIC_KF_HANDLES) {
		Children.Add(
			SAssignNew(OutHandle, SFICGraphViewKeyframeHandle, this)
			.IsOutHandle(true)
			.Style(Style)
			.Visibility_Lambda([this]() {
				return GraphView->IsKeyframeSelected(*Attribute, GetFrame()) ? EVisibility::All : EVisibility::Collapsed;
			}));
	}
}

int SFICGraphViewKeyframe::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	if (GetKeyframe() && GraphView->IsKeyframeSelected(*Attribute, GetFrame())) {
		FFICValueTimeFloat InControl = GetKeyframe()->GetInControl();
		FFICValueTimeFloat OutControl = GetKeyframe()->GetOutControl();
		TArray<FVector2D> PlotPoints;
		if (InHandle) PlotPoints.Add(FVector2D((1.0 / GraphView->GetFramePerLocal()) * -InControl.Frame, (1.0 / GraphView->GetValuePerLocal()) * InControl.Value));
		PlotPoints.Add(FVector2D(0, 0));
		if (OutHandle) PlotPoints.Add(FVector2D((1.0 / GraphView->GetFramePerLocal()) * OutControl.Frame, (1.0 / GraphView->GetValuePerLocal()) * -OutControl.Value));
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId+5, AllottedGeometry.ToPaintGeometry(), PlotPoints, ESlateDrawEffect::None, Style->KeyframeSelectedColor.GetSpecifiedColor(), true, 2);
	}
	
	int NewLayerId = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId+10, InWidgetStyle, bParentEnabled);

	return NewLayerId+10;
}

FVector2D SFICGraphViewKeyframe::ComputeDesiredSize(float) const {
	return MainHandle->GetDesiredSize();
}

FChildren* SFICGraphViewKeyframe::GetChildren() {
	return &Children;
}

void SFICGraphViewKeyframe::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(MainHandle.ToSharedRef(), -MainHandle->GetDesiredSize()/2.0f, MainHandle->GetDesiredSize(), 1));

	FVector2D OffsetAdjust = FVector2D(GraphView->GetFramePerLocal(), GraphView->GetValuePerLocal());
	
	if (GetKeyframe()) {
		if (InHandle) {
			FFICValueTimeFloat InControl = GetKeyframe()->GetInControl();
			FVector2D FromHandleOffset(-InControl.Frame, InControl.Value);
			FromHandleOffset /= OffsetAdjust;
			FromHandleOffset -= InHandle->GetDesiredSize()/2.0f;
			ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(InHandle.ToSharedRef(), FromHandleOffset, InHandle->GetDesiredSize(), 1));
		}

		if (OutHandle) {
			FFICValueTimeFloat OutControl = GetKeyframe()->GetOutControl();
			FVector2D ToHandleOffset(OutControl.Frame, -OutControl.Value);
			ToHandleOffset /= OffsetAdjust;
			ToHandleOffset -= OutHandle->GetDesiredSize()/2.0f;
			ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(OutHandle.ToSharedRef(), ToHandleOffset, OutHandle->GetDesiredSize(), 1));
		}
	}
}

FReply SFICGraphViewKeyframe::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
}

FReply SFICGraphViewKeyframe::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		if (!GraphView->IsKeyframeSelected(*Attribute, GetFrame())) GraphView->SetSelection({TPair<FFICAttribute*, FICFrame>(Attribute, GetFrame())});
		return FReply::Handled().BeginDragDrop(MakeShared<FFICGraphKeyframeDragDrop>(SharedThis(GraphView), MouseEvent));
	}
	return FReply::Handled();
}

FReply SFICGraphViewKeyframe::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& Event) {
	if (Event.GetEffectingButton() == EKeys::LeftMouseButton) {
		GraphView->ToggleKeyframeSelection(*Attribute, GetFrame(), &Event.GetModifierKeys());
	} else	if (Event.GetEffectingButton() == EKeys::RightMouseButton) {
		TPair<FFICAttribute*, FICFrame> KF_Selection(&GetAttribute(), GetFrame());
		if (!GraphView->GetSelection().Contains(KF_Selection)) {
			GraphView->SetSelection({KF_Selection});
		}
		TSet<TPair<FFICAttribute*, FICFrame>> Keyframes = GraphView->GetSelection();
		TFunction<void(EFICKeyframeType)> SetKeyframeType;
		SetKeyframeType = [Keyframes, this](EFICKeyframeType Type) {
			TSharedRef<FFICChange_Group> Group = MakeShared<FFICChange_Group>();
			TMap<FFICAttribute*, TSharedRef<FFICAttribute>> Snapshots;
			for (const TPair<FFICAttribute*, FICFrame>& KF : Keyframes) {
				TSharedRef<FFICAttribute>* Snapshot = Snapshots.Find(KF.Key);
				if (!Snapshot) Snapshots.Add(KF.Key, KF.Key->Get());
				TSharedRef<FFICKeyframe>* NKF = KF.Key->GetKeyframes().Find(KF.Value);
				if (NKF) (*NKF)->SetType(Type);
				KF.Key->LockUpdateEvent();
				KF.Key->RecalculateAllKeyframes();
				KF.Key->UnlockUpdateEvent(false);
			}
			for (const TPair<FFICAttribute*, TSharedRef<FFICAttribute>>& Snapshot : Snapshots) {
				Group->PushChange(MakeShared<FFICChange_Attribute>(Snapshot.Key, Snapshot.Value));
			}
			GraphView->Context->ChangeList.PushChange(Group);
			for (const TPair<FFICAttribute*, FICFrame>& KF : Keyframes) KF.Key->OnUpdate.Broadcast();
		};
		
		TSharedPtr<IMenu> MenuHandle;
		FMenuBuilder MenuBuilder(true, NULL);
		MenuBuilder.AddMenuEntry(
            FText::FromString("Ease"),
            FText(),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([SetKeyframeType, this]() {
            	SetKeyframeType(FIC_KF_EASE);
            }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
		MenuBuilder.AddMenuEntry(
            FText::FromString("Ease-In/Out"),
            FText(),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([SetKeyframeType, this]() {
                SetKeyframeType(FIC_KF_EASEINOUT);
            }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
		MenuBuilder.AddMenuEntry(
            FText::FromString("Linear"),
            FText(),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([SetKeyframeType, this]() {
                SetKeyframeType(FIC_KF_LINEAR);
            }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
		MenuBuilder.AddMenuEntry(
            FText::FromString("Step"),
            FText(),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([SetKeyframeType, this]() {
                SetKeyframeType(FIC_KF_STEP);
            }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
	
		FSlateApplication::Get().PushMenu(SharedThis(this), *Event.GetEventPath(), MenuBuilder.MakeWidget(), Event.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
		return FReply::Handled();
	}
	return FReply::Handled();
}

UFICEditorContext* SFICGraphViewKeyframe::GetContext() {
	return GraphView->Context;
}

TSharedPtr<FFICKeyframe> SFICGraphViewKeyframe::GetKeyframe() const {
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = Attribute->GetKeyframes();
	TSharedRef<FFICKeyframe>* Keyframe = Keyframes.Find(Frame);
	if (!Keyframe) return nullptr;
	return *Keyframe;
}

void SFICGraphView::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Style = InArgs._Style;
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
	BoxSelection.bIsValid = false;
}

SFICGraphView::~SFICGraphView() {
	for (TSharedRef<FFICEditorAttributeBase> Attribute : Attributes) {
		Attribute->GetAttribute().OnUpdate.Remove(DelegateHandles[Attribute]);
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
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(AnimationLocalEnd - AnimationLocalStart, FSlateLayoutTransform(AnimationLocalStart)), &Style->HighlightRangeBrush, ESlateDrawEffect::None, Style->HighlightRangeBrush.TintColor.GetSpecifiedColor());
	
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

	// Draw Active Frame
	FLinearColor FrameColor = FLinearColor(FColor::FromHex("666600"));
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(FrameToLocal(ActiveFrame.Get()), 0), FVector2D(FrameToLocal(ActiveFrame.Get()), AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None, FrameColor, true, 2);
	
	// Draw Plots
	for (const TSharedRef<FFICEditorAttributeBase>& Attribute : Attributes) {
		TArray<FVector2D> PlotPoints;
		for (FICFrame PlotFrame : Range) {
			FVector2D PlotPoint = FrameAttributeToLocal(Attribute, PlotFrame);
			PlotPoints.Add(PlotPoint);
		}
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), PlotPoints, ESlateDrawEffect::None, Attribute->GraphColor, true, 2);
	}

	// Draw Box Selection
	if (BoxSelection.bIsValid) {
		float BeginTime = FrameToLocal(BoxSelection.Min.X);
		float EndTime = FrameToLocal(BoxSelection.Max.X);
		float BeginValue = ValueToLocal(BoxSelection.Min.Y);
		float EndValue = ValueToLocal(BoxSelection.Max.Y);
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(FVector2D(EndTime - BeginTime, EndValue - BeginValue), FSlateLayoutTransform(FVector2D(BeginTime, BeginValue))), &Style->SelectionBoxBrush, ESlateDrawEffect::None, Style->SelectionBoxBrush.TintColor.GetSpecifiedColor());
	}

	LayerId = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId+10, InWidgetStyle, bParentEnabled);

	OutDrawElements.PopClip();
	
	return LayerId+20;
}

FReply SFICGraphView::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Unhandled().DetectDrag(AsShared(), EKeys::Invalid);
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
		for (TSharedRef<FFICEditorAttributeBase> Attribute : Attributes) {
			FFICFrameRange Range = FrameRange.Get();
			for (FICFrame Frame : Range) {
				FVector2D PlotPoint = FrameAttributeToLocal(Attribute, Frame);
				FVector2D Difference = PlotPoint - LocalMousePos;
				if (Difference.Size() < 5) {
					BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute->GetAttribute(), TNumericLimits<int64>::Min(), Frame)
					Attribute->SetKeyframe(FFICValueTime(Frame, LocalToValue(LocalMousePos.Y)));
					Attribute->GetAttribute().RecalculateAllKeyframes();
					END_QUICK_ATTRIB_CHANGE(Context->ChangeList)
					return FReply::Handled();
				}
			}
		}
		SetSelection({});
	}
	return FReply::Unhandled();
}

FReply SFICGraphView::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
	return SPanel::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FReply SFICGraphView::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) {
		return FReply::Handled().BeginDragDrop(MakeShared<FFICGraphPanDragDrop>(SharedThis(this), MouseEvent));
	} else if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		return FReply::Handled().BeginDragDrop(MakeShared<FFICGraphSelectionDragDrop>(SharedThis(this), MouseEvent));
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
	if (InKeyEvent.GetKey() == EKeys::Delete) {
		TSet<TPair<FFICAttribute*, FICFrame>> Selection = GetSelection();
		TMap<FFICAttribute*, TSharedRef<FFICAttribute>> Snapshots;
		for (const TPair<FFICAttribute*, FICFrame>& SelectedKeyframe : Selection) {
			TSharedRef<FFICAttribute>* Snapshot = Snapshots.Find(SelectedKeyframe.Key);
			if (!Snapshot) Snapshots.Add(SelectedKeyframe.Key, SelectedKeyframe.Key->Get());
			SelectedKeyframe.Key->RemoveKeyframe(SelectedKeyframe.Value);
		}
		TSharedRef<FFICChange_Group> Group = MakeShared<FFICChange_Group>();
		for (const TPair<FFICAttribute*, TSharedRef<FFICAttribute>>& Snapshot : Snapshots) {
			Group->PushChange(MakeShared<FFICChange_Attribute>(Snapshot.Key, Snapshot.Value));
		}
		Context->ChangeList.PushChange(Group);
		return FReply::Handled();
	}
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
		TSharedRef<SFICGraphViewKeyframe> Child = Children[ChildIndex];
		float Frame = FrameToLocal(Child->GetFrame());
		float Value = ValueToLocal(Child->GetKeyframe()->GetValue());
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Child, FVector2D(Frame, Value), Child->GetDesiredSize(), 1));
	}
}

const TSet<TPair<FFICAttribute*, int64>>& SFICGraphView::GetSelection() {
	return SelectedKeyframes;
}

void SFICGraphView::SetSelection(const TSet<TPair<FFICAttribute*, FICFrame>>& InSelection) {
	SelectedKeyframes = InSelection;
	SelectedWithBox = SelectedKeyframes;
	Invalidate(EInvalidateWidgetReason::Layout);
}

void SFICGraphView::AddKeyframeToSelection(FFICAttribute& InAttribute, FICFrame InFrame) {
	SelectedKeyframes.Add(TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame));
	Invalidate(EInvalidateWidgetReason::Layout);
}

void SFICGraphView::RemoveKeyframeFromSelection(FFICAttribute& InAttribute, FICFrame InFrame) {
	SelectedKeyframes.Remove(TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame));
	Invalidate(EInvalidateWidgetReason::Layout);
}

bool SFICGraphView::IsKeyframeSelected(FFICAttribute& InAttribute, FICFrame InFrame) {
	return GetSelection().Contains(TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame));
}

void SFICGraphView::ToggleKeyframeSelection(FFICAttribute& InAttribute, FICFrame InFrame, const FModifierKeysState* InModifiers) {
	bool bIsSelected = IsKeyframeSelected(InAttribute, InFrame);
	if (!InModifiers || InModifiers->IsShiftDown()) {
		if (bIsSelected) {
			RemoveKeyframeFromSelection(InAttribute, InFrame);
		} else {
			AddKeyframeToSelection(InAttribute, InFrame);
		}
	} else if (InModifiers->IsControlDown()) {
		if (!bIsSelected) AddKeyframeToSelection(InAttribute, InFrame);
	} else if (InModifiers->IsAltDown()) {
		if (bIsSelected) RemoveKeyframeFromSelection(InAttribute, InFrame);
	} else if (!BoxSelection.bIsValid) {
		SetSelection({TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame)});
	} else {
		if (!bIsSelected) AddKeyframeToSelection(InAttribute, InFrame);
	}
}

void SFICGraphView::BeginBoxSelection(const FModifierKeysState& InModifiers) {
	SelectedWithBox = SelectedKeyframes;
	if (!InModifiers.IsCommandDown() && !InModifiers.IsShiftDown() && !InModifiers.IsAltDown()) {
		SetSelection({});
	}
	BoxSelection = FBox2D(0);
	BoxSelection.bIsValid = true;
}

void SFICGraphView::EndBoxSelection(const FModifierKeysState& InModifiers) {
	SelectedWithBox = SelectedKeyframes;
	BoxSelection.bIsValid = false;
}

void SFICGraphView::SetBoxSelection(FBox2D InBox, const FModifierKeysState& InModifiers) {
	BoxSelection = InBox;
	BoxSelection.bIsValid = true;
	SelectedKeyframes = SelectedWithBox;
	
	FFICFrameRange Frames(InBox.Min.X, InBox.Max.X);
	FFICValueRange Values(InBox.Min.Y, InBox.Max.Y);

	for (TSharedRef<FFICEditorAttributeBase> Attribute : Attributes) {
		TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = Attribute->GetAttribute().GetKeyframes();
		for (const TPair<FICFrame, TSharedRef<FFICKeyframe>>& Keyframe : Keyframes) {
			if (Frames.IsInRange(Keyframe.Key) && Values.IsInRange(Keyframe.Value->GetValue())) {
				ToggleKeyframeSelection(Attribute->GetAttribute(), Keyframe.Key, &InModifiers);
			}
		}
	}
}

void SFICGraphView::SetAttributes(const TArray<TSharedRef<FFICEditorAttributeBase>>& InAttributes) {
	for (TSharedRef<FFICEditorAttributeBase> Attribute : Attributes) {
		Attribute->GetAttribute().OnUpdate.Remove(DelegateHandles[Attribute]);
	}
	
	Attributes = InAttributes;

	for (TSharedRef<FFICEditorAttributeBase> Attribute : Attributes) {
		DelegateHandles.Add(Attribute, Attribute->GetAttribute().OnUpdate.AddSP(this, &SFICGraphView::Update));
	}
	
	Update();
}


void SFICGraphView::Update() {
	Children.Empty();
	
	for (TSharedRef<FFICEditorAttributeBase> Attribute : Attributes) {
		for (const TPair<FICFrame, TSharedRef<FFICKeyframe>>& Keyframe : Attribute->GetAttribute().GetKeyframes()) {
			Children.Add(SNew(SFICGraphViewKeyframe, this, &Attribute->GetAttribute(), Keyframe.Key));
		}
	}
}

void SFICGraphView::FitAll() {
	FFICFrameRange Frames = FrameHighlightRange.Get();
	FFICValueRange Values;
	Values.Begin = TNumericLimits<FICValue>::Max();
	Values.End = TNumericLimits<FICValue>::Lowest();
	int MaxKeyframeCountOfAnyAttribute = 0;
	for (TSharedRef<FFICEditorAttributeBase> Attribute : Attributes) {
		Values.Begin = FMath::Min3(Values.Begin, Attribute->GetValue(TNumericLimits<FICFrame>::Min()), Attribute->GetValue(TNumericLimits<FICFrame>::Max()));
		Values.End = FMath::Max3(Values.End, Attribute->GetValue(TNumericLimits<FICFrame>::Min()), Attribute->GetValue(TNumericLimits<FICFrame>::Max()));
		TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = Attribute->GetAttribute().GetKeyframes();
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
	FrameSpan = FrameSpan/5;
	ValueSpan = FMath::Max(ValueSpan/10.0f, 1.0f);
	Frames.Begin -= FrameSpan;
	Frames.End += FrameSpan;
	OnFrameRangeChanged.Execute(Frames);
	if (Attributes.Num() > 0) {
		Values.Begin -= ValueSpan;
		Values.End += ValueSpan;
	} else {
		Values.Begin = -1;
		Values.End = 1;
	}
	OnValueRangeChanged.Execute(Values);
}

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
		Values.End,
		Values.Begin,
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

FVector2D SFICGraphView::FrameAttributeToLocal(TSharedRef<FFICEditorAttributeBase> InAttribute, FICFrame InFrame) const {
	return FVector2D(
		FrameToLocal(InFrame),
		ValueToLocal(InAttribute->GetValue(InFrame)));
}

TSharedPtr<SFICGraphViewKeyframe> SFICGraphView::FindKeyframeControl(TSharedRef<FFICEditorAttributeBase> InAttribute, FICFrame InFrame) {
	return FindKeyframeControl(&InAttribute->GetAttribute(), InFrame);
}

TSharedPtr<SFICGraphViewKeyframe> SFICGraphView::FindKeyframeControl(FFICAttribute* InAttribute, FICFrame InFrame) {
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFICGraphViewKeyframe> Child = StaticCastSharedRef<SFICGraphViewKeyframe>(Children.GetChildAt(i));
		if (&Child->GetAttribute() == InAttribute && Child->GetFrame() == InFrame) {
			return Child;
		}
	}
	return nullptr;
}
