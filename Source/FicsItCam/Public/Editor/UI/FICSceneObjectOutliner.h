#pragma once

class UFICEditorContext;

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

	void OpenAddSceneObjectMenu();
};
