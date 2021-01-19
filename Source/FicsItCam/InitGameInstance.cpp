#include "InitGameInstance.h"

#include "FICSubsystemHolder.h"

UInitGameInstanceFicsItCam::UInitGameInstanceFicsItCam() {
	ModSubsystems.Add(UFICSubsystemHolder::StaticClass());
}
