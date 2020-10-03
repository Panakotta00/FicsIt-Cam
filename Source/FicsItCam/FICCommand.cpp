#include "FICCommand.h"

#include "FICSubsystem.h"
#include "FICUtils.h"

AFICCommand::AFICCommand() {
	bOnlyUsableByPlayer = true;
	MinNumberOfArguments = 1;
	ModId = "FicsItCam";
	CommandName = "fic";
}

#pragma optimize("", off)
EExecutionStatus AFICCommand::ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) {
	AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(this);
	UFICEditorContext* Context = SubSys->GetEditor();
	
	if (Arguments[0] == "list") {
		for (const TPair<FString, AFICAnimation*>& Entry : SubSys->StoredAnimations) {
			Sender->SendChatMessage(Entry.Key);
		}
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "create") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		if (SubSys->StoredAnimations.Contains(Arguments[1])) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' already exists.");
		} else {
			SubSys->StoredAnimations.Add(Arguments[1], GetWorld()->SpawnActor<AFICAnimation>());
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' created.");
		}
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "delete") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		if (SubSys->StoredAnimations.Remove(Arguments[1])) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' deleted.");
			return EExecutionStatus::COMPLETED;
		} else {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::BAD_ARGUMENTS;
		}
	}
	if (Arguments[0] == "edit") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		AFICAnimation** FoundAnimation = SubSys->StoredAnimations.Find(Arguments[1]);
		if (!FoundAnimation) {
			SubSys->SetActiveAnimation(nullptr);
			return EExecutionStatus::COMPLETED;
		} else {
			SubSys->SetActiveAnimation(*FoundAnimation);
			return EExecutionStatus::COMPLETED;
		}
	}
	if (Arguments[0] == "play") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		AFICAnimation** FoundAnimation = SubSys->StoredAnimations.Find(Arguments[1]);
		if (!FoundAnimation) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		SubSys->PlayAnimation(*FoundAnimation);
		Sender->SendChatMessage("Playing Animation '" + Arguments[1] + "'.");
	}
	return EExecutionStatus::BAD_ARGUMENTS;
}
#pragma optimize("", on)