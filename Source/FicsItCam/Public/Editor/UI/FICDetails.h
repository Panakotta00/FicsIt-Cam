#pragma once

#include "SlateBasics.h"

class UFICEditorContext;

struct FFICSceneObjectReference {
	UObject* SceneObject;

	FFICSceneObjectReference(UObject* InSceneObject = nullptr) : SceneObject(InSceneObject) {}
};

class SFICDetails : public SCompoundWidget {
	static FSlateColorBrush DefaultBackgroundBrush;
	
	SLATE_BEGIN_ARGS(SFICDetails) :
		_Background(&DefaultBackgroundBrush) {}
		SLATE_ARGUMENT(UFICEditorContext*, Context)
		SLATE_ATTRIBUTE(const FSlateBrush*, Background)
	SLATE_END_ARGS()

public:
	virtual ~SFICDetails() override;
	void Construct(const FArguments& InArgs);

private:
	UFICEditorContext* Context = nullptr;
	TAttribute<const FSlateBrush*> BackgroundBrush;
	SVerticalBox::FSlot* SceneObjectDetailsSlot = nullptr;
	FDelegateHandle OnSceneObjectsChangedDelegateHandle;

	TArray<TSharedPtr<FFICSceneObjectReference>> SceneObjectList;

	void UpdateSceneObjectList();
	void SelectSceneObject(UObject* SceneObject);
};
