#include "InitGameInstance.h"

#include "FICSubsystemHolder.h"
#include "FicsItCamModule.h"
#include "Registry/SubsystemHolderRegistry.h"

UInitGameInstanceFicsItCam::UInitGameInstanceFicsItCam() {
	bRootModule = true;
	ModSubsystems.Add(UFICSubsystemHolder::StaticClass());
}
