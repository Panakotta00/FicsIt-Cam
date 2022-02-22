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

void SFICVectorEditor::Construct(FArguments InArgs, UFICEditorContext* InContext, TSharedRef<TFICEditorAttribute<FFICFloatAttribute>> InXAttr, TSharedRef<TFICEditorAttribute<FFICFloatAttribute>> InYAttr, TSharedRef<TFICEditorAttribute<FFICFloatAttribute>> InZAttr) {
	XAttr = InXAttr;
	YAttr = InYAttr;
	ZAttr = InZAttr;
	Frame = InArgs._Frame;
	AutoKeyframe = InArgs._AutoKeyframe;
	Context = InContext;

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
			.Value_Lambda([this]() {
				return XAttr->GetValue();
			})
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
			.OnValueChanged_Lambda([this](float Value) {
				Context->CommitAutoKeyframe((uint8*)this + 1);
				XAttr->SetValue(Value);
				Context->CommitAutoKeyframe(nullptr);
			})
			.OnValueCommitted_Lambda([this](float Value, ETextCommit::Type) {
				Context->CommitAutoKeyframe((uint8*)this + 1);
				XAttr->SetValue(Value);
				Context->CommitAutoKeyframe(nullptr);
			})
			.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
		]
	];
	if (InArgs._ShowKeyframeControls) {
		Holder->AddSlot().AutoWidth()[
            SNew(SFICKeyframeControl, Context, XAttr.ToSharedRef())
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
	        .Value_Lambda([this]() {
		        return YAttr->GetValue();
	        })
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
			.OnValueChanged_Lambda([this](float Value) {
				Context->CommitAutoKeyframe((uint8*)this + 2);
				YAttr->SetValue(Value);
				Context->CommitAutoKeyframe(nullptr);
			})
	        .OnValueCommitted_Lambda([this](float Value, ETextCommit::Type) {
	        	Context->CommitAutoKeyframe((uint8*)this + 2);
		        YAttr->SetValue(Value);
				Context->CommitAutoKeyframe(nullptr);
	        })
			.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
	    ]
	];
	if (InArgs._ShowKeyframeControls) {
		Holder->AddSlot().AutoWidth()[
			SNew(SFICKeyframeControl, Context, YAttr.ToSharedRef())
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
	        .Value_Lambda([this]() {
		        return ZAttr->GetValue();
	        })
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
			.OnValueChanged_Lambda([this](float Value) {
	        	Context->CommitAutoKeyframe((uint8*)this + 3);
				ZAttr->SetValue(Value);
	        	Context->CommitAutoKeyframe(nullptr);
			})
	        .OnValueCommitted_Lambda([this](float Value, ETextCommit::Type) {
	        	Context->CommitAutoKeyframe((uint8*)this + 3);
		        ZAttr->SetValue(Value);
	        	Context->CommitAutoKeyframe(nullptr);
	        })
			.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
	    ]
    ];
	if (InArgs._ShowKeyframeControls) {
		Holder->AddSlot().AutoWidth()[
			SNew(SFICKeyframeControl, Context, ZAttr.ToSharedRef())
            .Frame(Frame)
		];
	}
}
