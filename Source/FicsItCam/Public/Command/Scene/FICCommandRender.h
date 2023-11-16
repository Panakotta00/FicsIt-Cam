#pragma once

#include "Command/FICCommand.h"
#include "Runtime/Process/FICRuntimeProcessRenderScene.h"
#include "FICCommandRender.generated.h"

UCLASS()
class UFICCommandRender : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandRender() {
		bFinal = true;
		CommandName = TEXT("render");
		CommandSyntax = TEXT("/fic render <scene>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		TryGetSceneFromArg(Scene, 0)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		AFICEditorSubsystem* EditSubSys = AFICEditorSubsystem::GetFICEditorSubsystem(InSender);
		GetSceneKey(Key, Scene->SceneName)
		CheckSceneUsage(SubSys, EditSubSys, Key, Scene->SceneName)
		UFICRuntimeProcessRenderScene* Process = NewObject<UFICRuntimeProcessRenderScene>(SubSys);
		Process->Scene = Scene;
		SubSys->CreateRuntimeProcess(Key, Process, true);
		return EExecutionStatus::COMPLETED;
	}
};