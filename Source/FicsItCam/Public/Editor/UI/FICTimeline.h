#pragma once

#include "FICGraphView.h"
#include "FICRangeSelector.h"
#include "FICSequencer.h"
#include "FICSequencerTreeView.h"
#include "FICTimelineScrubber.h"
#include "SlateBasics.h"
#include "FICTimeline.generated.h"

class UFICEditorContext;

USTRUCT()
struct FFICTimelineStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFICTimelineStyle& GetDefault();
	
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		OutBrushes.Add(&GraphViewIcon);
		OutBrushes.Add(&SequencerIcon);
	}

	UPROPERTY(EditAnywhere)
	FSlateBrush GraphViewIcon;
	UPROPERTY(EditAnywhere)
	FSlateBrush SequencerIcon;
};

UCLASS(hidecategories = Object, MinimalAPI)
class UFICTimelineStyleContainer : public USlateWidgetStyleContainerBase {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (ShowOnlyInnerProperties))
	FFICTimelineStyle Style;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &Style;
	}
};

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
		_Style(&FFICTimelineStyle::GetDefault()),
		_Background(&DefaultBackgroundBrush) {}
		SLATE_STYLE_ARGUMENT(FFICTimelineStyle, Style)
		SLATE_ATTRIBUTE(const FSlateBrush*, Background)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);

private:
	UFICEditorContext* Context = nullptr;
	const FFICTimelineStyle* Style = nullptr;
	TAttribute<const FSlateBrush*> BackgroundBrush;

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
