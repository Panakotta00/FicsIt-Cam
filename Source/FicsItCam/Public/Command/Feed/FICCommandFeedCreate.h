#pragma once

#include "FICCommandFeed.h"
#include "FICUtils.h"
#include "Runtime/Process/FICRuntimeProcessCameraFeed.h"
#include "FICCommandFeedCreate.generated.h"

UCLASS()
class UFICCommandFeedCreate : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandFeedCreate() {
		bFinal = true;
		ParentCommand = UFICCommandFeed::StaticClass();
		CommandName = TEXT("create");
		CommandSyntax = TEXT("/fic feed create <camera>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		CheckCameraRefFromArg(CameraName, CameraRef, 0)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		FeedKey(Key, 0)
		if (SubSys->GetRuntimeProcesses().Contains(Key)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Feed for '%s' already exists!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		UFICRuntimeProcessCameraFeed* Process = NewObject<UFICRuntimeProcessCameraFeed>(SubSys);
		TArray<FString> CamArgs = InArgs;
		CamArgs.RemoveAt(0);
		Process->CameraArgument = FFICCameraArgument::FromCli(InSender, CameraRef, CameraName, CamArgs);
		if (!SubSys->CreateRuntimeProcess(Key, Process)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to create Feed for '%s'!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Feed for '%s' created."), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};