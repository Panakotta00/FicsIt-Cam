#include "FicsItCam/Public/Data/FICAnimation.h"

#include "FGGameUserSettings.h"

AFICAnimation::AFICAnimation() {
	FOV.FallBackValue = 90.0f;
	Aperture.FallBackValue = 100;
	FocusDistance.FallBackValue = 10000;
}

void AFICAnimation::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
}

void AFICAnimation::RecalculateAllKeyframes() {
	PosX.RecalculateAllKeyframes();
	PosY.RecalculateAllKeyframes();
	PosZ.RecalculateAllKeyframes();
	RotYaw.RecalculateAllKeyframes();
	RotPitch.RecalculateAllKeyframes();
	RotRoll.RecalculateAllKeyframes();
	FOV.RecalculateAllKeyframes();
	Aperture.RecalculateAllKeyframes();
	FocusDistance.RecalculateAllKeyframes();
}

float AFICAnimation::GetEndOfAnimation() {
	return (float)AnimationEnd / (float)FPS;
}

float AFICAnimation::GetStartOfAnimation() {
	return (float)AnimationStart / (float)FPS;
}
