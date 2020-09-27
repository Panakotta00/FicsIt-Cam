#include "FICEditorContext.h"

#pragma optimize("", off)
void UFICEditorContext::ShowEditor() {
	HideEditor();
	
	GameViewport = FSlateApplication::Get().GetGameViewport();
	GameViewportContainer = StaticCastSharedPtr<SHorizontalBox>(GameViewport->GetParentWidget());
	TSharedPtr<SOverlay> GameOverlay = StaticCastSharedPtr<SOverlay>(GameViewportContainer->GetParentWidget());
	GameOverlay->RemoveSlot(GameViewportContainer.ToSharedRef());

	EditorWidget = SNew(SFICEditor)
        .Context(this)
        .GameWidget(GameViewport);
	
	GameOverlay->AddSlot()[
		EditorWidget.ToSharedRef()
	];
}

void UFICEditorContext::HideEditor() {
	if (EditorWidget) {
		GameViewportContainer->RemoveSlot(EditorWidget.ToSharedRef());
		GameViewportContainer->AddSlot()[
			GameViewport.ToSharedRef()
		];
		EditorWidget = nullptr;
	}
}
#pragma optimize("", on)
