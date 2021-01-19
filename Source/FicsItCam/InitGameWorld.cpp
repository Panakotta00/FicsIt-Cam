#include "InitGameWorld.h"

#include "FICCommand.h"

UInitGameWorldFicsItCam::UInitGameWorldFicsItCam() {
	mChatCommands.Add(AFICCommand::StaticClass());
}
