#include "Editor/UI/FICViewport.h"

#include "Editor/FICEditorContext.h"

SFICViewport::~SFICViewport() {
	if (Context) Context->OnOverlayWidgetsChanged.Remove(OnViewportOverlaysChangedHandle);
}

void SFICViewport::Construct(const FArguments& InArgs, UFICEditorContext* InContext, TSharedRef<SWidget> InGameWidget) {
	Context = InContext;
	GameWidget = InGameWidget;

	ChildSlot[
		SAssignNew(Overlay, SOverlay)
	];

	OnViewportOverlaysChangedHandle = Context->OnOverlayWidgetsChanged.AddSP(this, &SFICViewport::UpdateViewportOverlays);

	UpdateViewportOverlays();
}

void SFICViewport::UpdateViewportOverlays() {
	Overlay->ClearChildren();
	Overlay->AddSlot()[
		GameWidget.ToSharedRef()
	];
	for (TSharedRef<SWidget> Widget : Context->GetOverlayWidgets()) {
		Overlay->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)[
			Widget
		];
	}
}
