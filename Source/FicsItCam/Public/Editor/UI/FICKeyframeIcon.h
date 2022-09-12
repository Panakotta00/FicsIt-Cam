#pragma once

#include "FICKeyframeControl.h"
#include "Data/Attributes/FICKeyframe.h"
#include "FICKeyframeIcon.generated.h"

USTRUCT()
struct FFICKeyframeIconStyle : public FSlateWidgetStyle {
	GENERATED_BODY()
	
	static const FFICKeyframeIconStyle& GetDefault();
	
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		Icons.GetResources(OutBrushes);
	}
	
	UPROPERTY(EditAnywhere)
	FSlateColor SelectedColor;
	UPROPERTY(EditAnywhere)
	FSlateColor UnselectedColor;
	UPROPERTY(EditAnywhere)
	FFICNumericKeyframeIcons Icons;
};

UCLASS(hidecategories = Object, MinimalAPI)
class UFICKeyframeIconStyleContainer : public USlateWidgetStyleContainerBase {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (ShowOnlyInnerProperties))
	FFICKeyframeIconStyle Style;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &Style;
	}
};

class SFICKeyframeIcon : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICKeyframeIcon) : _Style(&FFICKeyframeIconStyle::GetDefault()) {}
	SLATE_STYLE_ARGUMENT(FFICKeyframeIconStyle, Style)
	SLATE_ATTRIBUTE(TSharedPtr<FFICKeyframe>, Keyframe)
	SLATE_ATTRIBUTE(bool, IsSelected)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	const FFICKeyframeIconStyle* Style = nullptr;
	TAttribute<TSharedPtr<FFICKeyframe>> Keyframe;
	TAttribute<bool> IsSelected;
};
