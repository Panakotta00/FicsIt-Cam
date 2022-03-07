#pragma once

#include "command/ChatCommandInstance.h"
#include "Command/CommandSender.h"
#include "FICSubsystem.h"
#include "Editor/FICEditorSubsystem.h"
#include "Editor/FICEditorContext.h"
#include "Runtime/FICCameraReference.h"
#include "FICCommand.generated.h"

#define CheckArgCount(Count) \
	if (InArgs.Num() < Count) { \
    	InSender->SendChatMessage(FString::Printf(TEXT("Syntax: %s"), *CommandSyntax), FColor::Red); \
    	return EExecutionStatus::BAD_ARGUMENTS; \
    }

#define TryGetSceneFromArg(SceneVarName, ArgNum) \
	AFICScene* SceneVarName = AFICSubsystem::GetFICSubsystem(InSender)->FindSceneByName(InArgs[ArgNum]); \
	if (!SceneVarName) { \
		InSender->SendChatMessage(FString::Printf(TEXT("Unable to find scene '%s'!"), *InArgs[ArgNum]), FColor::Red); \
		return EExecutionStatus::BAD_ARGUMENTS; \
	}

#define TryGetBoolFromArgOpt(BoolVarName, DefaultValue, ArgNum) \
	bool BoolVarName = DefaultValue; \
	if (InArgs.Num() > ArgNum) { \
		BoolVarName = InArgs[ArgNum] == TEXT("true"); \
	}

#define GetSceneKey(SceneKeyVar, SceneName) \
	FString SceneKeyVar = FString::Printf(TEXT("Scene_%s"), SceneName);

#define CheckSceneUsage(SubSys, EditSubSys, ProcessKey, SceneNameToCheck) \
	if (SubSys->GetActiveRuntimeProcessesMap().Contains(ProcessKey) || (EditSubSys->GetActiveEditorContext() && EditSubSys->GetActiveEditorContext()->GetScene()->SceneName == SceneNameToCheck)) { \
    	InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' is already in use."), *SceneNameToCheck), FColor::Red); \
    	return EExecutionStatus::UNCOMPLETED; \
    };

#define CheckCameraRefFromArg(CameraNameVar, CameraRefVar, ArgNum) \
	FString CameraNameVar; \
	FFICCameraReference CameraRef = FFICCameraReference::FromString(InSender, InArgs[ArgNum], &CameraNameVar); \
	if ((CameraRef.IsValid(nullptr) && !CameraRef.IsValid(InSender)) || (!CameraRef.IsValid(nullptr) && CameraNameVar.Len() < 1)) { \
		InSender->SendChatMessage(FString::Printf(TEXT("Invalid camera reference '%s'!"), *InArgs[ArgNum]), FColor::Red); \
		return EExecutionStatus::BAD_ARGUMENTS; \
	}

UCLASS()
class UFICCommand : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY()
	TSubclassOf<UFICCommand> ParentCommand = nullptr;

	UPROPERTY()
	FString CommandName;

	UPROPERTY()
	bool bFinal = false;

	UPROPERTY()
	FString CommandDescription;

	UPROPERTY()
	FString CommandSyntax;

	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) { return EExecutionStatus::BAD_ARGUMENTS; }
};