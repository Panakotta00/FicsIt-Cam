#pragma once

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
	void UpdateViewportOverlays();
};
