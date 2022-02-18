#include "Editor/UI/FICEditorSettings.h"

#include "FICUtils.h"
#include "Editor/FICEditorContext.h"

void SFICEditorSettings::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;

	ChildSlot[
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
	];
}
