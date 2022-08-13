#include "Editor/UI/FICDragDrop.h"

#include "Editor/FICChangeList.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICSequencer.h"
#include "Widgets/SToolTip.h"

FFICGraphDragDrop::FFICGraphDragDrop(TSharedRef<SFICGraphView> GraphView, FPointerEvent InitEvent) : GraphView(GraphView) {
	FVector2D LocalPos = GraphView->GetCachedGeometry().AbsoluteToLocal(InitEvent.GetScreenSpacePosition());
	TimelineStart = GraphView->LocalToFrame(LocalPos.X);
	ValueStart = GraphView->LocalToValue(LocalPos.Y);
}

void FFICGraphDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FDragDropOperation::OnDragged(DragDropEvent);

	FVector2D CursorDelta = DragDropEvent.GetCursorDelta();

	if (bRestrictDirections) {
		if (DragDropEvent.IsControlDown()) CursorDelta.Y = 0;
		if (DragDropEvent.IsShiftDown()) CursorDelta.X = 0;
	}
	
	CumulativeDelta += CursorDelta;
	
	float TimelinePerLocal, ValuePerLocal;
	FFICFrameRange FrameRange = GraphView->GetFrameRange();
	FFICValueRange ValueRange = GraphView->GetValueRange();

	ValuePerLocal = (float)ValueRange.Length() / GraphView->GetCachedGeometry().Size.Y;
	ValueDiff = ValuePerLocal * CursorDelta.Y;
	TimelinePerLocal = (float)FrameRange.Length() / GraphView->GetCachedGeometry().Size.X;
	TimelineDiff = FMath::RoundToDouble(TimelinePerLocal * CumulativeDelta.X);
	CumulativeDelta.X -= (float)TimelineDiff / TimelinePerLocal;

	CumulativeTimelineDiff += TimelineDiff;
	CumulativeValueDiff += ValueDiff;
}

void FFICGraphPanDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);

	FFICFrameRange FrameRange = GraphView->GetFrameRange();
	FFICValueRange ValueRange = GraphView->GetValueRange();

	GraphView->SetFrameRange(FrameRange - TimelineDiff);
	GraphView->SetValueRange(ValueRange + ValueDiff);
}

FFICGraphSelectionDragDrop::FFICGraphSelectionDragDrop(TSharedRef<SFICGraphView> GraphView, FPointerEvent InitEvent): FFICGraphDragDrop(GraphView, InitEvent) {
	bRestrictDirections = false;
	GraphView->BeginBoxSelection(InitEvent.GetModifierKeys());
}

void FFICGraphSelectionDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);
	
	FFICFrameRange FrameRange(TimelineStart, TimelineStart + CumulativeTimelineDiff);
	FFICValueRange ValueRange(ValueStart, ValueStart - CumulativeValueDiff);
	FBox2D Box(FVector2D(FrameRange.Begin, ValueRange.Begin), FVector2D(FrameRange.End, ValueRange.End));
	GraphView->SetBoxSelection(Box, DragDropEvent.GetModifierKeys());
}

void FFICGraphSelectionDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FFICGraphDragDrop::OnDrop(bDropWasHandled, MouseEvent);
	GraphView->EndBoxSelection(MouseEvent.GetModifierKeys());
}

void FFICGraphKeyframeDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);
	double start = FPlatformTime::Seconds();
	for (TTuple<FFICAttribute*, TSharedRef<FFICAttribute>> Snapshot : OldAttributeState) {
		Snapshot.Key->Set(Snapshot.Value);
	}

	TMap<FFICAttribute*, TArray<FICFrame>> Movements;
	for (const TPair<FFICAttribute*, FICFrame>& Keyframe : OldKeyframes) {
		Movements.FindOrAdd(Keyframe.Key).Add(Keyframe.Value);
	}
	TSet<TPair<FFICAttribute*, FICFrame>> Selection;
	double start2 = FPlatformTime::Seconds();
	for (TPair<FFICAttribute*, TArray<FICFrame>>& Movement : Movements) {
		Movement.Key->LockUpdateEvent();
		TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = Movement.Key->GetKeyframes();
		Movement.Value.Sort();
		int32 Step = (CumulativeTimelineDiff>0) ? -1 : 1;
		for (int32 Index = (Step<0) ? Movement.Value.Num()-1 : 0; Movement.Value.IsValidIndex(Index); Index += Step) {
			TSharedRef<FFICKeyframe>* KeyframePtr = Keyframes.Find(Movement.Value[Index]);
			if (!KeyframePtr) continue;
			TSharedRef<FFICKeyframe>& Keyframe = *KeyframePtr;
			Keyframe->SetValue(Keyframe->GetValue() - CumulativeValueDiff);
			Movement.Key->MoveKeyframe(Movement.Value[Index], Movement.Value[Index] + CumulativeTimelineDiff);
			Selection.Add(TPair<FFICAttribute*, FICFrame>(Movement.Key, Movement.Value[Index] + CumulativeTimelineDiff));
		}
		Movement.Key->RecalculateAllKeyframes();
		Movement.Key->UnlockUpdateEvent();
	}
	GraphView->SetSelection(Selection);
}

void FFICGraphKeyframeDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FFICGraphDragDrop::OnDrop(bDropWasHandled, MouseEvent);
	auto Change = MakeShared<FFICChange_Group>();
	for (const TPair<FFICAttribute*, TSharedRef<FFICAttribute>>& State : OldAttributeState) {
		Change->PushChange(MakeShared<FFICChange_Attribute>(State.Key, State.Value));
	}
	GraphView->Context->ChangeList.PushChange(Change);
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

FFICSequencerKeyframeDragDrop::FFICSequencerKeyframeDragDrop(TSharedRef<SFICSequencerRowAttribute> InRowAttribute, TSharedRef<SFICSequencerRowAttributeKeyframe> Keyframe, FPointerEvent InitEvent) {
	RowAttribute = InRowAttribute;
	Attribute = Keyframe->GetAttribute();
	Frame = Keyframe->GetFrame();
	OldAttributeState = Attribute->Get();
}

void FFICSequencerKeyframeDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	CumulativeLocalDiff += DragDropEvent.GetCursorDelta();
	CumulativeTimeDiff = RowAttribute->GetFramePerLocal() * CumulativeLocalDiff.X;
	
	Attribute->Set(OldAttributeState.ToSharedRef());

	Attribute->LockUpdateEvent();
	Attribute->MoveKeyframe(Frame, Frame + CumulativeTimeDiff);
	Attribute->RecalculateAllKeyframes();
	Attribute->UnlockUpdateEvent();
}

void FFICSequencerKeyframeDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}
