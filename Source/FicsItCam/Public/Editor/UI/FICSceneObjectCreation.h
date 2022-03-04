#pragma once
#include "Brushes/SlateColorBrush.h"

class UFICEditorContext;

class FFICSceneObjectDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFICGraphDragDrop, FDragDropOperation)

	UFICEditorContext* Context = nullptr;
	UClass* SceneObjectClass = nullptr;
	UObject* SceneObject = nullptr;
	bool bSceneObjectTemp = true;

	FFICSceneObjectDragDrop(UClass* InSceneObjectClass, bool bSceneObjectTemp);
	FFICSceneObjectDragDrop(UFICEditorContext* InContext, UObject* InSceneObject, bool bSceneObjectTemp);

	// Begin FDragDropOperation
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	// End FDragDropOperation
};

class SFICSceneObjectCreationRow : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICSceneObjectCreationRow) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext, UClass* InSceneObjectClass);

private:
	static FSlateColorBrush DefaultHoverBackground;
	UFICEditorContext* Context = nullptr;
	UClass* SceneObjectClass = nullptr;

	bool bClick = false;

public:
	// Begin SWidget
	virtual bool IsInteractable() const override { return true; }
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget
};

class SFICSceneObjectCreation : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICSceneObjectCreation) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);

private:
	UFICEditorContext* Context = nullptr;
};
