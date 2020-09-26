#include "FICUtils.h"

FRotator NormalizeRotator(FRotator Rot) {
	while (Rot.Pitch > 180.0) Rot.Pitch -= 360.0f;
	while (Rot.Pitch < -180.0) Rot.Pitch += 360.0f;
	while (Rot.Yaw > 180.0) Rot.Yaw -= 360.0f;
	while (Rot.Yaw < -180.0) Rot.Yaw += 360.0f;
	while (Rot.Roll > 180.0) Rot.Roll -= 360.0f;
	while (Rot.Roll < -180.0) Rot.Roll += 360.0f;
	return Rot;
}
