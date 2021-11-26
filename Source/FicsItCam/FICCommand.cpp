#include "FICCommand.h"

#include "FICSubsystem.h"
#include "Engine/World.h"
#include "FGPlayerController.h"
#include "FICTimelapseCamera.h"
#include "Command/CommandSender.h"
#include "Misc/DefaultValueHelper.h"

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
		return EExecutionStatus::COMPLETED;
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
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "timelapse") {
		if (Arguments.Num() < 2) {
			for (TPair<FString, AFICTimelapseCamera*> Entry : SubSys->TimelapseCameras) {
				Sender->SendChatMessage(FString::Printf(TEXT("%s-Timelapse: %fs"), *Entry.Key, Entry.Value->Frequency));
			}
			return EExecutionStatus::COMPLETED;
		} else if (Arguments[1] == "create") {
			if (Arguments.Num() < 4) return EExecutionStatus::BAD_ARGUMENTS;
			FString CameraName = FString::Printf(TEXT("%s-Timelapse"), *Arguments[2]);
			if (SubSys->TimelapseCameras.Contains(CameraName)) {
				Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' already exists.");
				return EExecutionStatus::UNCOMPLETED;
			} else {
				float Frequency;
				if (!FDefaultValueHelper::ParseFloat(Arguments[3], Frequency)) return EExecutionStatus::BAD_ARGUMENTS;
				FVector Pos = Sender->GetPlayer()->PlayerCameraManager->GetCameraLocation();
				FRotator Rot = Sender->GetPlayer()->PlayerCameraManager->GetCameraRotation();
				FActorSpawnParameters Params;
				Params.Name = FName(CameraName);
				AFICTimelapseCamera* Camera = GetWorld()->SpawnActor<AFICTimelapseCamera>(Pos, Rot, Params);
				SubSys->TimelapseCameras.Add(CameraName, Camera);
				Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' created.");
			}
			return EExecutionStatus::COMPLETED;
		} else if (Arguments[1] == "delete") {
			if (Arguments.Num() < 3) return EExecutionStatus::BAD_ARGUMENTS;
			FString CameraName = FString::Printf(TEXT("%s-Timelapse"), *Arguments[2]);
			AFICTimelapseCamera** Camera = SubSys->TimelapseCameras.Find(CameraName);
			if (!Camera) {
				Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' doesn't exists.");
				return EExecutionStatus::UNCOMPLETED;
			} else {
				(*Camera)->Destroy();
				SubSys->TimelapseCameras.Remove(CameraName);
				Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' deleted.");
				return EExecutionStatus::COMPLETED;
			}
		} else if (Arguments[1] == "start") {
			if (Arguments.Num() < 3) return EExecutionStatus::BAD_ARGUMENTS;
			FString CameraName = FString::Printf(TEXT("%s-Timelapse"), *Arguments[2]);
			if (!SubSys->TimelapseCameras.Contains(CameraName)) {
				Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' doesn't exists.");
				return EExecutionStatus::UNCOMPLETED;
			} else {
				SubSys->TimelapseCameras[CameraName]->StartTimelapse();
				Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' started.");
				return EExecutionStatus::COMPLETED;
			}
		} else if (Arguments[1] == "stop") {
			if (Arguments.Num() < 3) return EExecutionStatus::BAD_ARGUMENTS;
			FString CameraName = FString::Printf(TEXT("%s-Timelapse"), *Arguments[2]);
			if (!SubSys->TimelapseCameras.Contains(CameraName)) {
				Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' doesn't exists.");
				return EExecutionStatus::UNCOMPLETED;
			} else {
				SubSys->TimelapseCameras[CameraName]->StopTimelapse();
				Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' stopped.");
				return EExecutionStatus::COMPLETED;
			}
		}
	}
	return EExecutionStatus::BAD_ARGUMENTS;
}
