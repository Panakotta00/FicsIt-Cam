#include "InitGameInstance.h"

#define LOCTEXT_NAMESPACE "FicsItCam"

UInitGameInstanceFicsItCam::UInitGameInstanceFicsItCam() {
	bRootModule = true;
	ModAxisBindings.Add(FModAxisBindingInfo{
		TEXT("FicsItCam.MoveForward"),
		FInputAxisKeyMapping("FicsItCam.MoveForward", EKeys::W, 1),
		FInputAxisKeyMapping("FicsItCam.MoveForward", EKeys::S, -1),
		LOCTEXT("MoveForward", "Move Forward in Editor"),
		LOCTEXT("MoveBackward", "Move Backward in Editor"),
	});
	ModAxisBindings.Add(FModAxisBindingInfo{
		TEXT("FicsItCam.MoveRight"),
		FInputAxisKeyMapping("FicsItCam.MoveRight", EKeys::D, 1),
		FInputAxisKeyMapping("FicsItCam.MoveRight", EKeys::A, -1),
		LOCTEXT("MoveRight", "Move Right in Editor"),
		LOCTEXT("MoveLeft", "Move Left in Editor"),
	});
	ModAxisBindings.Add(FModAxisBindingInfo{
		TEXT("FicsItCam.MoveRoll"),
		FInputAxisKeyMapping("FicsItCam.MoveRoll", EKeys::Q, 1),
		FInputAxisKeyMapping("FicsItCam.MoveRoll", EKeys::E, -1),
		LOCTEXT("MoveForward", "Rolls the camera in Editor clockwise"),
		LOCTEXT("MoveBackward", "Rolls the camera in Editor counterclockwise"),
	});
	ModAxisBindings.Add(FModAxisBindingInfo{
		TEXT("FicsItCam.MoveUp"),
		FInputAxisKeyMapping("FicsItCam.MoveUp", EKeys::SpaceBar, 1),
		FInputAxisKeyMapping("FicsItCam.MoveUp", EKeys::LeftAlt, -1),
		LOCTEXT("MoveUp", "Move Up in Editor"),
		LOCTEXT("MoveDown", "Move Down in Editor"),
	});
	ModKeyBindings.Add({
		TEXT("FicsItCam.ChangeFOV"),
		FInputActionKeyMapping(TEXT("FicsItCam.ChangeFOV"), EKeys::LeftShift, false, true),
		FText::FromString(TEXT("Mousewheel -> Change FOV"))
	});
	ModKeyBindings.Add({
		TEXT("FicsItCam.ChangeSpeed"),
		FInputActionKeyMapping(TEXT("FicsItCam.ChangeSpeed"), EKeys::LeftControl),
		FText::FromString(TEXT("Mousewheel -> Change Speed"))
	});
	ModKeyBindings.Add({
		TEXT("FicsItCam.Sprint"),
		FInputActionKeyMapping(TEXT("FicsItCam.Sprint"), EKeys::LeftShift),
		FText::FromString(TEXT("Fast Movement & Scrubbing"))
	});
	ModKeyBindings.Add({
		TEXT("FicsItCam.ToggleAllKeyframes"),
		FInputActionKeyMapping(TEXT("FicsItCam.ToggleAllKeyframes"), EKeys::I),
		FText::FromString(TEXT("Toggle all keyframes"))
	});
	ModKeyBindings.Add({
        TEXT("FicsItCam.NextKeyframe"),
        FInputActionKeyMapping(TEXT("FicsItCam.NextKeyframe"), EKeys::M),
        FText::FromString(TEXT("Goto next keyframe"))
    });
	ModKeyBindings.Add({
        TEXT("FicsItCam.PrevKeyframe"),
        FInputActionKeyMapping(TEXT("FicsItCam.PrevKeyframe"), EKeys::N),
        FText::FromString(TEXT("Goto previous keyframe"))
	});
	ModKeyBindings.Add({
        TEXT("FicsItCam.NextFrame"),
        FInputActionKeyMapping(TEXT("FicsItCam.NextFrame"), EKeys::Right),
        FText::FromString(TEXT("Goto next frame"))
	});
	ModKeyBindings.Add({
        TEXT("FicsItCam.PrevFrame"),
        FInputActionKeyMapping(TEXT("FicsItCam.PrevFrame"), EKeys::Left),
        FText::FromString(TEXT("Goto previous frame"))
	});
	ModKeyBindings.Add({
        TEXT("FicsItCam.ToggleCursor"),
        FInputActionKeyMapping(TEXT("FicsItCam.ToggleCursor"), EKeys::G),
        FText::FromString(TEXT("Toggles the mouse cursor"))
    });
	ModKeyBindings.Add({
		TEXT("FicsItCam.StopAnimation"),
		FInputActionKeyMapping(TEXT("FicsItCam.StopAnimation"), EKeys::C),
		FText::FromString(TEXT("Stops the currently running animation"))
	});
	ModKeyBindings.Add({
		TEXT("FicsItCam.ToggleAutoKeyframe"),
		FInputActionKeyMapping(TEXT("FicsItCam.ToggleAutoKeyframe"), EKeys::K),
		FText::FromString(TEXT("Toggles all keyframes"))
	});
	ModKeyBindings.Add({
		TEXT("FicsItCam.ToggleShowPath"),
		FInputActionKeyMapping(TEXT("FicsItCam.ToggleShowPath"), EKeys::L),
		FText::FromString(TEXT("Shows/Hides the 3D camnera path"))
	});
	ModKeyBindings.Add({
		TEXT("FicsItCam.ToggleLockCamera"),
		FInputActionKeyMapping(TEXT("FicsItCam.ToggleLockCamera"), EKeys::J),
		FText::FromString(TEXT("Enables/Disables view lock to path camera"))
	});
}
