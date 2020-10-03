#include "FICVectorEditor.h"

#include "FICKeyframeControl.h"
#include "FICNumericType.h"
#include "SNumericEntryBox.h"

void SFICVectorEditor::Construct(FArguments InArgs) {
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
		if (XAttr.Get()) static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(XAttr.Get())->SetValue(Val);
	});
    if (!OnYCommitted.IsBound()) OnYCommitted = FFICVectorValueCommitted::CreateLambda([this](float Val, auto) {
		if (YAttr.Get()) static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(YAttr.Get())->SetValue(Val);
	});
    if (!OnZCommitted.IsBound()) OnZCommitted = FFICVectorValueCommitted::CreateLambda([this](float Val, auto) {
		if (ZAttr.Get()) static_cast<TFICEditorAttribute<FFICFloatAttribute>*>(ZAttr.Get())->SetValue(Val);
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
			.OnValueChanged(OnXChanged)
			.OnValueCommitted(OnXCommitted)
			.TypeInterface(MakeShared<TFICNumericTypeInterface<float>>())
		]
	];
	if (InArgs._ShowKeyframeControls) {
		Holder->AddSlot().AutoWidth()[
            SNew(SFICKeyframeControl)
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
			.OnValueChanged(OnYChanged)
	        .OnValueCommitted(OnYCommitted)
			.TypeInterface(MakeShared<TFICNumericTypeInterface<float>>())
	    ]
	];
	if (InArgs._ShowKeyframeControls) {
		Holder->AddSlot().AutoWidth()[
			SNew(SFICKeyframeControl)
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
			.OnValueChanged(OnZChanged)
	        .OnValueCommitted(OnZCommitted)
			.TypeInterface(MakeShared<TFICNumericTypeInterface<float>>())
	    ]
    ];
	if (InArgs._ShowKeyframeControls) {
		Holder->AddSlot().AutoWidth()[
			SNew(SFICKeyframeControl)
            .Attribute(ZAttr)
            .Frame(Frame)
		];
	}
}
