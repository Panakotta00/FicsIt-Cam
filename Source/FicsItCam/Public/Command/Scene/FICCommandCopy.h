#pragma once

#include "Command/FICCommand.h"
#include "FICCommandCopy.generated.h"

UCLASS()
class UFICCommandCopy : public UFICCommand {
	GENERATED_BODY()
public:
	UFICCommandCopy() {
		bFinal = true;
		CommandName = TEXT("copy");
		CommandSyntax = TEXT("/fic copy <old scene> <new scene>");
	}
	
	virtual EExecutionStatus ExecuteCommand(UCommandSender* InSender, TArray<FString> InArgs) override {
		CheckArgCount(2)
		TryGetSceneFromArg(OldScene, 0)
		if (!UFICUtils::IsValidFICObjectName(InArgs[1])) {
			InSender->SendChatMessage(FString::Printf(TEXT("'%s' is no valid scene name!"), *InArgs[1]));
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		if (AFICSubsystem::GetFICSubsystem(InSender)->FindSceneByName(InArgs[1])) {
			InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' already exists!"), *InArgs[1]));
			return EExecutionStatus::BAD_ARGUMENTS;
		}
		AFICScene* NewScene = InSender->GetWorld()->SpawnActor<AFICScene>();
		NewScene->SceneName = InArgs[1];
		NewScene->AnimationRange = OldScene->AnimationRange;
		NewScene->FPS = OldScene->FPS;
		NewScene->ResolutionHeight = OldScene->ResolutionHeight;
		NewScene->ResolutionWidth = OldScene->ResolutionWidth;
		NewScene->SensorDimension = OldScene->SensorDimension;
		NewScene->bBulletTime = OldScene->bBulletTime;
		NewScene->bUseCinematic = OldScene->bUseCinematic;
		NewScene->bLooping = OldScene->bLooping;
		NewScene->LastCameraTransform = OldScene->LastCameraTransform;
		NewScene->bViewportEverSaved = OldScene->bViewportEverSaved;
		for (UObject* OldSceneObject : OldScene->GetSceneObjects()) {
			NewScene->AddSceneObject(StaticDuplicateObject(OldSceneObject, NewScene));
		}
		InSender->SendChatMessage(FString::Printf(TEXT("Scene '%s' copied to new scene '%s'."), *InArgs[0], *InArgs[1]), FColor::Green);
		return EExecutionStatus::COMPLETED;
	}
};