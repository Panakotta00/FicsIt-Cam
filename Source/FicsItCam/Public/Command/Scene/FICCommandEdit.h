#pragma once

#include "Command/FICCommand.h"
#include "FICCommandEdit.generated.h"

UCLASS()
class UFICCommandEdit : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandEdit() {
		bFinal = true;
		CommandName = TEXT("edit");
		CommandSyntax = TEXT("/fic edit <scene>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		TryGetSceneFromArg(Scene, 0)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		AFICEditorSubsystem* EditSubSys = AFICEditorSubsystem::GetFICEditorSubsystem(InSender);
		GetSceneKey(Key, Scene->SceneName)
		CheckSceneUsage(SubSys, EditSubSys, Key, Scene->SceneName)
		EditSubSys->OpenEditor(Scene);
		return EExecutionStatus::COMPLETED;
	}
};