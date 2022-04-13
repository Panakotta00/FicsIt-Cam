#pragma once

#include "FICCommandTimelapse.h"
#include "FICCommandTimelapseTP.generated.h"

UCLASS()
class UFICCommandTimelapseTP : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandTimelapseTP() {
		bFinal = true;
		ParentCommand = UFICCommandTimelapse::StaticClass();
		CommandName = TEXT("tp");
		CommandSyntax = TEXT("/fic timelapse tp <camera>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(1)
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InSender);
		TimelapseKey(Key, 0)
		CheckTimelapseFromKey(Timelapse, Key)
		UFICRuntimeProcessTimelapseCamera* Process = Cast<UFICRuntimeProcessTimelapseCamera>(SubSys->FindRuntimeProcess(Key));
		AFGPlayerController* Player = InSender->GetPlayer();
		if (Player) {
			FFICCameraSettingsSnapshot Camera = Process->CameraArgument.GetCameraSettingsSnapshot(this);
			Player->GetCharacter()->SetActorLocation(Camera.Location);
			Player->SetControlRotation(Camera.Rotation);
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Teleported to '%s'!"), *InArgs[0]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};