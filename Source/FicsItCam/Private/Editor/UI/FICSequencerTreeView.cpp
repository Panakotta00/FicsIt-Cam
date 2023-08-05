#include "Editor/UI/FICSequencerTreeView.h"

#include "Editor/UI/FICSequencer.h"

void SFICSequencerTreeViewRow::Construct(const FArguments& InArgs,  const TSharedRef<STableViewBase>& Base, TSharedPtr<FFICSequencerRowMeta> InMeta) {
	Meta = InMeta;

	Style = InArgs._Style;
	if (!Style) Style = &FFICSequencerStyle::GetDefault();
	
	STableRow<TSharedPtr<FFICSequencerRowMeta>>::FArguments SuperArgs;
	SuperArgs.Content()[
	SNew(STextBlock)
		.Text(Meta->Name)
	];
	STableRow<TSharedPtr<FFICSequencerRowMeta>>::Construct(SuperArgs, Base);
}

int32 SFICSequencerTreeViewRow::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	const FSlateBrush* BackgroundBrush;
	FLinearColor Color;
	TAttribute<FLinearColor> ColorAttribute;
	if (Meta->Color != FLinearColor::White) ColorAttribute = Meta->Color;
	SFICSequencerRow::GetRowBrushAndColor(IndexInList, ColorAttribute, &Style->RowBackgroundEven, &Style->RowBackgroundOdd, InWidgetStyle, BackgroundBrush, Color);
	
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), BackgroundBrush, ESlateDrawEffect::None, Color);
	
	return STableRow<TSharedPtr<FFICSequencerRowMeta>>::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

int32 SFICSequencerTreeViewRow::GetRowIndex() const {
	return IndexInList;
}

void SFICSequencerTreeView::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;

	Style = InArgs._Style;
	if (!Style) Style = &FFICSequencerStyle::GetDefault();

	OnUpdate = InArgs._OnUpdate;

	OnSceneObjectsChangedDelegate = Context->OnSceneObjectsChanged.AddRaw(this, &SFICSequencerTreeView::UpdateRoot);

	STreeView<TSharedPtr<FFICSequencerRowMeta>>::FArguments SuperArgs;
	SuperArgs.TreeItemsSource(&RootRows);
	SuperArgs.OnGenerateRow_Raw(this, &SFICSequencerTreeView::GenerateRow);
	SuperArgs.OnGetChildren_Raw(this, &SFICSequencerTreeView::GetRowChildren);
	SuperArgs.OnSelectionChanged_Lambda([this](TSharedPtr<FFICSequencerRowMeta>, ESelectInfo::Type) {
		//TriggerUpdate();
	});
	SuperArgs.OnTreeViewScrolled_Lambda([this](double) {
		//TriggerUpdate();
	});
	SuperArgs.OnExpansionChanged_Lambda([this](TSharedPtr<FFICSequencerRowMeta>, bool) {
		//TriggerUpdate();
	});
	SuperArgs.OnRowReleased_Lambda([this](const TSharedRef<ITableRow>&) {
		TriggerUpdate();
	});
	SuperArgs.OnItemScrolledIntoView_Lambda([this](TSharedPtr<FFICSequencerRowMeta>, const TSharedPtr<ITableRow>&) {
		TriggerUpdate();
	});
	STreeView<TSharedPtr<FFICSequencerRowMeta>>::Construct(SuperArgs);

	UpdateRoot();
}

SFICSequencerTreeView::~SFICSequencerTreeView() {
	Context->OnSceneObjectsChanged.Remove(OnSceneObjectsChangedDelegate);
}

void SFICSequencerTreeView::RequestListRefresh() {
	STreeView<TSharedPtr<FFICSequencerRowMeta>>::RequestListRefresh();
	bDirty = true;
}

void SFICSequencerTreeView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	STreeView<TSharedPtr<FFICSequencerRowMeta>>::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (bDirty) {
		TriggerUpdate();
		bDirty = false;
	}
}

void SFICSequencerTreeView::UpdateRoot() {
	RootRows.Empty();
	for (UObject* SceneObject : Context->GetScene()->GetSceneObjects()) {
		const TSharedRef<FFICEditorAttributeBase>& Attribute = Context->GetEditorAttributes()[SceneObject];
		RootRows.Add(MakeShared<FFICSequencerRowMeta>(StaticCastSharedRef<IFICSequencerRowProvider, FFICEditorAttributeBase>(Attribute), FText::FromString(Cast<IFICSceneObject>(SceneObject)->GetSceneObjectName()), FLinearColor::White));
	}
	RebuildList();
}

TArray<TSharedPtr<ITableRow>> SFICSequencerTreeView::GetVisibleTableRows() {
	TArray<TSharedPtr<ITableRow>> Rows;
	for (TTuple<TSharedPtr<FFICSequencerRowMeta>, TSharedPtr<ITableRow>> Row : WidgetGenerator.ItemToWidgetMap) {
		Rows.Add(Row.Value);
	}
	return Rows;
}

int32 SFICSequencerTreeView::GetRowIndex(const TSharedPtr<FFICSequencerRowMeta>& InMeta) {
	return StaticCastSharedPtr<SFICSequencerTreeViewRow>(WidgetFromItem(InMeta))->GetRowIndex();
}

TSharedRef<ITableRow> SFICSequencerTreeView::GenerateRow(TSharedPtr<FFICSequencerRowMeta> Row, const TSharedRef<STableViewBase>& Base) {
	bDirty = true;
	return SNew(SFICSequencerTreeViewRow, Base, Row)
		.Style(Style);
}

void SFICSequencerTreeView::GetRowChildren(TSharedPtr<FFICSequencerRowMeta> InEntry, TArray<TSharedPtr<FFICSequencerRowMeta>>& OutArray) {
	OutArray = InEntry->GetChildren();
}

void SFICSequencerTreeView::TriggerUpdate() {
	bool _ = OnUpdate.ExecuteIfBound();
}
