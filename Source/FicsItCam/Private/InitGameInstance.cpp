#include "InitGameInstance.h"

#define LOCTEXT_NAMESPACE "FicsItCam"

UInitGameInstanceFicsItCam::UInitGameInstanceFicsItCam() {
	bRootModule = false;
	ModAxisBindings.Add(FModAxisBindingInfo{
		TEXT("FicsItCam.MoveRoll"),
		FInputAxisKeyMapping("FicsItCam.MoveRoll", EKeys::E, 1),
		FInputAxisKeyMapping("FicsItCam.MoveRoll", EKeys::Q, -1),
		LOCTEXT("RollClockwise", "Rolls the camera in Editor clockwise"),
		LOCTEXT("RollCounterclockwise", "Rolls the camera in Editor counterclockwise"),
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
		TEXT("FicsItCam.ToggleAllKeyframes"),
		FInputActionKeyMapping(TEXT("FicsItCam.ToggleAllKeyframes"), EKeys::I),
		FText::FromString(TEXT("Toggle all keyframes"))
	});
	ModKeyBindings.Add({
        TEXT("FicsItCam.NextKeyframe"),
        FInputActionKeyMapping(TEXT("FicsItCam.NextKeyframe"), EKeys::Period),
        FText::FromString(TEXT("Goto next keyframe"))
    });
	ModKeyBindings.Add({
        TEXT("FicsItCam.PrevKeyframe"),
        FInputActionKeyMapping(TEXT("FicsItCam.PrevKeyframe"), EKeys::Comma),
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
	ModKeyBindings.Add({
		TEXT("FicsItCam.Undo"),
		FInputActionKeyMapping(TEXT("FicsItCam.Undo"), EKeys::Z, false, true),
		FText::FromString(TEXT("Undos a previous change you applied."))
	});
	ModKeyBindings.Add({
		TEXT("FicsItCam.Redo"),
		FInputActionKeyMapping(TEXT("FicsItCam.Redo"), EKeys::Y, false, true),
		FText::FromString(TEXT("Redos a change you have previously un-done."))
	});
	ModKeyBindings.Add({
		TEXT("FicsItCam.Grab"),
		FInputActionKeyMapping(TEXT("FicsItCam.Grab"), EKeys::G, false, true),
		FText::FromString(TEXT("Activates the grab tool."))
	});
}
