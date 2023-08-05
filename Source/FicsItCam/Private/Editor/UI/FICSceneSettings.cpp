#include "Editor/UI/FICSceneSettings.h"

#include "Editor/FICEditorContext.h"
#include "Widgets/Input/SNumericEntryBox.h"

void SFICSceneSettings::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;

	ChildSlot[
		SNew(SScrollBox)
		+SScrollBox::Slot().Padding(5)[
			SNew(SCheckBox)
			.Content()[SNew(STextBlock).Text(FText::FromString("Use Cinematic Camera"))]
			.IsChecked_Lambda([this]() {
				return Context->GetScene()->bUseCinematic ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
				Context->GetScene()->bUseCinematic = State == ECheckBoxState::Checked;
			})
			.ToolTipText(FText::FromString(TEXT("If enabled, tries to use a more fancy camera which f.e. can do Depth-Of-Field,\ntho it will require more performance hence using it in combination with the play command is not reccomended.")))
		]
		+SScrollBox::Slot().Padding(5)[
			SNew(SCheckBox)
			.Content()[SNew(STextBlock).Text(FText::FromString("Bullet Time"))]
			.IsChecked_Lambda([this]() {
				return Context->GetScene()->bBulletTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
				Context->GetScene()->bBulletTime = State == ECheckBoxState::Checked;
			})
			.ToolTipText(FText::FromString(TEXT("If enabled, game simulation will pause allowing you to have a bullet time effect.")))
		]
		+SScrollBox::Slot().Padding(5)[
			SNew(SCheckBox)
			.Content()[SNew(STextBlock).Text(FText::FromString("Looping"))]
			.IsChecked_Lambda([this]() {
				return Context->GetScene()->bLooping ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
				Context->GetScene()->bLooping = State == ECheckBoxState::Checked;
			})
			.ToolTipText(FText::FromString(TEXT("If enabled, animation will restart automatically at the end of the animation sequence.")))
		]
		+SScrollBox::Slot().Padding(5).HAlign(HAlign_Fill)[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth()[
				SNew(STextBlock).Text(FText::FromString("FPS: "))
			]
			+SHorizontalBox::Slot().FillWidth(1)[
				SNew(SNumericEntryBox<int>)
				.Value_Lambda([this]() {
					return Context->GetScene()->FPS;
				})
				.SupportDynamicSliderMaxValue(true)
				.SliderExponent(1)
				.Delta(1)
				.MinValue(1)
				.LinearDeltaSensitivity(10)
				.AllowSpin(false)
				.OnValueCommitted_Lambda([this](int Val, auto) {
					Context->GetScene()->FPS = FMath::Max(1, Val);
				})
				.TypeInterface(MakeShared<TDefaultNumericTypeInterface<int>>())
			]
		]
		+SScrollBox::Slot().Padding(5).HAlign(HAlign_Fill)[
			SNew(SHorizontalBox)
			.ToolTipText(FText::FromString(TEXT("The resolution setting will be used to determine the aspect ratio and image size for rendering the animation.")))
			+SHorizontalBox::Slot().AutoWidth()[
				SNew(STextBlock).Text(FText::FromString("Resolution: "))
			]
			+SHorizontalBox::Slot().FillWidth(1)[
				SNew(SNumericEntryBox<int>)
				.Value_Lambda([this]() {
					return Context->GetScene()->ResolutionWidth;
				})
				.SupportDynamicSliderMaxValue(true)
				.SliderExponent(1)
				.Delta(1)
				.MinValue(1)
				.LinearDeltaSensitivity(10)
				.AllowSpin(false)
				.OnValueCommitted_Lambda([this](int Value, auto) {
					if (Value % 2 != 0) Value += 1;
					Context->GetScene()->ResolutionWidth = FMath::Max(1, Value);
				})
				.TypeInterface(MakeShared<TDefaultNumericTypeInterface<int>>())
				.ToolTipText(FText::FromString(TEXT("Resolution Width")))
			]
			+SHorizontalBox::Slot().AutoWidth()[
				SNew(STextBlock).Text(FText::FromString(" x "))
			]
			+SHorizontalBox::Slot().FillWidth(1)[
				SNew(SNumericEntryBox<int>)
				.Value_Lambda([this]() {
					return Context->GetScene()->ResolutionHeight;
				})
				.SupportDynamicSliderMaxValue(true)
				.SliderExponent(1)
				.Delta(1)
				.MinValue(1)
				.LinearDeltaSensitivity(10)
				.AllowSpin(false)
				.OnValueCommitted_Lambda([this](int Value, auto) {
					if (Value % 2 != 0) Value += 1;
					Context->GetScene()->ResolutionHeight = FMath::Max(1, Value);
				})
				.TypeInterface(MakeShared<TDefaultNumericTypeInterface<int>>())
				.ToolTipText(FText::FromString(TEXT("Resolution Height")))
			]
		]
		+SScrollBox::Slot().Padding(5).HAlign(HAlign_Fill)[
			SNew(SHorizontalBox)
			.ToolTipText(FText::FromString(TEXT("The sensor size used to adjust the DOF and aspect ration. (only functional with cinematic camera)")))
			+SHorizontalBox::Slot().AutoWidth()[
				SNew(STextBlock).Text(FText::FromString("Sensor Size: "))
			]
			+SHorizontalBox::Slot().FillWidth(1)[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([this]() {
					return Context->GetScene()->SensorDimension.X;
				})
				.SupportDynamicSliderMaxValue(true)
				.SliderExponent(1)
				.Delta(0.1)
				.MinValue(0)
				.LinearDeltaSensitivity(10)
				.AllowSpin(false)
				.OnValueCommitted_Lambda([this](float Val, auto) {
					Context->GetScene()->SensorDimension.X = FMath::Max(0.0f, Val);
				})
				.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
				.ToolTipText(FText::FromString(TEXT("Sensor Width")))
			]
			+SHorizontalBox::Slot().AutoWidth()[
				SNew(STextBlock).Text(FText::FromString(" x "))
			]
			+SHorizontalBox::Slot().FillWidth(1)[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([this]() {
					return Context->GetScene()->SensorDimension.Y;
				})
				.SupportDynamicSliderMaxValue(true)
				.SliderExponent(1)
				.Delta(0.1)
				.MinValue(0)
				.LinearDeltaSensitivity(10)
				.AllowSpin(false)
				.OnValueCommitted_Lambda([this](float Val, auto) {
					Context->GetScene()->SensorDimension.Y = FMath::Max(0.0f, Val);
				})
				.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
				.ToolTipText(FText::FromString(TEXT("Sensor Height")))
			]
		]
	];
}
