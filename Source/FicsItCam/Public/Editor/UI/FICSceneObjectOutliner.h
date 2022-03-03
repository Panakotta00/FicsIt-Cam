#pragma once

class UFICEditorContext;

class SFICSceneObjectOutlinerRow : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICSceneObjectOutlinerRow) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext, UObject* InSceneObject);

private:
	UFICEditorContext* Context = nullptr;
	UObject* SceneObject = nullptr;

	bool bClick = false;

public:
	// Begin SWidget
	virtual bool IsInteractable() const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget
};

struct FFICSceneObjectReference {
	UObject* SceneObject;

	FFICSceneObjectReference(UObject* InSceneObject = nullptr) : SceneObject(InSceneObject) {}
};

class SFICSceneObjectOutliner : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICSceneObjectOutliner) {}
	SLATE_END_ARGS()
	
public:
	virtual ~SFICSceneObjectOutliner() override;
	
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);
	
private:
	UFICEditorContext* Context = nullptr;
	TSharedPtr<SButton> WidgetAddButton;
	TSharedPtr<SListView<TSharedPtr<FFICSceneObjectReference>>> SceneObjectListWidget;
	FDelegateHandle OnSceneObjectsChangedDelegateHandle;
	FDelegateHandle OnSelectionChangedDelegateHandle;

	TArray<TSharedPtr<FFICSceneObjectReference>> SceneObjectList;
	TMap<UObject*, TSharedPtr<FFICSceneObjectReference>> SceneObjectMap;

	void UpdateSceneObjects();
	void UpdateSelection();
};
