#include "Editor/UI/FICDragDrop.h"

#include "Editor/FICChangeList.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICSequencer.h"
#include "Editor/UI/FICSequencerTreeView.h"
#include "Widgets/SToolTip.h"

FFICGraphDragDrop::FFICGraphDragDrop(TSharedRef<SFICGraphView> GraphView, FPointerEvent InitEvent) : GraphView(GraphView) {
	FVector2D LocalPos = GraphView->GetCachedGeometry().AbsoluteToLocal(InitEvent.GetScreenSpacePosition());
	TimelineStart = GraphView->LocalToFrame(LocalPos.X);
	ValueStart = GraphView->LocalToValue(LocalPos.Y);
}

void FFICGraphDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FDragDropOperation::OnDragged(DragDropEvent);

	FVector2D CursorDelta = DragDropEvent.GetCursorDelta() / GraphView->GetCachedGeometry().Scale;

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
	GraphView->GetSelectionManager().BeginBoxSelection(InitEvent.GetModifierKeys());
}

void FFICGraphSelectionDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);
	
	FFICFrameRange FrameRange(TimelineStart, TimelineStart + CumulativeTimelineDiff);
	FFICValueRange ValueRange(ValueStart, ValueStart - CumulativeValueDiff);
	FBox2D Box(FVector2D(FrameRange.Begin, ValueRange.Begin), FVector2D(FrameRange.End, ValueRange.End));
	GraphView->GetSelectionManager().SetBoxSelection(Box, DragDropEvent.GetModifierKeys());
}

void FFICGraphSelectionDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FFICGraphDragDrop::OnDrop(bDropWasHandled, MouseEvent);
	GraphView->GetSelectionManager().EndBoxSelection(MouseEvent.GetModifierKeys());
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
	GraphView->GetSelectionManager().SetSelection(Selection);
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

FFICSequencerDragDrop::FFICSequencerDragDrop(TSharedRef<SFICSequencer> Sequencer, FPointerEvent InitEvent) : Sequencer(Sequencer) {
	StartLocal = Sequencer->GetCachedGeometry().AbsoluteToLocal(InitEvent.GetScreenSpacePosition());
	StartFrameF = Sequencer->LocalToFrameF(StartLocal.X);
	StartActiveRange = Sequencer->Context->GetActiveRange();
	StartFramePerLocal = Sequencer->GetFramePerLocal();
}

void FFICSequencerDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	CumulativeLocalDiff += DragDropEvent.GetCursorDelta() / Sequencer->GetCachedGeometry().Scale;
	
	FDragDropOperation::OnDragged(DragDropEvent);
}

void FFICSequencerDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}

FFICSequencerPanDragDrop::FFICSequencerPanDragDrop(TSharedRef<SFICSequencer> Sequencer, FPointerEvent InitEvent) : FFICSequencerDragDrop(Sequencer, InitEvent) {}

void FFICSequencerPanDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICSequencerDragDrop::OnDragged(DragDropEvent);

	FVector2D Local = Sequencer->GetCachedGeometry().AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());
	FICFrameFloat FrameDiff = (StartLocal - Local).X * StartFramePerLocal;
	
	Sequencer->Context->SetActiveRange(StartActiveRange + FrameDiff);
}

FFICSequencerKeyframeDragDrop::FFICSequencerKeyframeDragDrop(TSharedRef<SFICSequencer> Sequencer, FPointerEvent InitEvent) : FFICSequencerDragDrop(Sequencer, InitEvent) {
	OldKeyframes = Sequencer->GetSelectionManager().GetSelection();
	for (const TPair<FFICAttribute*, FICFrame>& Keyframe : OldKeyframes) {
		TSharedRef<FFICAttribute>* State = OldAttributeState.Find(Keyframe.Key);
		if (State) continue;
		OldAttributeState.Add(Keyframe.Key, Keyframe.Key->Get());
	}
}

void FFICSequencerKeyframeDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICSequencerDragDrop::OnDragged(DragDropEvent);
	
	CumulativeTimeDiff = Sequencer->GetFramePerLocal() * CumulativeLocalDiff.X;

	for (TTuple<FFICAttribute*, TSharedRef<FFICAttribute>> Snapshot : OldAttributeState) {
		Snapshot.Key->Set(Snapshot.Value);
	}

	TMap<FFICAttribute*, TArray<FICFrame>> Movements;
	for (const TPair<FFICAttribute*, FICFrame>& Keyframe : OldKeyframes) {
		Movements.FindOrAdd(Keyframe.Key).Add(Keyframe.Value);
	}
	TSet<TPair<FFICAttribute*, FICFrame>> Selection;
	for (TPair<FFICAttribute*, TArray<FICFrame>>& Movement : Movements) {
		Movement.Key->LockUpdateEvent();
		TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = Movement.Key->GetKeyframes();
		Movement.Value.Sort();
		int32 Step = (CumulativeTimeDiff>0) ? -1 : 1;
		for (int32 Index = (Step<0) ? Movement.Value.Num()-1 : 0; Movement.Value.IsValidIndex(Index); Index += Step) {
			Movement.Key->MoveKeyframe(Movement.Value[Index], Movement.Value[Index] + CumulativeTimeDiff);
			Selection.Add(TPair<FFICAttribute*, FICFrame>(Movement.Key, Movement.Value[Index] + CumulativeTimeDiff));
		}
		Movement.Key->RecalculateAllKeyframes();
		Movement.Key->UnlockUpdateEvent();
	}
	Sequencer->GetSelectionManager().SetSelection(Selection);
}

void FFICSequencerKeyframeDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FFICSequencerDragDrop::OnDrop(bDropWasHandled, MouseEvent);
	auto Change = MakeShared<FFICChange_Group>();
	for (const TPair<FFICAttribute*, TSharedRef<FFICAttribute>>& State : OldAttributeState) {
		Change->PushChange(MakeShared<FFICChange_Attribute>(State.Key, State.Value));
	}
	Sequencer->Context->ChangeList.PushChange(Change);
}


FFICSequencerSelectionDragDrop::FFICSequencerSelectionDragDrop(TSharedRef<SFICSequencer> Sequencer, FPointerEvent InitEvent): FFICSequencerDragDrop(Sequencer, InitEvent) {
	Sequencer->GetSelectionManager().BeginBoxSelection(InitEvent.GetModifierKeys());
}

void FFICSequencerSelectionDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICSequencerDragDrop::OnDragged(DragDropEvent);

	FGeometry Geometry = Sequencer->GetCachedGeometry();
	FVector2D Start = StartLocal;
	FVector2D End = StartLocal + CumulativeLocalDiff;
	FBox2D Box(FVector2D(FMath::Min(Start.X, End.X), FMath::Min(Start.Y, End.Y)), FVector2D(FMath::Max(Start.X, End.X), FMath::Max(Start.Y, End.Y)));
	Sequencer->GetSelectionManager().SetBoxSelection(Box, DragDropEvent.GetModifierKeys());
}

void FFICSequencerSelectionDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FFICSequencerDragDrop::OnDrop(bDropWasHandled, MouseEvent);
	Sequencer->GetSelectionManager().EndBoxSelection(MouseEvent.GetModifierKeys());
}
