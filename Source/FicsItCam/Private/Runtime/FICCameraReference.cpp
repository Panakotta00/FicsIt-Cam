#include "Runtime/FICCameraReference.h"

#include "FICSubsystem.h"
#include "Internationalization/Regex.h"
#include "Data/FICScene.h"
#include "Runtime/Process/FICRuntimeProcessPlayScene.h"

UFICRuntimeProcessPlayScene* FFICCameraReference::GetCurrentScenePlay(UObject* WorldContext) const {
	if (!bUsePlay) return nullptr;
	for (TPair<FString, UFICRuntimeProcess*> RuntimeProcess : AFICSubsystem::GetFICSubsystem(WorldContext)->GetRuntimeProcesses()) {
		UFICRuntimeProcessPlayScene* PlayScene = Cast<UFICRuntimeProcessPlayScene>(RuntimeProcess.Value);
		if (PlayScene && PlayScene->Scene->SceneName == Scene) {
			return PlayScene;
		}
	}
	return nullptr;
}

FFICCameraReference FFICCameraReference::FromString(UObject* WorldContext, FString ReferenceString, FString* OutName) {
	static const FRegexPattern Pattern = FRegexPattern("^(>|#)?((-?\\d+)~)?(\\w+)(:(\\w+))?(\\[(.*)\\])?$");

	if (OutName) *OutName = ReferenceString;

	FRegexMatcher Matcher(Pattern, ReferenceString);
	if (Matcher.FindNext()) {
		FFICCameraReference Ref;
		
		FString Type = Matcher.GetCaptureGroup(1);
		FString Num = Matcher.GetCaptureGroup(3);
		FString Scene = Matcher.GetCaptureGroup(4);
		FString Camera = Matcher.GetCaptureGroup(6);
		FString Data = Matcher.GetCaptureGroup(8);
		Ref.Data = Data;
		
		*OutName = Scene;

		if (Type == ">") Ref.bUsePlay = true;
		else if (Type != "#") {
			return Ref;
		}

		if (WorldContext) {
			if (!Ref.IsValid(WorldContext)) return Ref;
		}

		if (Num.Len() > 0) Ref.Frame = FCString::Atoi64(*Num);

		Ref.Scene = Scene;
		Ref.Camera = Camera;

		return Ref;
	}

	return FFICCameraReference();
}

FString FFICCameraReference::ToString() const {
	FString Str = bUsePlay ? TEXT(">") : TEXT("#");
	if (Frame != 0) Str += FString::Printf(TEXT("%lld~"), Frame);
	Str += Scene;
	if (Camera.Len() > 0) Str += FString::Printf(TEXT(":%s"), *Camera);
	if (Data.Len() > 0) Str += FString::Printf(TEXT("[%s]"), *Data);
	return Str;
}

bool FFICCameraReference::IsValid(UObject* WorldContext) const {
	if (!WorldContext) {
		return Scene.Len() > 0;
	}
	
	AFICScene* ScenePtr = GetScene(WorldContext);
	if (!ScenePtr) return false;

	if (Camera.Len() < 1) {
		return true;
	}

	for (UObject* Object : ScenePtr->GetSceneObjects()) {
		if (auto SceneObject = Cast<IFICSceneObject>(Object)) {
			if (SceneObject->GetSceneObjectName() == Camera) return true;
		}
	}
	return false;
}

AFICScene* FFICCameraReference::GetScene(UObject* WorldContext) const {
	if (Scene.Len() < 1) return nullptr;
	return AFICSubsystem::GetFICSubsystem(WorldContext)->FindSceneByName(Scene);
}

FICFrameFloat FFICCameraReference::GetTime(UObject* WorldContext,UFICRuntimeProcessPlayScene** OptOutRuntimePlay) const {
	UFICRuntimeProcessPlayScene* PlayScene = GetCurrentScenePlay(WorldContext);
	if (OptOutRuntimePlay) *OptOutRuntimePlay = PlayScene;
	if (PlayScene) {
		return PlayScene->GetProgress() * PlayScene->Scene->FPS;
	}
	return Frame;
}

UFICCamera* FFICCameraReference::GetCamera(UObject* WorldContext, UFICRuntimeProcessPlayScene** OptOutRuntimePlay, FICFrameFloat* OptOutTime) const {
	UFICRuntimeProcessPlayScene* PlayScene;
	FICFrameFloat Time = GetTime(WorldContext, &PlayScene);
	if (OptOutRuntimePlay) *OptOutRuntimePlay = PlayScene;
	if (OptOutTime) *OptOutTime = Time;
	if (PlayScene) return PlayScene->Scene->GetActiveCamera(Time);
	AFICScene* UsedScene = GetScene(WorldContext);
	if (UsedScene) {
		if (Camera.Len() < 1) return UsedScene->GetActiveCamera(Frame);
		else for (UObject* SceneObject : UsedScene->GetSceneObjects()) {
			UFICCamera* CameraPtr = Cast<UFICCamera>(SceneObject);
			if (CameraPtr && CameraPtr->GetSceneObjectName() == Camera) {
				return CameraPtr;
			}
		}
	}
	return nullptr;
}

FMinimalViewInfo FFICCameraReference::GetViewInfo(UObject* WorldContext) const {
	UFICRuntimeProcessPlayScene* PlayScene;
	FICFrameFloat Time = GetTime(WorldContext, &PlayScene);
	AFICScene* UsedScene = PlayScene ? PlayScene->Scene : GetScene(WorldContext);
	if (UsedScene) {
		if (Camera.Len() < 1) {
			return UsedScene->CalculateView(Time);
		} else {
			for (UObject* Object : UsedScene->GetSceneObjects()) {
				if (auto SceneObject = Cast<IFICSceneObject>(Object)) {
					IFICSceneObject* Obj = Cast<IFICSceneObject>(SceneObject);
					if (Obj->GetSceneObjectName() == Camera) {
						FMinimalViewInfo ViewInfo;
						if (UsedScene->ResolutionHeight > 0) {
							ViewInfo.AspectRatio = (float)UsedScene->ResolutionWidth / (float)UsedScene->ResolutionHeight;
						}
						ViewInfo.ProjectionMode = ECameraProjectionMode::Perspective;
						Obj->ModifyView(ViewInfo, Time);
						return ViewInfo;
					}
				}
			}
			return UsedScene->CalculateView(Time);
		}
	}
	return FMinimalViewInfo();
}
