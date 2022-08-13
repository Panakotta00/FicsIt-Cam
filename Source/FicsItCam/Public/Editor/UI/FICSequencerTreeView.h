#pragma once

#include "FICSequencerRow.h"
#include "Components/ScrollBox.h"
#include "Editor/FICEditorContext.h"
#include "Widgets/Views/STreeView.h"

DECLARE_DELEGATE(FFICSequencerUpdate)

class SFICSequencerTreeView : public STreeView<TSharedPtr<FFICSequencerRowMeta>> {
	SLATE_BEGIN_ARGS(SFICSequencerTreeView) {}
	SLATE_EVENT(FOnTableViewScrolled, OnScrolled)
	SLATE_EVENT(FFICSequencerUpdate, OnUpdate)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);

private:
	UFICEditorContext* Context = nullptr;

	FFICSequencerUpdate OnUpdate;

	TArray<TSharedPtr<FFICSequencerRowMeta>> RootRows;

public:
	FOnTableViewScrolled OnScrolled;
	
	void UpdateRoot();

	TArray<TSharedPtr<ITableRow>> GetVisibleTableRows();

private:
	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FFICSequencerRowMeta> Row, const TSharedRef<STableViewBase>& Base);
	void GetRowChildren(TSharedPtr<FFICSequencerRowMeta> InEntry, TArray<TSharedPtr<FFICSequencerRowMeta>>& OutArray);
};
