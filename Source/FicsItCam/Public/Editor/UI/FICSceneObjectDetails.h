#pragma once

class UFICEditorContext;

class SFICSceneObjectDetails : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICSceneObjectDetails) {}
	SLATE_END_ARGS()

public:
	virtual ~SFICSceneObjectDetails() override;
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);

private:
	UFICEditorContext* Context = nullptr;
	FDelegateHandle OnSelectionChangedDelegateHandle;

	void UpdateSelection();
	void UpdatePanel(UObject* SceneObject);
};
