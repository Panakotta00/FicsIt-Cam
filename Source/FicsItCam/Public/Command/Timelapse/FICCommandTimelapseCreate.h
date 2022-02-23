#pragma once

#include "FICCommandTimelapse.h"
#include "FICUtils.h"
#include "Runtime/Process/FICRuntimeProcessTimelapseCamera.h"
#include "FICCommandTimelapseCreate.generated.h"

UCLASS()
class UFICCommandTimelapseCreate : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandTimelapseCreate() {
		bFinal = true;
		ParentCommand = UFICCommandTimelapse::StaticClass();
		CommandName = TEXT("create");
		CommandSyntax = TEXT("/fic timelapse create <camera> <seconds per frame as float>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(2)
		CheckCameraRefFromArg(CameraName, CameraRef, 0)
		float SPF = FCString::Atof(*InArgs[1]);
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		TimelapseKey(Key, 0)
		if (SubSys->GetRuntimeProcesses().Contains(Key)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Timelapse for '%s' already exists!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		UFICRuntimeProcessTimelapseCamera* Process = NewObject<UFICRuntimeProcessTimelapseCamera>(SubSys);
		TArray<FString> CamArgs = InArgs;
		CamArgs.RemoveAt(0);
		Process->CameraArgument = FFICCameraArgument::FromCli(InSender, CameraRef, CameraName, CamArgs);
		Process->SecondsPerFrame = SPF;
		if (!SubSys->CreateRuntimeProcess(Key, Process)) {
			InSender->SendChatMessage(FString::Printf(TEXT("Unable to create Timelapse for '%s'!"), *InArgs[0]), FColor::Red);
			return EExecutionStatus::UNCOMPLETED;
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Timelapse for '%s' created."), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};