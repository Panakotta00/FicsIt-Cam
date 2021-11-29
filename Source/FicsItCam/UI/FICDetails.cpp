#include "FICDetails.h"

#include "FICEditorContext.h"
#include "FICNumericType.h"
#include "FICVectorEditor.h"
#include "FICKeyframeControl.h"
#include "Widgets/Input/SNumericEntryBox.h"

FSlateColorBrush SFICDetails::DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex("030303"));

TSharedRef<SWidget> ScalarAttribute(UFICEditorContext* Context, TFICEditorAttribute<FFICFloatAttribute>& Attr, const FString& Label) {
	return
	SNew(SHorizontalBox)
	+SHorizontalBox::Slot().Padding(5).AutoWidth()[
		SNew(STextBlock)
		.Text(FText::FromString(Label))
	]
	+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
		SNew(SNumericEntryBox<float>)
		.Value_Lambda([&Attr]() -> TOptional<float> {
			return Attr.GetValue();
		})
		.SupportDynamicSliderMaxValue(true)
		.SupportDynamicSliderMinValue(true)
		.SliderExponent(1)
		.Delta(1)
		.MinValue(TOptional<float>())
		.MaxValue(TOptional<float>())
		.MinSliderValue(TOptional<float>())
		.MaxSliderValue(TOptional<float>())
		.LinearDeltaSensitivity(10)
		.AllowSpin(true)
		.OnValueChanged_Lambda([&Attr](float Val) {
			Attr.SetValue(Val);
		})
		.OnValueCommitted_Lambda([&Attr](float Val, auto) {
			Attr.SetValue(Val);
		})
		.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
	]
	+SHorizontalBox::Slot().Padding(5).AutoWidth()[
		SNew(SFICKeyframeControl)
		.Attribute_Lambda([&Attr]() -> FFICEditorAttributeBase* {
			return &Attr;
		})
		.Frame_Lambda([Context]() {
			return Context->GetCurrentFrame();
		})
	];
}

void SFICDetails::Construct(const FArguments& InArgs) {
	Context = InArgs._Context;
	BackgroundBrush = InArgs._Background;

	ChildSlot[
		SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(BackgroundBrush)
		]
		+SOverlay::Slot()[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(STextBlock)
				.Text(FText::FromString("Details:"))
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("Position:"))
				]
				+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
					SNew(SFICVectorEditor)
					.ShowKeyframeControls(true)
					.XAttr_Lambda([this]() {
						return &Context->PosX;
					})
					.YAttr_Lambda([this]() {
						return &Context->PosY;
					})
					.ZAttr_Lambda([this]() {
						return &Context->PosZ;
					})
					.Frame_Lambda([this]() {
						return Context->GetCurrentFrame();
					})
					.AutoKeyframe_Lambda([this]() {
						return Context->bAutoKeyframe;
					})
				]
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("Rotation:"))
				]
				+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
					SNew(SFICVectorEditor)
					.ShowKeyframeControls(true)
					.XAttr_Lambda([this]() {
						return &Context->RotPitch;
					})
					.YAttr_Lambda([this]() {
						return &Context->RotYaw;
					})
					.ZAttr_Lambda([this]() {
						return &Context->RotRoll;
					})
					.AutoKeyframe_Lambda([this]() {
						return Context->bAutoKeyframe;
					})
				]
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("FOV:"))
				]
				+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
					SNew(SNumericEntryBox<float>)
					.Value_Lambda([this]() {
						return Context->FOV.GetValue();
					})
					.SupportDynamicSliderMaxValue(true)
					.SupportDynamicSliderMinValue(true)
					.SliderExponent(1)
					.Delta(1)
					.MinValue(TOptional<float>())
					.MaxValue(TOptional<float>())
					.MinSliderValue(TOptional<float>())
					.MaxSliderValue(TOptional<float>())
					.LinearDeltaSensitivity(10)
					.AllowSpin(true)
					.OnValueChanged_Lambda([this](float Val) {
						Context->FOV.SetValue(Val);
					})
					.OnValueCommitted_Lambda([this](float Val, auto) {
						Context->FOV.SetValue(Val);
					})
					.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
				]
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(SFICKeyframeControl)
					.Attribute_Lambda([this]() {
						return &Context->FOV;
					})
					.Frame_Lambda([this]() {
						return Context->GetCurrentFrame();
					})
				]
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				ScalarAttribute(Context, Context->Aperture, TEXT("Aperture:"))
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				ScalarAttribute(Context, Context->FocusDistance, TEXT("Focus Distance:"))
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
			SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("Settings:"))
				]
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Use Cinematic Camera"))]
						.IsChecked_Lambda([this]() {
							return Context->GetAnimation()->bUseCinematic ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->GetAnimation()->bUseCinematic = State == ECheckBoxState::Checked;
						})
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Bullet Time"))]
						.IsChecked_Lambda([this]() {
							return Context->GetAnimation()->bBulletTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->GetAnimation()->bBulletTime = State == ECheckBoxState::Checked;
						})
					]
				]
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
			SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("Editor:"))
				]
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Lock Viewport Camera"))]
						.IsChecked_Lambda([this]() {
							return Context->bMoveCamera ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->bMoveCamera = State == ECheckBoxState::Checked;
						})
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Show Camera Path"))]
						.IsChecked_Lambda([this]() {
							return Context->bShowPath ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->bShowPath = State == ECheckBoxState::Checked;
						})
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Auto Keyframe"))]
						.IsChecked_Lambda([this]() {
							return Context->bAutoKeyframe ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->bAutoKeyframe = State == ECheckBoxState::Checked;
						})
					]
				]
			]
			+SVerticalBox::Slot().FillHeight(1)[
				SNew(SSpacer)
			]
		]
	];
}
