#include "FICCommand.h"

#include "FICSubsystem.h"
#include "Engine/World.h"
#include "FGPlayerController.h"
#include "Command/CommandSender.h"

AFICCommand::AFICCommand() {
	bOnlyUsableByPlayer = true;
	MinNumberOfArguments = 1;
	CommandName = "fic";
}

EExecutionStatus AFICCommand::ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) {
	if (!Sender->IsPlayerSender() || (Sender->GetPlayer() != GetWorld()->GetFirstPlayerController())) {
		Sender->SendChatMessage("Only Host is allowed to run this command.");
		return EExecutionStatus::INSUFFICIENT_PERMISSIONS;
	}
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
			FVector Pos = Sender->GetPlayer()->PlayerCameraManager->GetCameraLocation();
			FRotator Rot = Sender->GetPlayer()->PlayerCameraManager->GetCameraRotation();
			float FOV = Sender->GetPlayer()->PlayerCameraManager->GetFOVAngle();
			FActorSpawnParameters Params;
			Params.Name = FName(Arguments[1]);
			AFICAnimation* Anim = GetWorld()->SpawnActor<AFICAnimation>(Params);
			Anim->PosX.SetDefaultValue(Pos.X);
			Anim->PosY.SetDefaultValue(Pos.Y);
			Anim->PosZ.SetDefaultValue(Pos.Z);
			Anim->RotPitch.SetDefaultValue(Rot.Pitch);
			Anim->RotYaw.SetDefaultValue(Rot.Yaw);
			Anim->RotRoll.SetDefaultValue(Rot.Roll);
			Anim->FOV.SetDefaultValue(FOV);
			SubSys->StoredAnimations.Add(Arguments[1], Anim);
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' created.");
		}
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "delete") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		AFICAnimation** Anim = SubSys->StoredAnimations.Find(Arguments[1]);
		if (SubSys->StoredAnimations.Remove(Arguments[1])) {
			if (Anim) (*Anim)->Destroy();
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
	if (Arguments[0] == "render") {
		if (Arguments.Num() < 2) return EExecutionStatus::BAD_ARGUMENTS;
		AFICAnimation** FoundAnimation = SubSys->StoredAnimations.Find(Arguments[1]);
		if (!FoundAnimation) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		SubSys->PlayAnimation(*FoundAnimation, true);
		Sender->SendChatMessage("Rendering Animation '" + Arguments[1] + "'.");
	}
	return EExecutionStatus::BAD_ARGUMENTS;
}
