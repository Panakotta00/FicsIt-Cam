#include "InitGameWorld.h"
#include "FICCommand.h"
#include "FicsItCamModule.h"

UInitGameWorldFicsItCam::UInitGameWorldFicsItCam() {
	bRootModule = true;
	mChatCommands.Add(AFICCommand::StaticClass());
}
