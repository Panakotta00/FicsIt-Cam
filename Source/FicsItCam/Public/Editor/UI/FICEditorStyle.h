#pragma once

#include "CoreMinimal.h"
#include "SlateStyle.h"
#include "SlateWidgetStyle.h"
#include "SlateWidgetStyleContainerBase.h"
#include "Styling/SlateTypes.h"
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

USTRUCT()
struct FFICGraphViewStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFICGraphViewStyle& GetDefault();

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		NumericKeyframeIcons.GetResources(OutBrushes);
		OutBrushes.Add(&SelectionBoxBrush);
		OutBrushes.Add(&HighlightRangeBrush);
	}

	UPROPERTY(EditAnywhere)
	FFICNumericKeyframeIcons NumericKeyframeIcons;
	UPROPERTY(EditAnywhere)
	FSlateColor KeyframeSelectedColor;
	UPROPERTY(EditAnywhere)
	FSlateColor KeyframeUnselectedColor;
	UPROPERTY(EditAnywhere)
	FSlateBrush SelectionBoxBrush;
	UPROPERTY(EditAnywhere)
	FSlateBrush HighlightRangeBrush;
};

UCLASS(MinimalAPI)
class UFICGraphViewStyleContainer : public USlateWidgetStyleContainerBase {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FFICGraphViewStyle Style;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &Style;
	}
};

USTRUCT()
struct FFICRangeSelectorStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFICRangeSelectorStyle& GetDefault();

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; }

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		OutBrushes.Add(&BackgroundBrush);
		OutBrushes.Add(&SelectBrush);
	}

	UPROPERTY(EditAnywhere)
	FSlateBrush BackgroundBrush = static_cast<FSlateBrush>(FSlateColorBrush(FColor::FromHex(TEXT("030303"))));

	UPROPERTY(EditAnywhere)
	FLinearColor RangeIncrementColor = FColor::FromHex(TEXT("404040"));

	UPROPERTY(EditAnywhere)
	FSlateBrush SelectBrush = static_cast<FSlateBrush>(FSlateColorBrush(FColor::FromHex(TEXT("80808088"))));

	UPROPERTY(EditAnywhere)
	FLinearColor SelectHandleColor = FColor::FromHex(TEXT("A0A0A0"));

	UPROPERTY(EditAnywhere)
	FLinearColor HighlightColor = FColor::FromHex(TEXT("FF8500"));
};

USTRUCT()
struct FFICVectorEditStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFICVectorEditStyle& GetDefault();

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; }

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		SpinBoxStyle.GetResources(OutBrushes);
		OutBrushes.Add(&CoordinateBrush);
	}

	FFICVectorEditStyle();

	UPROPERTY(EditAnywhere)
	FSlateBrush CoordinateBrush = static_cast<FSlateBrush>(FSlateColorBrush(FLinearColor::White));

	UPROPERTY(EditAnywhere)
	FLinearColor XColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere)
	FLinearColor YColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere)
	FLinearColor ZColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere)
	FSpinBoxStyle SpinBoxStyle;
};

USTRUCT()
struct FFICTimelineStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFICTimelineStyle& GetDefault();

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		ForwardPlayButtonStyle.GetResources(OutBrushes);
		ReversePlayButtonStyle.GetResources(OutBrushes);
		AttributeTreeScrollBarStyle.GetResources(OutBrushes);
		AttributeTreeStyle.GetResources(OutBrushes);
		OutBrushes.Add(&Background);
		OutBrushes.Add(&GraphViewIcon);
		OutBrushes.Add(&SequencerIcon);
	}

	UPROPERTY(EditAnywhere)
	FSlateBrush Background = static_cast<FSlateBrush>(FSlateNoResource());

	UPROPERTY(EditAnywhere)
	FSlateBrush GraphViewIcon;

	UPROPERTY(EditAnywhere)
	FSlateBrush SequencerIcon;

	UPROPERTY(EditAnywhere)
	FCheckBoxStyle ForwardPlayButtonStyle;

	UPROPERTY(EditAnywhere)
	FCheckBoxStyle ReversePlayButtonStyle;

	UPROPERTY(EditAnywhere)
	FScrollBarStyle AttributeTreeScrollBarStyle;

	UPROPERTY(EditAnywhere)
	FTableViewStyle AttributeTreeStyle;

	UPROPERTY(EditAnywhere)
	FTableRowStyle AttributeTreeRowStyle;
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

USTRUCT()
struct FFICEditorStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFICEditorStyle& GetDefault();

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override {
		OutBrushes.Add(&Background);
	}

	UPROPERTY(EditAnywhere)
	FSlateBrush Background = static_cast<FSlateBrush>(FSlateColorBrush(FColor::FromHex("030303")));
};

UCLASS()
class UFICEditorStyleContainer : public USlateWidgetStyleContainerBase {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FFICEditorStyle Style;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override {
		return &Style;
	}
};
