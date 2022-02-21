#include "FICCommand.h"

#include "FGGameUserSettings.h"
#include "FICSubsystem.h"
#include "Engine/World.h"
#include "FGPlayerController.h"
#include "Command/CommandSender.h"
#include "Data/FICAnimation.h"
#include "Editor/FICEditorSubsystem.h"
#include "Misc/DefaultValueHelper.h"
#include "Runtime/FICTimelapseCamera.h"
#include "Runtime/Process/FICRuntimeProcess.h"
#include "Runtime/Process/FICRuntimeProcessPlayScene.h"

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
		
	if (Arguments[0] == "list") {
		Sender->SendChatMessage(TEXT("List of Animations:"));
		for (TActorIterator<AFICScene> Scene(GetWorld()); Scene; ++Scene) {
			Sender->SendChatMessage(Scene->SceneName);
		}
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "create") {
		if (Arguments.Num() < 2) {
			Sender->SendChatMessage(TEXT("Syntax: /fic create <name>"), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		if (SubSys->FindSceneByName(Arguments[1])) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' already exists.");
		} else {
			FVector Pos = Sender->GetPlayer()->PlayerCameraManager->GetCameraLocation();
			FRotator Rot = Sender->GetPlayer()->PlayerCameraManager->GetCameraRotation();
			float FOV = Sender->GetPlayer()->PlayerCameraManager->GetFOVAngle();
			AFICScene* Scene = GetWorld()->SpawnActor<AFICScene>();
			FIntPoint Resolution = UFGGameUserSettings::GetFGGameUserSettings()->GetScreenResolution();
			Scene->ResolutionWidth = Resolution.X;
			Scene->ResolutionHeight = Resolution.Y;
			Scene->SceneName = Arguments[1];
			UFICCamera* Camera = NewObject<UFICCamera>(Scene);
			Scene->AddSceneObject(Camera);
			Camera->Position.SetDefaultValue(Pos);
			Camera->Rotation.SetDefaultValue(Rot);
			Camera->FOV.SetDefaultValue(FOV);
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' created.");
		}
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "delete") {
		if (Arguments.Num() < 2) {
			Sender->SendChatMessage(TEXT("Syntax: /fic delete <name>"), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		AFICScene* Scene = SubSys->FindSceneByName(Arguments[1]);
		if (Scene) {
			Scene->Destroy();
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' deleted.");
			return EExecutionStatus::COMPLETED;
		} else {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::BAD_ARGUMENTS;
		}
	}
	if (Arguments[0] == "edit") {
		if (Arguments.Num() < 2) {
			Sender->SendChatMessage(TEXT("Syntax: /fic edit <name>"), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		AFICScene* Scene = SubSys->FindSceneByName(Arguments[1]);
		if (!Scene) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::COMPLETED;
		} else {
			AFICEditorSubsystem::GetFICEditorSubsystem(Scene)->OpenEditor(Scene);
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' opened for edit.");
			return EExecutionStatus::COMPLETED;
		}
	}
	if (Arguments[0] == "play") {
		if (Arguments.Num() < 2) {
			Sender->SendChatMessage(TEXT("Syntax: /fic play <name>"), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		AFICScene* Scene = SubSys->FindSceneByName(Arguments[1]);
		if (!Scene) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		UFICRuntimeProcessPlayScene* Play = NewObject<UFICRuntimeProcessPlayScene>(SubSys);
		Play->Scene = Scene;
		SubSys->StartProcess(Play);
		Sender->SendChatMessage("Playing Animation '" + Arguments[1] + "'.");
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "render") {
		if (Arguments.Num() < 2) {
			Sender->SendChatMessage(TEXT("Syntax: /fic render <name>"), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		AFICScene* Scene = SubSys->FindSceneByName(Arguments[1]);
		if (!Scene) {
			Sender->SendChatMessage("Animation '" + Arguments[1] + "' not found.");
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		/*UFICRuntimeProcessRenderScene* Render = NewObject<UFICRuntimeProcessRenderScene>(SubSys);
		Render->Scene = Scene;
		SubSys->StartProcess(Render);*/
		Sender->SendChatMessage("Rendering Animation '" + Arguments[1] + "'.");
		return EExecutionStatus::COMPLETED;
	}
	if (Arguments[0] == "timelapse") {
		if (Arguments.Num() >= 2) {
			if (Arguments[1] == "list") {
				Sender->SendChatMessage(TEXT("List of Timelapse-Cameras:"));
				for (TPair<FString, AFICTimelapseCamera*> Entry : SubSys->TimelapseCameras) {
					Sender->SendChatMessage(FString::Printf(TEXT("%s-Timelapse: %fs"), *Entry.Key, Entry.Value->Frequency));
				}
				return EExecutionStatus::COMPLETED;
			} else if (Arguments[1] == "create") {
				if (Arguments.Num() < 4) {
					Sender->SendChatMessage(TEXT("Syntax: /fic timelapse create <name> <seconds per frame>"), FColor::Red);
					return EExecutionStatus::BAD_ARGUMENTS;
				}
				FString CameraName = FString::Printf(TEXT("%s-Timelapse"), *Arguments[2]);
				if (SubSys->TimelapseCameras.Contains(CameraName)) {
					Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' already exists.");
					return EExecutionStatus::UNCOMPLETED;
				} else {
					float Frequency;
					if (!FDefaultValueHelper::ParseFloat(Arguments[3], Frequency)) return EExecutionStatus::BAD_ARGUMENTS;
					FVector Pos = Sender->GetPlayer()->PlayerCameraManager->GetCameraLocation();
					FRotator Rot = Sender->GetPlayer()->PlayerCameraManager->GetCameraRotation();
					AFICTimelapseCamera* Camera = GetWorld()->SpawnActor<AFICTimelapseCamera>(Pos, Rot);
					Camera->Name = Arguments[2];
					Camera->Frequency = Frequency;
					SubSys->TimelapseCameras.Add(CameraName, Camera);
					Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' created.");
				}
				return EExecutionStatus::COMPLETED;
			} else if (Arguments[1] == "delete") {
				if (Arguments.Num() < 3) {
					Sender->SendChatMessage(TEXT("Syntax: /fic timelapse delete <name>"), FColor::Red);
					return EExecutionStatus::BAD_ARGUMENTS;
				}
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
				if (Arguments.Num() < 3) {
					Sender->SendChatMessage(TEXT("Syntax: /fic timelapse start <name>"), FColor::Red);
					return EExecutionStatus::BAD_ARGUMENTS;
				}
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
				if (Arguments.Num() < 3) {
					Sender->SendChatMessage(TEXT("Syntax: /fic timelapse stop <name>"), FColor::Red);
					return EExecutionStatus::BAD_ARGUMENTS;
				}
				FString CameraName = FString::Printf(TEXT("%s-Timelapse"), *Arguments[2]);
				if (!SubSys->TimelapseCameras.Contains(CameraName)) {
					Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' doesn't exists.");
					return EExecutionStatus::UNCOMPLETED;
				} else {
					SubSys->TimelapseCameras[CameraName]->StopTimelapse();
					Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' stopped.");
					return EExecutionStatus::COMPLETED;
				}
			} else if (Arguments[1] == "tp") {
				if (Arguments.Num() < 3) {
					Sender->SendChatMessage(TEXT("Syntax: /fic timelapse stop <name>"), FColor::Red);
					return EExecutionStatus::BAD_ARGUMENTS;
				}
				FString CameraName = FString::Printf(TEXT("%s-Timelapse"), *Arguments[2]);
				if (!SubSys->TimelapseCameras.Contains(CameraName)) {
					Sender->SendChatMessage("Timelapse-Camera '" + CameraName + "' doesn't exists.");
					return EExecutionStatus::UNCOMPLETED;
				} else {
					AFICTimelapseCamera* timelapseCamera = SubSys->TimelapseCameras[CameraName];
					Sender->GetPlayer()->GetCharacter()->SetActorLocation(timelapseCamera->CaptureComponent->GetComponentLocation());
					Sender->GetPlayer()->SetControlRotation(timelapseCamera->CaptureComponent->GetComponentRotation());
					Sender->SendChatMessage("Teleported to Timelapse-Camera '" + CameraName + "'.");
					return EExecutionStatus::COMPLETED;
				}
			}
		}
		Sender->SendChatMessage(TEXT("No valid Sub-Subcommand!"), FColor::Red);
		Sender->SendChatMessage(TEXT("/fic timelapse list"), FColor::Red);
		Sender->SendChatMessage(TEXT("/fic timelapse create <name> <seconds per frame>"), FColor::Red);
		Sender->SendChatMessage(TEXT("/fic timelapse delete <name>"), FColor::Red);
		Sender->SendChatMessage(TEXT("/fic timelapse start <name>"), FColor::Red);
		Sender->SendChatMessage(TEXT("/fic timelapse stop <name>"), FColor::Red);
		return EExecutionStatus::BAD_ARGUMENTS;
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
