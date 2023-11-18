﻿#pragma once

#include "FICKeyframeIcon.h"
#include "Data/FICTypes.h"
#include "FICSequencerRow.generated.h"

class SFICSequencer;
class SFICSequencerRow;
class IFICSequencerRowProvider;
class UFICEditorContext;
class SFICSequencerRowAttribute;
struct FFICAttribute;
class FFICEditorAttributeBase;

USTRUCT()
struct FFICSequencerStyle : public FSlateWidgetStyle {
	GENERATED_BODY()
	
	static const FFICSequencerStyle& GetDefault();
	
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		OutBrushes.Add(&HighlightRangeBrush);
		OutBrushes.Add(&SelectionBoxBrush);
		OutBrushes.Add(&RowBackgroundOdd);
		OutBrushes.Add(&RowBackgroundEven);
		KeyframeIcon.GetResources(OutBrushes);
	}

	UPROPERTY(EditAnywhere)
	FSlateBrush HighlightRangeBrush;
	UPROPERTY(EditAnywhere)
	FLinearColor ActiveFrameColor;
	UPROPERTY(EditAnywhere)
	FLinearColor GridColor;

	UPROPERTY(EditAnywhere)
	FSlateColor KeyframeSelectedColor;
	UPROPERTY(EditAnywhere)
	FSlateColor KeyframeUnselectedColor;
	UPROPERTY(EditAnywhere)
	FSlateBrush SelectionBoxBrush;
	
	UPROPERTY(EditAnywhere)
	FSlateBrush RowBackgroundOdd;
	UPROPERTY(EditAnywhere)
	FSlateBrush RowBackgroundEven;

	UPROPERTY(EditAnywhere)
	FFICKeyframeIconStyle KeyframeIcon;
};

UCLASS(hidecategories = Object, MinimalAPI)
class UFICFFICSequencerStyleContainer : public USlateWidgetStyleContainerBase {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (ShowOnlyInnerProperties))
	FFICSequencerStyle Style;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &Style;
	}
};

/**
 * Combinds a SequencerRowProvider with a name and other metadata
 */
class FFICSequencerRowMeta {
public:
	FFICSequencerRowMeta(TSharedPtr<IFICSequencerRowProvider> InProvider, FText InName, FLinearColor InColor) : Provider(InProvider), Name(InName), Color(InColor) {}
	
	TSharedPtr<IFICSequencerRowProvider> Provider;
	FText Name;
	FLinearColor Color;

	TArray<TSharedPtr<FFICSequencerRowMeta>> GetChildren();
	void ClearCachedChildren();

private:
	TOptional<TArray<TSharedPtr<FFICSequencerRowMeta>>> CachedChildren;
};

/**
 * Can be added to any data type and provides a interface used by the sequencer
 * to create sequencer rows and other data.
 */
class IFICSequencerRowProvider {
public:
	virtual ~IFICSequencerRowProvider() {}

	// Creates a Sequencer Row Widget that will be used for this attribute when shown in sequencer.
	virtual TSharedRef<SFICSequencerRow> CreateRow(SFICSequencer* InSequencer) = 0;

	// Returns the child sequencer rows of this data type
	virtual TArray<TSharedPtr<FFICSequencerRowMeta>> GetChildRows() { return {}; }
};

/**
 * Base Widget Class for all Sequencer Rows.
 * Handles shared responsibilities and common functionality like row coloring.
 */
class SFICSequencerRow : public SPanel {
	SLATE_BEGIN_ARGS(SFICSequencerRow) : _Style(nullptr), _BackgroundColor(FLinearColor::White) {}
	SLATE_STYLE_ARGUMENT(FFICSequencerStyle, Style)
	SLATE_ATTRIBUTE(FLinearColor, BackgroundColor)
	SLATE_END_ARGS()
	
public:
	void Construct(const FArguments& InArgs, SFICSequencer* InSequencer);

protected:
	UFICEditorContext* Context = nullptr;
	SFICSequencer* Sequencer = nullptr;
	
	const FFICSequencerStyle* Style = nullptr;
	TAttribute<FLinearColor> BackgroundColor;

	FICFrame ActiveFrame = 0;
	FFICFrameRange FrameRange;

public:
	SFICSequencerRow();
	
	// Begin SWidget
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	// End SWidget
	
	virtual void UpdateFrameRange(FFICFrameRange InFrameRange);
	virtual void UpdateActiveFrame(FICFrame InFrame);

	virtual TArray<TTuple<FFICAttribute&, FICFrame>> GetKeyframesInBox(const FBox2D& InBox) { return TArray<TTuple<FFICAttribute&, FICFrame>>(); }

	TSharedRef<SFICSequencer> GetSequencer() const;

	static void GetRowBrushAndColor(int32 InIndex, const TAttribute<FLinearColor>& InColorAttribute, const FSlateBrush* InBrushEven, const FSlateBrush* InBrushOdd, const FWidgetStyle& InWidgetStyle, const FSlateBrush*& OutBrush, FLinearColor& OutColor);
};


class SFICSequencerRowAttributeKeyframe : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICSequencerRowAttributeKeyframe) : _Style(nullptr) {}
	SLATE_STYLE_ARGUMENT(FFICSequencerStyle, Style)

	SLATE_ATTRIBUTE(FICFrame, Frame)
	SLATE_ATTRIBUTE(FFICFrameRange, FrameRange)
	SLATE_ATTRIBUTE(FFICFrameRange, FrameHighlightRange)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, SFICSequencerRowAttribute* InRowAttribute, UFICEditorContext* InContext, FFICAttribute* InAttribute, FICFrame InFrame);

private:
	SFICSequencerRowAttribute* RowAttribute = nullptr;
	UFICEditorContext* Context = nullptr;
	FFICAttribute* Attribute = nullptr;
	FICFrame Frame = 0;

	const FFICSequencerStyle* Style = nullptr;
	TAttribute<FICFrame> ActiveFrame;
	TAttribute<FFICFrameRange> FrameRange;
	
public:
	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget

	FFICAttribute* GetAttribute() const { return Attribute; }
	FICFrame GetFrame() const { return Frame; }
};

class SFICSequencerRowAttribute : public SFICSequencerRow {
private:
	SLATE_BEGIN_ARGS(SFICSequencerRowAttribute) : _Style(nullptr) {}
	SLATE_STYLE_ARGUMENT(FFICSequencerStyle, Style)
	SLATE_ATTRIBUTE(FLinearColor, BackgroundColor)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, SFICSequencer* InSequencer, TSharedRef<FFICEditorAttributeBase> InAttribute);

private:
	TSlotlessChildren<SFICSequencerRowAttributeKeyframe> Children;

	TSharedPtr<FFICEditorAttributeBase> Attribute;
	FDelegateHandle DelegateHandle;

public:
	SFICSequencerRowAttribute();
	virtual ~SFICSequencerRowAttribute() override;

	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SWidget

	virtual TArray<TTuple<FFICAttribute&, FICFrame>> GetKeyframesInBox(const FBox2D& InBox) override;

	void UpdateKeyframes();

	FFICAttribute* GetAttribute() const;
		
	FICFrame LocalToFrame(float Local) const;
	double LocalToFrameF(float Local) const;
	float FrameToLocal(FICFrame InFrame) const;
	float GetFramePerLocal() const;
};
