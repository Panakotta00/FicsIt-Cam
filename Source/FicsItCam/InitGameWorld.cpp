#include "InitGameWorld.h"
#include "FICCommand.h"
#include "FICSubsystem.h"

UInitGameWorldFicsItCam::UInitGameWorldFicsItCam() {
	bRootModule = true;
	mChatCommands.Add(AFICCommand::StaticClass());
	ModSubsystems.Add(AFICSubsystem::StaticClass());
}
