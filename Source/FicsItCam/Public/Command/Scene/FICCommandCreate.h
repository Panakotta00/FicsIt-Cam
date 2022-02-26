#pragma once

#include "FGGameUserSettings.h"
#include "FICUtils.h"
#include "Command/FICCommand.h"
#include "FICCommandCreate.generated.h"

UCLASS()
class UFICCommandCreate : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandCreate() {
		bFinal = true;
		CommandName = TEXT("create");
		CommandSyntax = TEXT("/fic create <scene name>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		if (!UFICUtils::IsValidFICObjectName(InArgs[0])) {
			InSender->SendChatMessage(FString::Printf(TEXT("'%s' is no valid scene name!"), *InArgs[0]));
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		if (AFICSubsystem::GetFICSubsystem(InSender)->FindSceneByName(InArgs[0])) {
			InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' already exists!"), *InArgs[0]));
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		AFICScene* Scene = InSender->GetWorld()->SpawnActor<AFICScene>();
		
		FIntPoint Resolution = UFGGameUserSettings::GetFGGameUserSettings()->GetScreenResolution();
		Scene->ResolutionWidth = Resolution.X;
		Scene->ResolutionHeight = Resolution.Y;
		Scene->SceneName = InArgs[0];
		UFICCamera* CDO = UFICCamera::StaticClass()->GetDefaultObject<UFICCamera>();
		if (CDO) Scene->AddSceneObject(CDO->CreateNewObject(AFICEditorSubsystem::GetFICEditorSubsystem(this), Scene));
		
		InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' created."), *InArgs[0]));
		return EExecutionStatus::COMPLETED;
	}
};