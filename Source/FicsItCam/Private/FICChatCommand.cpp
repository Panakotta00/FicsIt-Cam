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
	MinNumberOfArguments = 0;
	CommandName = "fic";
}

EExecutionStatus AFICChatCommand::ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) {
	if (!Sender->IsPlayerSender() || (Sender->GetPlayer() != GetWorld()->GetFirstPlayerController())) {
		Sender->SendChatMessage("Only Host is allowed to run this command.");
		return EExecutionStatus::INSUFFICIENT_PERMISSIONS;
	}

	AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(this);
	
	if (Arguments.Num() == 0) {
		GetWorldTimerManager().SetTimerForNextTick([SubSys]() {
			SubSys->OpenMenu();
		});
		return EExecutionStatus::COMPLETED;
	}

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
	TFunction<void(TSubclassOf<UFICCommand>)> PrintCommands;
	PrintCommands = [this, Sender, SubSys, &PrintCommands](TSubclassOf<UFICCommand> InCMD) {
		if (InCMD) {
			UFICCommand* Command = InCMD->GetDefaultObject<UFICCommand>();
			if (Command->bFinal) Sender->SendChatMessage(Command->CommandSyntax, FColor::Red);
		}
		const TMap<FString, UFICCommand*>* SubCMDs = SubSys->GetCommands().Find(InCMD);
		if (SubCMDs) for (const TPair<FString, UFICCommand*>& SubCommand : *SubCMDs) {
			PrintCommands(SubCommand.Value->GetClass());
		}
	};
	PrintCommands(CMD);
	return EExecutionStatus::BAD_ARGUMENTS;
}
