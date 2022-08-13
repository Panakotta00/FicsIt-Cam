#include "Editor/UI/FICSequencerTreeView.h"

void SFICSequencerTreeView::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;

	OnUpdate = InArgs._OnUpdate;

	STreeView<TSharedPtr<FFICSequencerRowMeta>>::FArguments SuperArgs;
	SuperArgs.TreeItemsSource(&RootRows);
	SuperArgs.OnGenerateRow_Raw(this, &SFICSequencerTreeView::GenerateRow);
	SuperArgs.OnGetChildren_Raw(this, &SFICSequencerTreeView::GetRowChildren);
	SuperArgs.OnTreeViewScrolled(InArgs._OnScrolled);
	SuperArgs.OnSelectionChanged_Lambda([this](TSharedPtr<FFICSequencerRowMeta>, ESelectInfo::Type) {
		OnUpdate.ExecuteIfBound();
	});
	STreeView<TSharedPtr<FFICSequencerRowMeta>>::Construct(SuperArgs);

	UpdateRoot();
}

void SFICSequencerTreeView::UpdateRoot() {
	RootRows.Empty();
	for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Attribute : Context->GetAllAttributes()->GetChildAttributes()) {
		RootRows.Add(MakeShared<FFICSequencerRowMeta>(StaticCastSharedRef<IFICSequencerRowProvider, FFICEditorAttributeBase>(Attribute.Value), FText::FromString(Attribute.Key), FLinearColor::White));
	}
	RebuildList();
	OnUpdate.ExecuteIfBound();
}

TArray<TSharedPtr<ITableRow>> SFICSequencerTreeView::GetVisibleTableRows() {
	TArray<TSharedPtr<ITableRow>> Rows;
	for (TTuple<TSharedPtr<FFICSequencerRowMeta>, TSharedPtr<ITableRow>> Row : WidgetGenerator.ItemToWidgetMap) {
		Rows.Add(Row.Value);
	}
	return Rows;
}

TSharedRef<ITableRow> SFICSequencerTreeView::GenerateRow(TSharedPtr<FFICSequencerRowMeta> Row, const TSharedRef<STableViewBase>& Base) {
	return SNew(STableRow<TSharedPtr<FFICSequencerRowMeta>>, Base)
		.Content()[
			SNew(STextBlock)
			.Text(Row->Name)
		];
}

void SFICSequencerTreeView::GetRowChildren(TSharedPtr<FFICSequencerRowMeta> InEntry, TArray<TSharedPtr<FFICSequencerRowMeta>>& OutArray) {
	OutArray = InEntry->GetChildren();
}
