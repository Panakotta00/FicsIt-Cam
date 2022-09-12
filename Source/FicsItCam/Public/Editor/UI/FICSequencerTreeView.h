#pragma once

#include "FICSequencerRow.h"
#include "Components/ScrollBox.h"
#include "Editor/FICEditorContext.h"
#include "Widgets/Views/STreeView.h"

DECLARE_DELEGATE(FFICSequencerUpdate)

class SFICSequencerTreeViewRow : public STableRow<TSharedPtr<FFICSequencerRowMeta>> {
	SLATE_BEGIN_ARGS(SFICSequencerTreeViewRow) : _Style(nullptr) {}
	SLATE_STYLE_ARGUMENT(FFICSequencerStyle, Style)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs,  const TSharedRef<STableViewBase>& Base, TSharedPtr<FFICSequencerRowMeta> InMeta);

private:
	TSharedPtr<FFICSequencerRowMeta> Meta;

	const FFICSequencerStyle* Style = nullptr;

public:
	// Begin SWidget
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	// End SWidget
	
	int32 GetRowIndex() const;
	
/*
	// Begin ITableRow
public:
	virtual void InitializeRow() override;
	virtual void ResetRow() override;
	virtual void SetIndexInList(int32 InIndexInList) override;
	virtual bool IsItemExpanded() const override;
	virtual void ToggleExpansion() override;
	virtual bool IsItemSelected() const override;
	virtual int32 GetIndentLevel() const override;
	virtual int32 DoesItemHaveChildren() const override;
	virtual TBitArray<> GetWiresNeededByDepth() const override;
	virtual bool IsLastChild() const override;
	virtual TSharedRef<SWidget> AsWidget() override;
	virtual TSharedPtr<SWidget> GetContent() override;
	virtual void Private_OnExpanderArrowShiftClicked() override;
	virtual FVector2D GetRowSizeForColumn(const FName& InColumnName) const override;
protected:
	virtual ESelectionMode::Type GetSelectionMode() const override;
	// End ITableRow
*/
};

class SFICSequencerTreeView : public STreeView<TSharedPtr<FFICSequencerRowMeta>> {
	SLATE_BEGIN_ARGS(SFICSequencerTreeView) : _Style(nullptr) {}
	SLATE_STYLE_ARGUMENT(FFICSequencerStyle, Style)
	SLATE_EVENT(FOnTableViewScrolled, OnScrolled)
	SLATE_EVENT(FFICSequencerUpdate, OnUpdate)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);

private:
	UFICEditorContext* Context = nullptr;

	const FFICSequencerStyle* Style = nullptr;

	FFICSequencerUpdate OnUpdate;

	TArray<TSharedPtr<FFICSequencerRowMeta>> RootRows;

	bool bDirty = false;

public:
	FOnTableViewScrolled OnScrolled;

	// Begin STreeView
	virtual void RequestListRefresh() override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End STreeView
	
	void UpdateRoot();

	TArray<TSharedPtr<ITableRow>> GetVisibleTableRows();
	int32 GetRowIndex(const TSharedPtr<FFICSequencerRowMeta>& InMeta);

private:
	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FFICSequencerRowMeta> Row, const TSharedRef<STableViewBase>& Base);
	void GetRowChildren(TSharedPtr<FFICSequencerRowMeta> InEntry, TArray<TSharedPtr<FFICSequencerRowMeta>>& OutArray);

	void TriggerUpdate();
};
