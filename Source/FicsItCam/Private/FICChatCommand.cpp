#include "FICChatCommand.h"

#include "FGGameUserSettings.h"
#include "FICSubsystem.h"
#include "Engine/World.h"
#include "FGPlayerController.h"
#include "Command/CommandSender.h"
#include "Command/FICCommand.h"
#include "Data/FICAnimation.h"
#include "Editor/FICEditorSubsystem.h"
#include "Misc/DefaultValueHelper.h"
#include "Runtime/FICTimelapseCamera.h"
#include "Runtime/Process/FICRuntimeProcess.h"
#include "Runtime/Process/FICRuntimeProcessPlayScene.h"
#include "Runtime/Process/FICRuntimeProcessRenderScene.h"

AFICChatCommand::AFICChatCommand() {
	bOnlyUsableByPlayer = true;
	MinNumberOfArguments = 1;
	CommandName = "fic";
}

EExecutionStatus AFICChatCommand::ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) {
	if (!Sender->IsPlayerSender() || (Sender->GetPlayer() != GetWorld()->GetFirstPlayerController())) {
		Sender->SendChatMessage("Only Host is allowed to run this command.");
		return EExecutionStatus::INSUFFICIENT_PERMISSIONS;
	}
	AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(this);

	TArray<FString> Args = Arguments;
	TSubclassOf<UFICCommand> CMD = nullptr;
	UFICCommand* SubCMD = nullptr;
	while (Args.Num() > 0) {
		UFICCommand* const* Command = SubSys->GetCommands()[CMD].Find(Args[0]);
		if (!Command) break;
		Args.RemoveAt(0);
		if ((*Command)->bFinal) {
			SubCMD = *Command;
			break;
		}
		CMD = (*Command)->GetClass();
	}
	if (SubCMD) {
		return SubCMD->ExecuteCommand(Sender, Args);
	}
		
	Sender->SendChatMessage(TEXT("No valid Subcommand!"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic list"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic create <name>"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic delete <name>"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic edit <name>"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic play <name>"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic render <name>"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic timelapse list"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic timelapse create <name> <seconds per frame>"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic timelapse delete <name>"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic timelapse start <name>"), FColor::Red);
	Sender->SendChatMessage(TEXT("/fic timelapse stop <name>"), FColor::Red);
	return EExecutionStatus::BAD_ARGUMENTS;
}
