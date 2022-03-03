#include "Editor/UI/FICSceneObjectDetails.h"

#include "Editor/FICEditorContext.h"

SFICSceneObjectDetails::~SFICSceneObjectDetails() {
	if (Context) Context->OnSceneObjectSelectionChanged.Remove(OnSelectionChangedDelegateHandle);
}

void SFICSceneObjectDetails::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;

	OnSelectionChangedDelegateHandle = Context->OnSceneObjectSelectionChanged.AddSP(this, &SFICSceneObjectDetails::UpdateSelection);

	UpdateSelection();
}

void SFICSceneObjectDetails::UpdateSelection() {
	UpdatePanel(Context->GetSelectedSceneObject());
}

void SFICSceneObjectDetails::UpdatePanel(UObject* SceneObject) {
	if (!SceneObject) {
		ChildSlot.DetachWidget();
		return;
	}
	ChildSlot.AttachWidget(Cast<IFICSceneObject>(SceneObject)->CreateDetailsWidget(Context));
}
