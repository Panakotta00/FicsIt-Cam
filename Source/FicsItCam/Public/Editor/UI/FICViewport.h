#pragma once

#include "SCompoundWidget.h"
#include "SOverlay.h"

class UFICEditorContext;

class SFICViewport : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICViewport) {}
	SLATE_END_ARGS()

public:
	virtual ~SFICViewport() override;
	
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext, TSharedRef<SWidget> InGameWidget);

private:
	UFICEditorContext* Context = nullptr;
	TSharedPtr<SWidget> GameWidget;
	FDelegateHandle OnViewportOverlaysChangedHandle;

	TSharedPtr<SOverlay> Overlay;

public:
	// Begin SWidget
	virtual bool IsInteractable() const override { return true; }
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	// End SWidget
	
	void UpdateViewportOverlays();
};
