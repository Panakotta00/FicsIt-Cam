#include "Editor/UI/FICVectorEditor.h"

#include "Data/Attributes/FICAttributeFloat.h"
#include "Editor/FICEditorContext.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Editor/UI/FICKeyframeControl.h"

SFICVectorEditor::SFICVectorEditor() {
	SpinBoxStyle = FCoreStyle::Get().GetWidgetStyle<FSpinBoxStyle>("NumericEntrySpinBox");
	SpinBoxStyle.ActiveFillBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
	SpinBoxStyle.InactiveFillBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
}

void SFICVectorEditor::Construct(FArguments InArgs, UFICEditorContext* InContext) {
	X = InArgs._X;
	Y = InArgs._Y;
	Z = InArgs._Z;
	XAttr = InArgs._XAttr;
	YAttr = InArgs._YAttr;
	ZAttr = InArgs._ZAttr;
	Frame = InArgs._Frame;
	OnXCommitted = InArgs._OnXCommitted;
	OnYCommitted = InArgs._OnYCommitted;
	OnZCommitted = InArgs._OnZCommitted;
	OnXChanged = InArgs._OnXChanged;
	OnYChanged = InArgs._OnYChanged;
	OnZChanged = InArgs._OnZChanged;
	AutoKeyframe = InArgs._AutoKeyframe;
	Context = InContext;

	if (!X.IsSet() && !X.IsBound()) X = TAttribute<TOptional<float>>::Create([this]() {
        if (XAttr.Get()) return TOptional<float>(static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(XAttr.Get())->GetValue());
        return TOptional<float>();
    });
    if (!Y.IsSet() && !Y.IsBound()) Y = TAttribute<TOptional<float>>::Create([this]() {
        if (YAttr.Get()) return TOptional<float>(static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(YAttr.Get())->GetValue());
        return TOptional<float>();
    });
    if (!Z.IsSet() && !Z.IsBound()) Z = TAttribute<TOptional<float>>::Create([this]() {
        if (ZAttr.Get()) return TOptional<float>(static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(ZAttr.Get())->GetValue());
        return TOptional<float>();
    });
    if (!OnXCommitted.IsBound()) OnXCommitted = FFICVectorValueCommitted::CreateLambda([this](float Val, auto) {
		if (XAttr.Get()) {
			auto Attr = static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(XAttr.Get());
			Attr->SetValue(Val);
			if (AutoKeyframe.Get()) Attr->SetKeyframe(Frame.Get().GetValue());
		}
	});
    if (!OnYCommitted.IsBound()) OnYCommitted = FFICVectorValueCommitted::CreateLambda([this](float Val, auto) {
		if (YAttr.Get()) {
			auto Attr = static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(YAttr.Get());
			Attr->SetValue(Val);
			if (AutoKeyframe.Get()) Attr->SetKeyframe(Frame.Get().GetValue());
		}
	});
    if (!OnZCommitted.IsBound()) OnZCommitted = FFICVectorValueCommitted::CreateLambda([this](float Val, auto) {
    	if (ZAttr.Get()) {
			auto Attr = static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(ZAttr.Get());
			Attr->SetValue(Val);
			if (AutoKeyframe.Get()) Attr->SetKeyframe(Frame.Get().GetValue());
		}
	});
	if (!OnXChanged.IsBound()) OnXChanged = FFICVectorValueChanged::CreateLambda([this](float Val) {
		if (XAttr.Get()) {
			auto Attr = static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(XAttr.Get());
			Attr->SetValue(Val);
			if (AutoKeyframe.Get()) Attr->SetKeyframe(Frame.Get().GetValue());
		}
	});
	if (!OnYChanged.IsBound()) OnYChanged = FFICVectorValueChanged::CreateLambda([this](float Val) {
		if (YAttr.Get()) {
			auto Attr = static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(YAttr.Get());
			Attr->SetValue(Val);
			if (AutoKeyframe.Get()) Attr->SetKeyframe(Frame.Get().GetValue());
		}
	});
	if (!OnZChanged.IsBound()) OnZChanged = FFICVectorValueChanged::CreateLambda([this](float Val) {
		if (ZAttr.Get()) {
			auto Attr = static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(ZAttr.Get());
			Attr->SetValue(Val);
			if (AutoKeyframe.Get()) Attr->SetKeyframe(Frame.Get().GetValue());
		}
	});

	TSharedRef<SHorizontalBox> Holder = SNew(SHorizontalBox);
	ChildSlot[Holder];
	Holder->AddSlot()[
		SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(&XBrush)
		]
		+SOverlay::Slot().Padding(5, 0, 0, 0)[
			SNew(SNumericEntryBox<float>)
			.Value(X)
			.SpinBoxStyle(&SpinBoxStyle)
			.SupportDynamicSliderMaxValue(true)
			.SupportDynamicSliderMinValue(true)
			.SliderExponent(1)
			.Delta(1)
			.MinValue(TOptional<float>())
			.MaxValue(TOptional<float>())
			.MinSliderValue(TOptional<float>())
			.MaxSliderValue(TOptional<float>())
	        .LinearDeltaSensitivity(1)
	        .AllowSpin(true)
			.OnValueChanged(OnXChanged)
			.OnValueCommitted(OnXCommitted)
			.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
		]
	];
	if (InArgs._ShowKeyframeControls) {
		Holder->AddSlot().AutoWidth()[
            SNew(SFICKeyframeControl, Context)
                .Attribute(XAttr)
                .Frame(Frame)
        ];
	}
	Holder->AddSlot()[
	    SNew(SOverlay)
	    +SOverlay::Slot()[
	        SNew(SImage)
	        .Image(&YBrush)
	    ]
	    +SOverlay::Slot().Padding(5, 0, 0, 0)[
	        SNew(SNumericEntryBox<float>)
	        .Value(Y)
	        .SpinBoxStyle(&SpinBoxStyle)
	        .SupportDynamicSliderMaxValue(true)
			.SupportDynamicSliderMinValue(true)
			.SliderExponent(1)
	        .Delta(1)
	        .MinValue(TOptional<float>())
	        .MaxValue(TOptional<float>())
	        .MinSliderValue(TOptional<float>())
	        .MaxSliderValue(TOptional<float>())
	        .LinearDeltaSensitivity(1)
	        .AllowSpin(true)
			.OnValueChanged(OnYChanged)
	        .OnValueCommitted(OnYCommitted)
			.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
	    ]
	];
	if (InArgs._ShowKeyframeControls) {
		Holder->AddSlot().AutoWidth()[
			SNew(SFICKeyframeControl, Context)
			.Attribute(YAttr)
			.Frame(Frame)
		];
	}
	Holder->AddSlot()[
	    SNew(SOverlay)
	    +SOverlay::Slot()[
	        SNew(SImage)
	        .Image(&ZBrush)
	    ]
	    +SOverlay::Slot().Padding(5, 0, 0, 0)[
	        SNew(SNumericEntryBox<float>)
	        .Value(Z)
	        .AllowSpin(true)
	        .SpinBoxStyle(&SpinBoxStyle)
	        .SupportDynamicSliderMaxValue(true)
	        .SupportDynamicSliderMinValue(true)
	        .SliderExponent(1)
	        .Delta(1)
			.MinValue(TOptional<float>())
			.MaxValue(TOptional<float>())
			.MinSliderValue(TOptional<float>())
			.MaxSliderValue(TOptional<float>())
	        .LinearDeltaSensitivity(1)
			.OnValueChanged(OnZChanged)
	        .OnValueCommitted(OnZCommitted)
			.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
	    ]
    ];
	if (InArgs._ShowKeyframeControls) {
		Holder->AddSlot().AutoWidth()[
			SNew(SFICKeyframeControl, Context)
            .Attribute(ZAttr)
            .Frame(Frame)
		];
	}
}
