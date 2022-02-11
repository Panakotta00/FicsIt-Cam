#include "Editor/UI/FICDragDrop.h"

#include "Editor/FICChangeList.h"
#include "Editor/FICEditorAttributeBase.h"
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

	TSharedPtr<FFICKeyframe> Keyframe = KeyframeControl->GetAttribute()->GetKeyframe(KeyframeControl->GetFrame());
	float NewValue = Keyframe->GetValue() - ValueDiff;
	if (DragDropEvent.IsControlDown()) NewValue = ValueStart;
	Keyframe->SetValue(NewValue);
	
	int64 NewFrame = KeyframeControl->GetFrame() + TimelineDiff;
	if (DragDropEvent.IsShiftDown()) NewFrame = TimelineStart;
	FFICAttribute* Attribute = KeyframeControl->GetAttribute()->GetAttribute();
	Attribute->MoveKeyframe(KeyframeControl->GetFrame(), NewFrame);
	Attribute->RecalculateAllKeyframes();
	KeyframeControl = GraphView->FindKeyframeControl(KeyframeControl->GetAttribute(), NewFrame).ToSharedRef();
}

TSharedPtr<SWidget> FFICGraphKeyframeDragDrop::GetDefaultDecorator() const {
	return SNew(SBorder)
		.BorderImage( FCoreStyle::Get().GetBrush("ToolTip.BrightBackground") )
		.Padding(FMargin(11.0f))[
			SNew(STextBlock)
			.ColorAndOpacity( FLinearColor::Black )
			.WrapTextAt_Static( &SToolTip::GetToolTipWrapWidth )
			.Text_Lambda([this]() {
				return FText::FromString(FString::Printf(TEXT("Frame: %lld\nValue: %i\n\nShift: Fixed Frame\nCTRL: Fixed Value"), KeyframeControl->GetFrame(), static_cast<int>(KeyframeControl->GetAttribute()->GetValue(KeyframeControl->GetFrame()))));
			})
		];
}

FVector2D FFICGraphKeyframeDragDrop::GetDecoratorPosition() const {
	return FSlateApplication::Get().GetCursorPos() + FVector2D(20, 20);
}

void FFICGraphKeyframeDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FFICGraphDragDrop::OnDrop(bDropWasHandled, MouseEvent);
	auto Change = MakeShared<FFICChange_Group>();
	Change->PushChange(MakeShared<FFICChange_ActiveFrame>(KeyframeControl->Context, TimelineStart, KeyframeControl->GetFrame()));
	Change->PushChange(MakeShared<FFICChange_Attribute>(KeyframeControl->GetAttribute()->GetAttribute(), AttribBegin));
	KeyframeControl->Context->ChangeList.PushChange(Change);
}

FFICGraphKeyframeHandleDragDrop::FFICGraphKeyframeHandleDragDrop(TSharedRef<SFICKeyframeHandle> KeyframeHandle, FPointerEvent InitEvent) : FFICGraphDragDrop(SharedThis(KeyframeHandle->KeyframeControl->GraphView), InitEvent), KeyframeHandle(KeyframeHandle) {
	AttribBegin = KeyframeHandle->KeyframeControl->GetAttribute()->GetAttribute()->Get();
}

void FFICGraphKeyframeHandleDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FFICGraphDragDrop::OnDragged(DragDropEvent);

	TSharedPtr<FFICKeyframe> Keyframe = KeyframeHandle->KeyframeControl->GetAttribute()->GetKeyframe(KeyframeHandle->KeyframeControl->GetFrame());
	if (KeyframeHandle->bIsOutHandle) {
		FFICValueTimeFloat OldControl = Keyframe->GetOutControl();
		FFICValueTimeFloat NewControl = OldControl;
		NewControl.Frame += TimelineDiff;
		NewControl.Value -= ValueDiff;
		Keyframe->SetOutControl(NewControl);

		float TimelinePerLocal = GraphView->GetFramePerLocal();
		float ValuePerLocal = GraphView->GetValuePerLocal();

		switch (Keyframe->KeyframeType) {
		case FIC_KF_EASE:
			Keyframe->KeyframeType = FIC_KF_MIRROR;
		case FIC_KF_MIRROR: {
			FVector2D OldVector(OldControl.Frame/TimelinePerLocal, OldControl.Value/ValuePerLocal);
			FVector2D NewVector(NewControl.Frame/TimelinePerLocal, NewControl.Value/ValuePerLocal);
			OldVector.Normalize();
			NewVector.Normalize();
			float Angle = FMath::RadiansToDegrees(FMath::Atan2(OldVector.Y, OldVector.X) - FMath::Atan2(NewVector.Y, NewVector.X));
			if (FMath::IsNaN(Angle)) break;
			Keyframe->GetInControl().Get(OldVector.X, OldVector.Y);
			OldVector.X /= TimelinePerLocal;
			OldVector.Y /= ValuePerLocal;
			NewVector = OldVector.GetRotated(-Angle);
			if (FMath::IsNaN(NewVector.X) || FMath::IsNaN(NewVector.Y)) break;
			Keyframe->SetInControl(FFICValueTimeFloat(NewVector.X*TimelinePerLocal, NewVector.Y*ValuePerLocal));
			break;
		}
		case FIC_KF_EASEINOUT:
			Keyframe->KeyframeType = FIC_KF_CUSTOM;
		}
	} else {
		FFICValueTimeFloat OldControl = Keyframe->GetInControl();
		FFICValueTimeFloat NewControl = OldControl;
		float Ratio = OldControl.Frame/OldControl.Value;
		NewControl.Frame -= TimelineDiff;
		NewControl.Value += ValueDiff;
		Keyframe->SetInControl(NewControl);

		float TimelinePerLocal = GraphView->GetFramePerLocal();
		float ValuePerLocal = GraphView->GetValuePerLocal();

		switch (Keyframe->KeyframeType) {
		case FIC_KF_EASE:
			Keyframe->KeyframeType = FIC_KF_MIRROR;
		case FIC_KF_MIRROR: {
			FVector2D OldVector(OldControl.Frame/TimelinePerLocal, OldControl.Value/ValuePerLocal);
			FVector2D NewVector(NewControl.Frame/TimelinePerLocal, NewControl.Value/ValuePerLocal);
			OldVector.Normalize();
			NewVector.Normalize();
			float Angle = FMath::RadiansToDegrees(FMath::Atan2(OldVector.Y, OldVector.X) - FMath::Atan2(NewVector.Y, NewVector.X));
			if (FMath::IsNaN(Angle)) break;
			Keyframe->GetOutControl().Get(OldVector.X, OldVector.Y);
			OldVector.X /= TimelinePerLocal;
			OldVector.Y /= ValuePerLocal;
			NewVector = OldVector.GetRotated(-Angle);
			if (FMath::IsNaN(NewVector.X) || FMath::IsNaN(NewVector.Y)) break;
			Keyframe->SetOutControl(FFICValueTimeFloat(NewVector.X*TimelinePerLocal, NewVector.Y*ValuePerLocal));
			break;
		}
		case FIC_KF_EASEINOUT:
			Keyframe->KeyframeType = FIC_KF_CUSTOM;
		}
	}
}

void FFICGraphKeyframeHandleDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FFICGraphDragDrop::OnDrop(bDropWasHandled, MouseEvent);
	auto Change = MakeShared<FFICChange_Group>();
	Change->PushChange(MakeShared<FFICChange_ActiveFrame>(KeyframeHandle->KeyframeControl->Context, TimelineStart, KeyframeHandle->KeyframeControl->GetFrame()));
	Change->PushChange(MakeShared<FFICChange_Attribute>(KeyframeHandle->KeyframeControl->GetAttribute()->GetAttribute(), AttribBegin));
	KeyframeHandle->KeyframeControl->Context->ChangeList.PushChange(Change);
}
