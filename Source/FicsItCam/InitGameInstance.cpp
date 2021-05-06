#include "InitGameInstance.h"

UInitGameInstanceFicsItCam::UInitGameInstanceFicsItCam() {
	bRootModule = true;
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
}
