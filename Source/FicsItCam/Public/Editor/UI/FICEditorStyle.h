#pragma once

#include "SlateBasics.h"
#include "FICEditorStyle.generated.h"

class FFICEditorStyles {
public:
	static void Initialize();
	static void Shutdown();
	static const class ISlateStyle& Get();
	static FName GetStyleSetName();

private:
	static TSharedRef<FSlateStyleSet> Create(); 
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};

USTRUCT()
struct FFICNumericKeyframeIcons : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFICNumericKeyframeIcons& GetDefault();
	
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		OutBrushes.Add(&AutoBrush);
		OutBrushes.Add(&MirrorBrush);
		OutBrushes.Add(&CustomBrush);
		OutBrushes.Add(&LinearBrush);
		OutBrushes.Add(&StepBrush);
		OutBrushes.Add(&EaseInOutBrush);
		OutBrushes.Add(&DefaultBrush);
		OutBrushes.Add(&HandleBrush);
	}

	UPROPERTY(EditAnywhere)
	FSlateBrush DefaultBrush;
	UPROPERTY(EditAnywhere)
	FSlateBrush AutoBrush;
	UPROPERTY(EditAnywhere)
	FSlateBrush MirrorBrush;
	UPROPERTY(EditAnywhere)
	FSlateBrush CustomBrush;
	UPROPERTY(EditAnywhere)
	FSlateBrush LinearBrush;
	UPROPERTY(EditAnywhere)
	FSlateBrush StepBrush;
	UPROPERTY(EditAnywhere)
	FSlateBrush EaseInOutBrush;
	
	UPROPERTY(EditAnywhere)
	FSlateBrush HandleBrush;
};

UCLASS(hidecategories = Object, MinimalAPI)
class UFICNumericKeyframeIconsContainer : public USlateWidgetStyleContainerBase {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (ShowOnlyInnerProperties))
	FFICNumericKeyframeIcons Style;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &Style;
	}
};
