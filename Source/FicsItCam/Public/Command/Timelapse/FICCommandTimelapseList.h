#pragma once

#include "FICCommandTimelapse.h"
#include "FICCommandTimelapseList.generated.h"

UCLASS()
class UFICCommandTimelapseList : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandTimelapseList() {
		bFinal = true;
		ParentCommand = UFICCommandTimelapse::StaticClass();
		CommandName = TEXT("list");
		CommandSyntax = TEXT("/fic timelapse list");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		InSender->SendChatMessage(TEXT("Timelapse Cameras:"), FColor::Green);
		TMap<FString, UFICRuntimeProcess*> ActiveProcesses = SubSys->GetActiveRuntimeProcessesMap();
		for (TTuple<FString, UFICRuntimeProcess*> Process : SubSys->GetRuntimeProcesses()) {
			UFICRuntimeProcessTimelapseCamera* Camera = Cast<UFICRuntimeProcessTimelapseCamera>(Process.Value);
			if (!Camera) continue;
			FString Name = Process.Key;
			Name.RemoveFromStart(TEXT("Timelapse_"));
			FString State = ActiveProcesses.Contains(Process.Key) ? TEXT("Running") : TEXT("Stopped");
			FString Text = FString::Printf(TEXT("'%s' %fspf %s"), *Name, Camera->SecondsPerFrame, *State);
			InSender->SendChatMessage(Text, FColor::Green);
		}
		return EExecutionStatus::COMPLETED;
	}
};