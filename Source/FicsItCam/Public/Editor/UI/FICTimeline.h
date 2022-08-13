#pragma once

#include "FICGraphView.h"
#include "FICRangeSelector.h"
#include "FICSequencer.h"
#include "FICSequencerTreeView.h"
#include "FICTimelineScrubber.h"
#include "SlateBasics.h"

class UFICEditorContext;

struct FFICEditorAttributeReference {
	FString Name;
	TSharedRef<FFICEditorAttributeBase> Attribute;
	TArray<TSharedPtr<FFICEditorAttributeReference>> Children;
	bool bChildrenLoaded = false;

	FFICEditorAttributeReference(FString Name, TSharedRef<FFICEditorAttributeBase> Attribute) : Name(Name), Attribute(Attribute) {}

	TArray<TSharedPtr<FFICEditorAttributeReference>> GetChildren();
};

class SFICTimelinePanel : public SCompoundWidget {
	static FSlateColorBrush DefaultBackgroundBrush;
	static FCheckBoxStyle DefaultToggleButtonStyle;
	static FSlateBoxBrush DefaultToggleButtonChecked;
	static FSlateBoxBrush DefaultToggleButtonUnchecked;
	
	SLATE_BEGIN_ARGS(SFICTimelinePanel) :
		_Background(&DefaultBackgroundBrush) {}
		SLATE_ATTRIBUTE(const FSlateBrush*, Background)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);

private:
	UFICEditorContext* Context = nullptr;
	TAttribute<const FSlateBrush*> BackgroundBrush;

	int Mode = 0;

	TSharedPtr<SFICRangeSelector> VisibleRange;
	TSharedPtr<SFICTimelineScrubber> Scrubber;
	TSharedPtr<STreeView<TSharedPtr<FFICEditorAttributeReference>>> AttributeTree;
	TSharedPtr<SFICGraphView> Graph;
	
	TSharedPtr<SFICSequencer> Sequencer;
	TSharedPtr<SFICSequencerTreeView> SequencerTreeView;
	
	TArray<TSharedPtr<FFICEditorAttributeReference>> Attributes;
	TArray<TSharedRef<FFICEditorAttributeBase>> SelectedLeafAttributes;

	FFICValueRange ActiveValueRange;

	FDelegateHandle OnSceneObjectsChangedDelegateHandle;
	FDelegateHandle OnSceneSelectedSceneObjectChangedDelegateHandle;
	
	void UpdateLeafAttributes();

public:
	virtual ~SFICTimelinePanel() override;
	
	// Begin SWidget
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End SWidget

	void UpdateEditorAttributes();
	void UpdateEditorAttributeSelection();
};
