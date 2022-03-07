#pragma once

#include "Command/FICCommand.h"
#include "Runtime/Process/FICRuntimeProcessPlayScene.h"
#include "FICCommandPlay.generated.h"

UCLASS()
class UFICCommandPlay : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandPlay() {
		bFinal = true;
		CommandName = TEXT("play");
		CommandSyntax = TEXT("/fic play <scene> [<'true' if in background>]");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		TryGetSceneFromArg(Scene, 0)
		TryGetBoolFromArgOpt(bBackground, false, 1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		AFICEditorSubsystem* EditSubSys = AFICEditorSubsystem::GetFICEditorSubsystem(InSender);
		GetSceneKey(Key, *Scene->SceneName)
		CheckSceneUsage(SubSys, EditSubSys, Key, Scene->SceneName)
		UFICRuntimeProcessPlayScene* Process = NewObject<UFICRuntimeProcessPlayScene>(SubSys);
		Process->Scene = Scene;
		Process->bBackground = bBackground;
		if (bBackground) {
			if (SubSys->CreateRuntimeProcess(Key, Process, false)) {
				InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' is now ready to play in background!"), *InArgs[0]), FColor::Green);
			} else {
				InSender->SendChatMessage(FString::Printf(TEXT("Failed make Scene '%s' ready for playing in background!"), *InArgs[0]), FColor::Red);
			}
		} else SubSys->CreateRuntimeProcess(Key, Process, true);
		return EExecutionStatus::COMPLETED;
	}
};