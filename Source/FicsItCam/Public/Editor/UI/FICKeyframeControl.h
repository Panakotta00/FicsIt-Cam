#pragma once

#include "FICEditorStyle.h"
#include "Data/FICTypes.h"
#include "FICKeyframeControl.generated.h"

class FFICEditorAttributeBase;
class UFICEditorContext;
class SFICGraphView;

USTRUCT()
struct FFICKeyframeControlStyle : public FSlateWidgetStyle {
	GENERATED_BODY()
	
	static const FFICKeyframeControlStyle& GetDefault();
	
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		NumericKeyframeIcons.GetResources(OutBrushes);
	}

	UPROPERTY(EditAnywhere)
	FFICNumericKeyframeIcons NumericKeyframeIcons;
	UPROPERTY(EditAnywhere)
	FSlateColor UnsetColor;
	UPROPERTY(EditAnywhere)
	FSlateColor SetColor;
	UPROPERTY(EditAnywhere)
	FSlateColor ChangedColor;
	UPROPERTY(EditAnywhere)
	FSlateColor AnimatedColor;
};

UCLASS(hidecategories = Object, MinimalAPI)
class UFICKeyframeControlStyleContainer : public USlateWidgetStyleContainerBase {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (ShowOnlyInnerProperties))
	FFICKeyframeControlStyle Style;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &Style;
	}
};

class SFICKeyframeControl : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SFICKeyframeControl) : _Style(&FFICKeyframeControlStyle::GetDefault()) {}
		SLATE_ATTRIBUTE(FICFrame, Frame)
		SLATE_STYLE_ARGUMENT(FFICKeyframeControlStyle, Style)
	SLATE_END_ARGS()

public:
	void Construct(FArguments InArgs, UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute);

private:
	TAttribute<FICFrame> Frame;
	const FFICKeyframeControlStyle* Style = nullptr;

	UFICEditorContext* Context = nullptr;
	TSharedPtr<FFICEditorAttributeBase> Attribute;
	
public:
	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	// End SWidget

	FICFrame GetFrame() const;
	TSharedRef<FFICEditorAttributeBase> GetAttribute() const;
};
