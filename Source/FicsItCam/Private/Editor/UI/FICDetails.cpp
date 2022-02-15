#include "Editor/UI/FICDetails.h"

#include "FICUtils.h"
#include "Data/Attributes/FICAttributeFloat.h"
#include "Data/Objects/FICSceneObject.h"
#include "Editor/FICEditorContext.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Editor/UI/FICKeyframeControl.h"
#include "Editor/UI/FICVectorEditor.h"
#include "Widgets/Layout/SExpandableArea.h"

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
		SNew(SFICKeyframeControl, Context)
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

	TSharedPtr<SVerticalBox> Outliner;

	ChildSlot[
		SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(BackgroundBrush)
		]
		+SOverlay::Slot()[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)[
				SAssignNew(Outliner, SVerticalBox)
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("Settings: "))
				]
				+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
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
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
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
					+SVerticalBox::Slot().Padding(5).AutoHeight().HAlign(HAlign_Fill)[
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
					+SVerticalBox::Slot().Padding(5).AutoHeight().HAlign(HAlign_Fill)[
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
							.OnValueCommitted_Lambda([this](int Val, auto) {
								Context->GetScene()->ResolutionWidth = FMath::Max(1, Val);
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
							.OnValueCommitted_Lambda([this](int Val, auto) {
								Context->GetScene()->ResolutionHeight = FMath::Max(1, Val);
							})
							.TypeInterface(MakeShared<TDefaultNumericTypeInterface<int>>())
							.ToolTipText(FText::FromString(TEXT("Resolution Height")))
						]
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight().HAlign(HAlign_Fill)[
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
						.ToolTipText(FText::FromString(FString::Printf(TEXT("If enabled, the viewport camera will be locked to the virtual camera for the animation,\nthis allows (if disabled) to move the camera on path without changing the viewport camera view/orientation.\n\n%s"), *UFICUtils::KeymappingToString("FicsItCam.ToggleLockCamera"))))
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
						.ToolTipText(FText::FromString(FString::Printf(TEXT("If enabled, a camera path will be drawn into the world that shows how the camera moves through space,\nit additionally shows the camera orientation at the current frame.\n\n%s"), *UFICUtils::KeymappingToString("FicsItCam.ToggleShowPath"))))
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
						.ToolTipText(FText::FromString(FString::Printf(TEXT("If enabled, a change of value of a attribute will directly cause it to set/create a keyframe for that attribute at the current frame.\n\n%s"), *UFICUtils::KeymappingToString("FicsItCam.ToggleAutoKeyframe"))))
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Force Resolution"))]
						.IsChecked_Lambda([this]() {
							return Context->bForceResolution ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->bForceResolution = State == ECheckBoxState::Checked;
						})
						.ToolTipText(FText::FromString(FString::Printf(TEXT("If enabled, viewport will be forced to use the aspect ratio of the resolution of the animation, causing black bars to appear."))))
					]
				]
			]
			+SVerticalBox::Slot().FillHeight(1)[
				SNew(SSpacer)
			]
		]
	];
	
	for (UObject* Object : Context->GetScene()->GetSceneObjects()) {
		Outliner->AddSlot().HAlign(HAlign_Fill).AutoHeight()[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Fuck You!")))
		];
		Outliner->AddSlot().HAlign(HAlign_Fill).AutoHeight()[
			SNew(SExpandableArea)
			.HeaderContent()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().FillWidth(1).HAlign(HAlign_Center).VAlign(VAlign_Center)[
					SNew(STextBlock)
					.Text(Cast<IFICSceneObject>(Object)->GetSceneObjectName())
				]
				+SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(5)[
					SNew(SFICKeyframeControl, Context)
					.Attribute(&*Context->GetEditorAttributes()[Object])
					.Frame_Lambda([this]() {
						return Context->GetCurrentFrame();
					})
				]
			]
			.BodyContent()[
				Context->GetEditorAttributes()[Object]->CreateDetailsWidget(Context)
			]
		];
	}
}
