#include "FICDragDrop.h"

FFICGraphDragDrop::FFICGraphDragDrop(TSharedRef<SFICGraphView> GraphView) : GraphView(GraphView) {}

void FFICGraphDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FDragDropOperation::OnDragged(DragDropEvent);

	FVector2D CursorDelta = DragDropEvent.GetCursorDelta();
	KommulativeDelta += CursorDelta;
	
	float TimelinePerLocal, ValuePerLocal;
	int64 TimelineBegin, TimelineEnd;
	float ValueBegin, ValueEnd;
	GraphView->GetTimeRange(TimelineBegin, TimelineEnd);
	GraphView->GetValueRange(ValueBegin, ValueEnd);

	ValuePerLocal = (float)(ValueEnd - ValueBegin) / GraphView->GetCachedGeometry().Size.Y;
	ValueDiff = ValuePerLocal * CursorDelta.Y;
	TimelinePerLocal = (float)(TimelineEnd - TimelineBegin) / GraphView->GetCachedGeometry().Size.X;
	TimelineDiff = TimelinePerLocal * KommulativeDelta.X;
	KommulativeDelta.X -= (int64)TimelineDiff / TimelinePerLocal;
}

void FFICGraphPanDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);

	int64 TimelineBegin, TimelineEnd;
	float ValueBegin, ValueEnd;
	GraphView->GetTimeRange(TimelineBegin, TimelineEnd);
	GraphView->GetValueRange(ValueBegin, ValueEnd);

	GraphView->SetTimeRange(TimelineBegin - TimelineDiff, TimelineEnd - TimelineDiff);
	GraphView->SetValueRange(ValueBegin + ValueDiff, ValueEnd + ValueDiff);
}

void FFICGraphKeyframeDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);

	if (!KeyframeControl) return;

	FFICKeyframe* Keyframe = KeyframeControl->GetAttribute()->GetKeyframe(KeyframeControl->GetFrame())->Get();
	Keyframe->SetValueFromFloat(Keyframe->GetValueAsFloat() - ValueDiff);
	
	int64 NewFrame = KeyframeControl->GetFrame() + TimelineDiff;
	KeyframeControl->GetAttribute()->GetAttribute()->MoveKeyframe(KeyframeControl->GetFrame(), NewFrame);
	GraphView->Update();
	KeyframeControl = GraphView->FindKeyframeControl(KeyframeControl->GetAttribute(), NewFrame);
}

FFICGraphKeyframeHandleDragDrop::FFICGraphKeyframeHandleDragDrop(TSharedPtr<SFICKeyframeHandle> KeyframeHandle) : FFICGraphDragDrop(SharedThis(KeyframeHandle->KeyframeControl->GraphView)), KeyframeHandle(KeyframeHandle) {}
#pragma optimize("", off)
void FFICGraphKeyframeHandleDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);

	FFICKeyframe* Keyframe = KeyframeHandle->KeyframeControl->GetAttribute()->GetKeyframe(KeyframeHandle->KeyframeControl->GetFrame())->Get();
	if (KeyframeHandle->bIsOutHandle) {
		float NewFrame, NewValue, OldFrame, OldValue;
		Keyframe->GetOutControlAsFloat(OldFrame, OldValue);
		NewFrame = OldFrame;
		NewValue = OldValue;
		NewFrame += TimelineDiff;
		NewValue -= ValueDiff;
		Keyframe->SetOutControlAsFloat(NewFrame, NewValue);

		float TimelinePerLocal = GraphView->GetFramePerLocal();
		float ValuePerLocal = GraphView->GetValuePerLocal();

		switch (Keyframe->KeyframeType) {
		case FIC_KF_EASE:
			Keyframe->KeyframeType = FIC_KF_MIRROR;
		case FIC_KF_MIRROR: {
			FVector2D OldVector(OldFrame/TimelinePerLocal, OldValue/ValuePerLocal);
			FVector2D NewVector(NewFrame/TimelinePerLocal, NewValue/ValuePerLocal);
			OldVector.Normalize();
			NewVector.Normalize();
			float Angle = FMath::RadiansToDegrees(FMath::Atan2(OldVector.Y, OldVector.X) - FMath::Atan2(NewVector.Y, NewVector.X));
			if (FMath::IsNaN(Angle)) break;
			Keyframe->GetInControlAsFloat(OldVector.X, OldVector.Y);
			OldVector.X /= TimelinePerLocal;
			OldVector.Y /= ValuePerLocal;
			NewVector = OldVector.GetRotated(-Angle);
			Keyframe->SetInControlAsFloat(NewVector.X*TimelinePerLocal, NewVector.Y*ValuePerLocal);
			break;
		}
		case FIC_KF_EASEINOUT:
			Keyframe->KeyframeType = FIC_KF_CUSTOM;
		}
	} else {
		float OldFrame, OldValue, NewFrame, NewValue;
		Keyframe->GetInControlAsFloat(OldFrame, OldValue);
		float Ratio = OldFrame/OldValue;
		NewFrame = OldFrame;
		NewValue = OldValue;
		NewFrame -= TimelineDiff;
		NewValue += ValueDiff;
		Keyframe->SetInControlAsFloat(NewFrame, NewValue);

		float TimelinePerLocal = GraphView->GetFramePerLocal();
		float ValuePerLocal = GraphView->GetValuePerLocal();

		switch (Keyframe->KeyframeType) {
		case FIC_KF_EASE:
			Keyframe->KeyframeType = FIC_KF_MIRROR;
		case FIC_KF_MIRROR: {
			FVector2D OldVector(OldFrame/TimelinePerLocal, OldValue/ValuePerLocal);
			FVector2D NewVector(NewFrame/TimelinePerLocal, NewValue/ValuePerLocal);
			OldVector.Normalize();
			NewVector.Normalize();
			float Angle = FMath::RadiansToDegrees(FMath::Atan2(OldVector.Y, OldVector.X) - FMath::Atan2(NewVector.Y, NewVector.X));
			if (FMath::IsNaN(Angle)) break;
			Keyframe->GetOutControlAsFloat(OldVector.X, OldVector.Y);
			OldVector.X /= TimelinePerLocal;
			OldVector.Y /= ValuePerLocal;
			NewVector = OldVector.GetRotated(-Angle);
			Keyframe->SetOutControlAsFloat(NewVector.X*TimelinePerLocal, NewVector.Y*ValuePerLocal);
			break;
		}
		case FIC_KF_EASEINOUT:
			Keyframe->KeyframeType = FIC_KF_CUSTOM;
		}
	}
}
#pragma optimize("", on)
