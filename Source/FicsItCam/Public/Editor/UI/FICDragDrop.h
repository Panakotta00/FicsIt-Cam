#pragma once

#include "FICGraphView.h"
#include "Data/Attributes/FICAttribute.h"
#include "Editor/FICEditorAttributeBase.h"

class FFICGraphDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphDragDrop, FDragDropOperation)

	TSharedRef<SFICGraphView> GraphView;
	FVector2D KommulativeDelta;

	int64 TimelineStart;
	float ValueStart;
	
	int64 TimelineDiff;
	float ValueDiff;


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

class FFICGraphKeyframeDragDrop : public FFICGraphDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphKeyframeDragDrop, FFICGraphDragDrop)

	TSharedRef<SFICKeyframeControl> KeyframeControl;
	TSharedRef<FFICAttribute> AttribBegin;

	FFICGraphKeyframeDragDrop(TSharedRef<SFICGraphView> GraphView, TSharedRef<SFICKeyframeControl> KeyframeControl, FPointerEvent InitEvent) : FFICGraphDragDrop(GraphView, InitEvent), KeyframeControl(KeyframeControl) {
		AttribBegin = KeyframeControl->GetAttribute()->GetAttribute()->Get();
		FDragDropOperation::SetDecoratorVisibility(true);
		bCreateNewWindow = false;
	}
	
	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;
	virtual FVector2D GetDecoratorPosition() const override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	// End FDragDropOperation
};

class FFICGraphKeyframeHandleDragDrop : public FFICGraphDragDrop {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphKeyframeHandleDragDrop, FFICGraphDragDrop)

	TSharedRef<SFICKeyframeHandle> KeyframeHandle;
	TSharedRef<FFICAttribute> AttribBegin;

	FFICGraphKeyframeHandleDragDrop(TSharedRef<SFICKeyframeHandle> KeyframeHandle, FPointerEvent InitEvent);

	// Begin FDragDropOperation
	virtual void OnDragged( const FDragDropEvent& DragDropEvent ) override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	// End FDragDropOperation
};
