#include "InitGameWorld.h"
#include "FICCommand.h"

UInitGameWorldFicsItCam::UInitGameWorldFicsItCam() {
	bRootModule = true;
	mChatCommands.Add(AFICCommand::StaticClass());
}
