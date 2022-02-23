#include "InitGameWorld.h"

#include "FGGameInstance.h"
#include "FGGameState.h"
#include "FGPlayerController.h"
#include "FICChatCommand.h"
#include "FICSubsystem.h"

UInitGameWorldFicsItCam::UInitGameWorldFicsItCam() {
	bRootModule = true;
	mChatCommands.Add(AFICChatCommand::StaticClass());
	ModSubsystems.Add(AFICSubsystem::StaticClass());
	ModSubsystems.Add(AFICEditorSubsystem::StaticClass());
}

#pragma optimize("", off)
void UInitGameWorldFicsItCam::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	if (Phase == ELifecyclePhase::POST_INITIALIZATION) {
		UGameInstance* GameInstance = GetWorld()->GetGameInstance();
		UFGGameInstance* FGGameInstance = Cast<UFGGameInstance>(GameInstance);
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		AFGPlayerController* FGPlayerController = Cast<AFGPlayerController>(PlayerController);
		AGameStateBase* GameState = GetWorld()->GetGameState();
		AFGGameState* FGGameState = Cast<AFGGameState>(GameState);
		UE_LOG(LogTemp, Warning, TEXT("%p %p %p %p %p %p"), GameInstance, FGGameInstance, PlayerController, FGPlayerController, GameState, FGGameState)
	}
}
#pragma optimize("", on)
