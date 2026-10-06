#include "Runtime/Process/FICRuntimeProcessPlayScene.h"

#include "FICSubsystem.h"
#include "Command/CommandSender.h"

void UFICRuntimeProcessPlayScene::Initialize() {
	Super::Initialize();
	Progress = (float)Scene->AnimationRange.Begin / (float)Scene->FPS;
}

void UFICRuntimeProcessPlayScene::Start(AFICRuntimeProcessorCharacter* InCharacter) {
	if (!bBackground && InCharacter) {
		InCharacter->SetCamera(true);
		if (Scene->ResolutionWidth > 0 && Scene->ResolutionHeight > 0) {
			InCharacter->Camera->SetAspectRatio((float)Scene->ResolutionWidth / (float)Scene->ResolutionHeight);
		}
	}

	for (UObject* Object : Scene->GetSceneObjects()) {
		if (auto SceneObject = Cast<IFICSceneObject>(Object)) {
			SceneObject->InitAnimation(Scene);
		}
	}

	ActiveSceneObjectManager.Initialize(Scene);
	ActiveSceneObjectManager.IsSceneObjectActive.BindLambda([this](UObject* SceneObject, FICFrameFloat Frame) {
		IFICSceneObjectActive* Active = Cast<IFICSceneObjectActive>(SceneObject);
		return Active->GetActiveAttribute().GetValue(Frame);
	});
}

void UFICRuntimeProcessPlayScene::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaTime) {
	FICFrameFloat Time = Progress * Scene->FPS;
	
	ActiveSceneObjectManager.UpdateActiveObjects(Time);
	
	FMinimalViewInfo ViewInfo = Scene->CalculateView(Time);

	if (!bBackground && InCharacter) {
		InCharacter->SetActorLocation(ViewInfo.Location);
		InCharacter->SetActorRotation(ViewInfo.Rotation);
		if (InCharacter->GetController()) {
			InCharacter->GetController()->SetControlRotation(ViewInfo.Rotation);
			if (APlayerController* PC = Cast<APlayerController>(InCharacter->GetController())) {
				if (PC->PlayerCameraManager) PC->PlayerCameraManager->UnlockFOV();
			}
		}
		if (InCharacter->Camera) {
			InCharacter->Camera->SetFieldOfView(ViewInfo.FOV);
			InCharacter->Camera->PostProcessSettings = ViewInfo.PostProcessSettings;
			InCharacter->Camera->PostProcessBlendWeight = ViewInfo.PostProcessBlendWeight;
		}
	}

	for (UObject* Object : Scene->GetSceneObjects()) {
		if (auto SceneObject = Cast<IFICSceneObject>(Object)) {
			SceneObject->TickAnimation(Time);
		}
	}
	
	if (Time > Scene->AnimationRange.End) {
		Progress = (Scene->AnimationRange.Begin + (Time - Scene->AnimationRange.End)) / (FICFrameFloat)Scene->FPS;
		if (!Scene->bLooping) {
			if (bBackground) {
				AFICSubsystem::GetFICSubsystem(this)->StopRuntimeProcess(this);
			}
			else AFICSubsystem::GetFICSubsystem(this)->RemoveRuntimeProcess(this);
		}
	} else {
		Progress += DeltaTime;
	}
}

void UFICRuntimeProcessPlayScene::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	ActiveSceneObjectManager.Shutdown();
	for (UObject* Object : Scene->GetSceneObjects()) {
		if (auto SceneObject = Cast<IFICSceneObject>(Object)) {
			SceneObject->ShutdownAnimation();
		}
	}
}

void UFICRuntimeProcessPlayScene::Shutdown() {}

UFICRuntimeProcessPlayScene* UFICRuntimeProcessPlayScene::StartPlayScene(AFICScene* InScene, bool bInBackground) {
	if (InScene->IsSceneAlreadyInUse()) return nullptr;
	AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(InScene);
	UFICRuntimeProcessPlayScene* Process = NewObject<UFICRuntimeProcessPlayScene>(SubSys);
	Process->Scene = InScene;
	Process->bBackground = bInBackground;
	if (SubSys->CreateRuntimeProcess(AFICScene::GetSceneProcessKey(InScene->SceneName), Process, true)) {
		return Process;
	} else {
		return nullptr;
	}
}
