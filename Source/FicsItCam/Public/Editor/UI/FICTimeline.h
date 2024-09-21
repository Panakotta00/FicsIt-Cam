#pragma once

#include "CoreMinimal.h"
#include "FICGraphView.h"
#include "FICRangeSelector.h"
#include "FICSequencer.h"
#include "FICSequencerTreeView.h"
#include "FICTimelineScrubber.h"

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
	SLATE_BEGIN_ARGS(SFICTimelinePanel) :
		_Style(&FFICEditorStyles::Get().GetWidgetStyle<FFICTimelineStyle>(TEXT("Timeline"))) {}
		SLATE_STYLE_ARGUMENT(FFICTimelineStyle, Style)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);

private:
	UFICEditorContext* Context = nullptr;
	const FFICTimelineStyle* Style = nullptr;

	int Mode = 1;

	TSharedPtr<SFICRangeSelector> VisibleRange;
	TSharedPtr<SFICTimelineScrubber> Scrubber;
	TSharedPtr<STreeView<TSharedPtr<FFICEditorAttributeReference>>> AttributeTree;
	TSharedPtr<SFICGraphView> Graph;
	TSharedPtr<SWidgetSwitcher> Switcher;
	
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
	virtual bool SupportsKeyboardFocus() const override;
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;
	// End SWidget

	void UpdateEditorAttributes();
	void UpdateEditorAttributeSelection();
};
