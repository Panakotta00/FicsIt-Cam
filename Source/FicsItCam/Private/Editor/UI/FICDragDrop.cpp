#include "Editor/UI/FICDragDrop.h"

#include "Editor/FICChangeList.h"
#include "Editor/FICEditorContext.h"
#include "Widgets/SToolTip.h"

FFICGraphDragDrop::FFICGraphDragDrop(TSharedRef<SFICGraphView> GraphView, FPointerEvent InitEvent) : GraphView(GraphView) {
	FVector2D LocalPos = GraphView->GetCachedGeometry().AbsoluteToLocal(InitEvent.GetScreenSpacePosition());
	TimelineStart = GraphView->LocalToFrame(LocalPos.X);
	ValueStart = GraphView->LocalToValue(LocalPos.Y);
}

void FFICGraphDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FDragDropOperation::OnDragged(DragDropEvent);

	FVector2D CursorDelta = DragDropEvent.GetCursorDelta();

	if (DragDropEvent.IsControlDown()) CursorDelta.Y = 0;
	if (DragDropEvent.IsShiftDown()) CursorDelta.X = 0;
	
	KommulativeDelta += CursorDelta;
	
	float TimelinePerLocal, ValuePerLocal;
	FFICFrameRange FrameRange = GraphView->GetFrameRange();
	FFICValueRange ValueRange = GraphView->GetValueRange();

	ValuePerLocal = (float)ValueRange.Length() / GraphView->GetCachedGeometry().Size.Y;
	ValueDiff = ValuePerLocal * CursorDelta.Y;
	TimelinePerLocal = (float)FrameRange.Length() / GraphView->GetCachedGeometry().Size.X;
	TimelineDiff = TimelinePerLocal * KommulativeDelta.X;
	KommulativeDelta.X -= (int64)TimelineDiff / TimelinePerLocal;
}

void FFICGraphPanDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);

	FFICFrameRange FrameRange = GraphView->GetFrameRange();
	FFICValueRange ValueRange = GraphView->GetValueRange();

	GraphView->SetFrameRange(FrameRange - TimelineDiff);
	GraphView->SetValueRange(ValueRange + ValueDiff);
}

void FFICGraphKeyframeDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);

	TSharedPtr<FFICKeyframe> Keyframe = GraphKeyframe->GetKeyframe();
	FICValue NewValue = Keyframe->GetValue() - ValueDiff;
	if (DragDropEvent.IsControlDown()) NewValue = ValueStart;
	Keyframe->SetValue(NewValue);
	
	FICFrame NewFrame = GraphKeyframe->GetFrame() + TimelineDiff;
	if (DragDropEvent.IsShiftDown()) NewFrame = TimelineStart;
	FFICAttribute* Attribute = &GraphKeyframe->GetAttribute();
	Attribute->MoveKeyframe(GraphKeyframe->GetFrame(), NewFrame);
	Attribute->RecalculateAllKeyframes();
	GraphKeyframe = GraphView->FindKeyframeControl(&GraphKeyframe->GetAttribute(), NewFrame);
}

TSharedPtr<SWidget> FFICGraphKeyframeDragDrop::GetDefaultDecorator() const {
	return SNew(SBorder)
		.BorderImage( FCoreStyle::Get().GetBrush("ToolTip.BrightBackground") )
		.Padding(FMargin(11.0f))[
			SNew(STextBlock)
			.ColorAndOpacity( FLinearColor::Black )
			.WrapTextAt_Static( &SToolTip::GetToolTipWrapWidth )
			.Text_Lambda([this]() {
				return FText::FromString(FString::Printf(TEXT("Frame: %lld\nValue: %i\n\nShift: Fixed Frame\nCTRL: Fixed Value"), GraphKeyframe->GetFrame(), static_cast<int>(GraphKeyframe->GetAttribute().GetFloatValue(GraphKeyframe->GetFrame()))));
			})
		];
}

FVector2D FFICGraphKeyframeDragDrop::GetDecoratorPosition() const {
                                                    	return FSlateApplication::Get().GetCursorPos() + FVector2D(20, 20);
}

void FFICGraphKeyframeDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FFICGraphDragDrop::OnDrop(bDropWasHandled, MouseEvent);
	auto Change = MakeShared<FFICChange_Group>();
	Change->PushChange(MakeShared<FFICChange_ActiveFrame>(GraphKeyframe->GetContext(), TimelineStart, GraphKeyframe->GetFrame()));
	Change->PushChange(MakeShared<FFICChange_Attribute>(&GraphKeyframe->GetAttribute(), AttribBegin.ToSharedRef()));
	GraphKeyframe->GetContext()->ChangeList.PushChange(Change);
}

FFICGraphKeyframeHandleDragDrop::FFICGraphKeyframeHandleDragDrop(TSharedRef<SFICGraphViewKeyframeHandle> KeyframeHandle, FPointerEvent InitEvent) : FFICGraphDragDrop(SharedThis(KeyframeHandle->GetGraphKeyframe()->GetGraphView()), InitEvent), KeyframeHandle(KeyframeHandle) {
	AttribBegin = KeyframeHandle->GetGraphKeyframe()->GetAttribute().Get();
}

void FFICGraphKeyframeHandleDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);

	TSharedPtr<FFICKeyframe> Keyframe = KeyframeHandle->GetGraphKeyframe()->GetKeyframe();
	FFICValueTimeFloat OldControl;
	FFICValueTimeFloat NewControl;
	if (KeyframeHandle->IsOutHandle()) {
		NewControl = OldControl = Keyframe->GetOutControl();
		NewControl.Frame += TimelineDiff;
		NewControl.Value -= ValueDiff;
		Keyframe->SetOutControl(NewControl);
	} else {
		NewControl = OldControl = Keyframe->GetInControl();
		NewControl.Frame -= TimelineDiff;
		NewControl.Value += ValueDiff;
		Keyframe->SetInControl(NewControl);
	}
	
	float TimelinePerLocal = GraphView->GetFramePerLocal();
	float ValuePerLocal = GraphView->GetValuePerLocal();

	FVector2D NewVector = FVector2D(NewControl.Frame/TimelinePerLocal, NewControl.Value/ValuePerLocal);
	bool bNewVector = false;

	switch (Keyframe->GetType()) {
	case FIC_KF_EASE:
		Keyframe->SetType(FIC_KF_MIRROR);
	case FIC_KF_MIRROR: {
		FVector2D OldVector(OldControl.Frame/TimelinePerLocal, OldControl.Value/ValuePerLocal);
		OldVector.Normalize();
		NewVector.Normalize();
		float Angle = FMath::RadiansToDegrees(FMath::Atan2(OldVector.Y, OldVector.X) - FMath::Atan2(NewVector.Y, NewVector.X));
		if (FMath::IsNaN(Angle)) break;
		Keyframe->GetInControl().Get(OldVector.X, OldVector.Y);
		OldVector.X /= TimelinePerLocal;
		OldVector.Y /= ValuePerLocal;
		NewVector = OldVector.GetRotated(-Angle);
		if (FMath::IsNaN(NewVector.X) || FMath::IsNaN(NewVector.Y)) break;
		bNewVector = true;
		break;
	}
	case FIC_KF_EASEINOUT:
		Keyframe->SetType(FIC_KF_CUSTOM);
	default: ;
	}
	if (bNewVector) {
		if (KeyframeHandle->IsOutHandle()) {
			Keyframe->SetInControl(FFICValueTimeFloat(NewVector.X*TimelinePerLocal, NewVector.Y*ValuePerLocal));
		} else {
			Keyframe->SetOutControl(FFICValueTimeFloat(NewVector.X*TimelinePerLocal, NewVector.Y*ValuePerLocal));
		}
	}
}

void FFICGraphKeyframeHandleDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FFICGraphDragDrop::OnDrop(bDropWasHandled, MouseEvent);
	auto Change = MakeShared<FFICChange_Group>();
	Change->PushChange(MakeShared<FFICChange_ActiveFrame>(KeyframeHandle->GetGraphKeyframe()->GetContext(), TimelineStart, KeyframeHandle->GetGraphKeyframe()->GetFrame()));
	Change->PushChange(MakeShared<FFICChange_Attribute>(&KeyframeHandle->GetGraphKeyframe()->GetAttribute(), AttribBegin.ToSharedRef()));
	KeyframeHandle->GetGraphKeyframe()->GetContext()->ChangeList.PushChange(Change);
}
