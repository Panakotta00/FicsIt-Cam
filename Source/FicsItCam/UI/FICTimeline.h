#pragma once

#include "FICGraphView.h"
#include "FICRangeSelector.h"
#include "FICTimelineScrubber.h"
#include "SlateBasics.h"

class UFICEditorContext;

struct FFICEditorAttributeReference {
	FString Name;
	FFICEditorAttributeBase* Attribute;

	FFICEditorAttributeReference(FString Name, FFICEditorAttributeBase* Attribute) : Name(Name), Attribute(Attribute) {}
};

class SFICTimelinePanel : public SCompoundWidget {
	static FSlateColorBrush DefaultBackgroundBrush;
	
	SLATE_BEGIN_ARGS(SFICTimelinePanel) :
		_Background(&DefaultBackgroundBrush) {}
		SLATE_ARGUMENT(UFICEditorContext*, Context)
		SLATE_ATTRIBUTE(const FSlateBrush*, Background)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	UFICEditorContext* Context = nullptr;
	TAttribute<const FSlateBrush*> BackgroundBrush;

	TSharedPtr<SFICRangeSelector> VisibleRange;
	TSharedPtr<SFICTimelineScrubber> Scrubber;
	TSharedPtr<STreeView<TSharedPtr<FFICEditorAttributeReference>>> AttributeTree;
	TSharedPtr<SFICGraphView> Graph;

	int64 ActiveRangeStart = 0;
	int64 ActiveRangeEnd = 0;

	TArray<TSharedPtr<FFICEditorAttributeReference>> Attributes;
	TArray<FFICEditorAttributeBase*> SelectedLeafAttributes;

	void ActiveRangeStartChanged(int64 Prev, int64 Cur);
	void ActiveRangeEndChanged(int64 Prev, int64 Cur);

	void UpdateLeafAttributes();

public:
	// Begin SWidget
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End SWidget
};
