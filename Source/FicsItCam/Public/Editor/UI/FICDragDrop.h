#pragma once

#include "FICGraphView.h"
#include "Data/Attributes/FICAttribute.h"

class SFICSequencerRowAttributeKeyframe;
class SFICSequencerRowAttribute;

class FFICGraphDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphDragDrop, FDragDropOperation)

	TSharedPtr<SFICGraphView> GraphView;
	FVector2D CumulativeDelta = FVector2D::ZeroVector;

	int64 TimelineStart;
	float ValueStart;
	
	int64 TimelineDiff = 0;
	float ValueDiff = 0.0f;

	int64 CumulativeTimelineDiff = 0;
	float CumulativeValueDiff = 0.0f;

	bool bRestrictDirections = true;

	FFICGraphDragDrop(TSharedRef<SFICGraphView> GraphView, FPointerEvent InitEvent);
	
	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	// End FDragDropOperation
};

class FFICGraphPanDragDrop : public FFICGraphDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphPanDragDrop, FFICGraphDragDrop)

	FFICGraphPanDragDrop(TSharedRef<SFICGraphView> GraphView, FPointerEvent InitEvent) : FFICGraphDragDrop(GraphView, InitEvent) {}

	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual FCursorReply OnCursorQuery() { return FCursorReply::Cursor(EMouseCursor::Hand); }
	// End FDragDropOperation
};

class FFICGraphSelectionDragDrop : public FFICGraphDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphSelectionDragDrop, FFICGraphDragDrop)

	FFICGraphSelectionDragDrop(TSharedRef<SFICGraphView> GraphView, FPointerEvent InitEvent);

	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery() { return FCursorReply::Cursor(EMouseCursor::Crosshairs); }
	// End FDragDropOperation
};

class FFICGraphKeyframeDragDrop : public FFICGraphDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphKeyframeDragDrop, FFICGraphDragDrop)

	TSet<TPair<FFICAttribute*, FICFrame>> OldKeyframes;
	TMap<FFICAttribute*, TSharedRef<FFICAttribute>> OldAttributeState;
	
	FFICGraphKeyframeDragDrop(TSharedRef<SFICGraphView> GraphView, FPointerEvent InitEvent) : FFICGraphDragDrop(GraphView, InitEvent) {
		OldKeyframes = GraphView->GetSelectionManager().GetSelection();
		for (const TPair<FFICAttribute*, FICFrame>& Keyframe : OldKeyframes) {
			TSharedRef<FFICAttribute>* State = OldAttributeState.Find(Keyframe.Key);
			if (State) continue;
			OldAttributeState.Add(Keyframe.Key, Keyframe.Key->Get());
		}
	}
	
	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	// End FDragDropOperation
};

class FFICGraphKeyframeHandleDragDrop : public FFICGraphDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphKeyframeHandleDragDrop, FFICGraphDragDrop)

	TSharedPtr<SFICGraphViewKeyframeHandle> KeyframeHandle;
	TSharedPtr<FFICAttribute> AttribBegin;

	FFICGraphKeyframeHandleDragDrop(TSharedRef<SFICGraphViewKeyframeHandle> KeyframeHandle, FPointerEvent InitEvent);

	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	// End FDragDropOperation
};

class FFICSequencerDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICSequencerDragDrop, FDragDropOperation)

	TSharedRef<SFICSequencer> Sequencer;
	FICFrameFloat StartFrameF;
	FICFrameFloat StartFramePerLocal;
	FFICFrameRange StartActiveRange;
	FVector2D StartLocal;
	FVector2D CumulativeLocalDiff = FVector2D::ZeroVector;

	FFICSequencerDragDrop(TSharedRef<SFICSequencer> Sequencer, FPointerEvent InitEvent);
	
	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	// End FDragDropOperation
};

class FFICSequencerPanDragDrop : public FFICSequencerDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphPanDragDrop, FFICSequencerDragDrop)

	FFICSequencerPanDragDrop(TSharedRef<SFICSequencer> Sequencer, FPointerEvent InitEvent);

	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual FCursorReply OnCursorQuery() { return FCursorReply::Cursor(EMouseCursor::Hand); }
	// End FDragDropOperation
};

class FFICSequencerKeyframeDragDrop : public FFICSequencerDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICSequencerKeyframeDragDrop, FFICSequencerDragDrop)

	TSet<TPair<FFICAttribute*, FICFrame>> OldKeyframes;
	TMap<FFICAttribute*, TSharedRef<FFICAttribute>> OldAttributeState;
	
	float CumulativeTimeDiff = 0;
	
	FFICSequencerKeyframeDragDrop(TSharedRef<SFICSequencer> Sequencer, FPointerEvent InitEvent);

	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	// End FDragDropOperation
};

class FFICSequencerSelectionDragDrop : public FFICSequencerDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphSelectionDragDrop, FFICSequencerDragDrop)

	FFICSequencerSelectionDragDrop(TSharedRef<SFICSequencer> Sequencer, FPointerEvent InitEvent);

	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery() { return FCursorReply::Cursor(EMouseCursor::Crosshairs); }
	// End FDragDropOperation
};
