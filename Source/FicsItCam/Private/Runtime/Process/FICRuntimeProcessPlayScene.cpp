#include "Runtime/Process/FICRuntimeProcessPlayScene.h"

#include "CineCameraComponent.h"
#include "FICSubsystem.h"
#include "Command/CommandSender.h"

void UFICRuntimeProcessPlayScene::Initialize() {
	Super::Initialize();
	Progress = (float)Scene->AnimationRange.Begin / (float)Scene->FPS;
}

void UFICRuntimeProcessPlayScene::Start(AFICRuntimeProcessorCharacter* InCharacter) {
	if (!bBackground) {
		InCharacter->SetCamera(true, Scene->bUseCinematic);
		if (Scene->bUseCinematic) {
			UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(InCharacter->Camera);
			CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::Manual;
			CineCamera->Filmback.SensorWidth = Scene->SensorDimension.X;
			CineCamera->Filmback.SensorHeight = Scene->SensorDimension.Y;
		} else {
			InCharacter->Camera->SetAspectRatio(Scene->ResolutionHeight / Scene->ResolutionWidth);
		}
	}

	for (UObject* SceneObject : Scene->GetSceneObjects()) {
		Cast<IFICSceneObject>(SceneObject)->InitAnimation();
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
	
	UFICCamera* Camera = Scene->GetActiveCamera(Time);
	FVector Pos = Camera->Position.Get(Time);
	FRotator Rot = Camera->Rotation.Get(Time);
	float FOV = Camera->FOV.GetValue(Time);
	float Aperture = Camera->Aperture.GetValue(Time);
	float FocusDistance = Camera->FocusDistance.GetValue(Time);

	if (!bBackground) {
		InCharacter->SetActorLocation(Pos);
		InCharacter->SetActorRotation(Rot);
		InCharacter->GetController()->SetControlRotation(Rot);
		Cast<APlayerController>(InCharacter->GetController())->PlayerCameraManager->UnlockFOV();
		UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(InCharacter->Camera);
		InCharacter->Camera->SetFieldOfView(FOV);
		if (CineCamera) {
			CineCamera->CurrentAperture = Aperture;
			CineCamera->FocusSettings.ManualFocusDistance = FocusDistance;
		}
	}

	for (UObject* SceneObject : Scene->GetSceneObjects()) {
		Cast<IFICSceneObject>(SceneObject)->TickAnimation(Time);
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
	for (UObject* SceneObject : Scene->GetSceneObjects()) {
		Cast<IFICSceneObject>(SceneObject)->ShutdownAnimation();
	}
}

void UFICRuntimeProcessPlayScene::Shutdown() {}
