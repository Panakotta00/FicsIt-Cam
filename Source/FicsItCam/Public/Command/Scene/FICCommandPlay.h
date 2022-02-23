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
		GetSceneKey(Key, Scene)
		CheckSceneUsage(SubSys, EditSubSys, Key, Scene->SceneName)
		UFICRuntimeProcessPlayScene* Process = NewObject<UFICRuntimeProcessPlayScene>(SubSys);
		Process->Scene = Scene;
		Process->bBackground = bBackground;
		SubSys->CreateRuntimeProcess(Key, Process, !bBackground);
		if (bBackground) InSender->SendChatMessage(FString::Printf(TEXT("Begin to play '%s' in background!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};