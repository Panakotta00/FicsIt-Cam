#pragma once

#include "FICGraphView.h"

class FFICGraphDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphDragDrop, FDragDropOperation)

	TSharedRef<SFICGraphView> GraphView;
	FVector2D KommulativeDelta;

	int64 TimelineDiff;
	float ValueDiff;

	FFICGraphDragDrop(TSharedRef<SFICGraphView> GraphView);
	
	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual FCursorReply OnCursorQuery() { return FCursorReply::Cursor(EMouseCursor::GrabHandClosed); }
	// End FDragDropOperation
};

class FFICGraphPanDragDrop : public FFICGraphDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphPanDragDrop, FFICGraphDragDrop)

	FFICGraphPanDragDrop(TSharedRef<SFICGraphView> GraphView) : FFICGraphDragDrop(GraphView) {}

	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	// End FDragDropOperation
};

class FFICGraphKeyframeDragDrop : public FFICGraphDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphKeyframeDragDrop, FFICGraphDragDrop)

	TSharedPtr<SFICKeyframeControl> KeyframeControl;

	FFICGraphKeyframeDragDrop(TSharedRef<SFICGraphView> GraphView, TSharedRef<SFICKeyframeControl> KeyframeControl) : FFICGraphDragDrop(GraphView), KeyframeControl(KeyframeControl) {}
	
	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	// End FDragDropOperation
};

class FFICGraphKeyframeHandleDragDrop : public FFICGraphDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphKeyframeHandleDragDrop, FFICGraphDragDrop)

	TSharedPtr<SFICKeyframeHandle> KeyframeHandle;

	FFICGraphKeyframeHandleDragDrop(TSharedPtr<SFICKeyframeHandle> KeyframeHandle);

	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	// End FDragDropOperation
};