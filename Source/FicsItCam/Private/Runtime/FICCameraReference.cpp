#include "Runtime/FICCameraReference.h"

#include "FICSubsystem.h"
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
	static const FRegexPattern Pattern = FRegexPattern("^(>|#)?((-?\\d+)~)?(\\w+)(:(\\w+))?$");

	if (OutName) *OutName = TEXT("");

	FRegexMatcher Matcher(Pattern, ReferenceString);
	if (Matcher.FindNext()) {
		FFICCameraReference Ref;
		
		FString Type = Matcher.GetCaptureGroup(1);
		FString Num = Matcher.GetCaptureGroup(3);
		FString Scene = Matcher.GetCaptureGroup(4);
		FString Camera = Matcher.GetCaptureGroup(6);
		
		if (Type == ">") Ref.bUsePlay = true;
		else if (Type != "#") {
			if (Type.Len() < 1 && Num.Len () < 1 && Camera.Len() < 1) *OutName = Scene;
			return FFICCameraReference();
		}
		
		if (Num.Len() > 0) Ref.Frame = FCString::Atoi64(*Num);

		Ref.Scene = Scene;
		Ref.Camera = Camera;

		if (WorldContext) {
			if (!Ref.IsValid(WorldContext)) return FFICCameraReference();
		}
		
		return Ref;
	}

	return FFICCameraReference();
}

FString FFICCameraReference::ToString() const {
	FString Str = bUsePlay ? TEXT(">") : TEXT("#");
	if (Frame != 0) Str += FString::Printf(TEXT("%lld~"), Frame);
	Str += Scene;
	if (Camera.Len() > 0) Str += FString::Printf(TEXT(":%s"), *Camera);
	return Str;
}

bool FFICCameraReference::IsValid(UObject* WorldContext) const {
	if (!WorldContext) {
		return Scene.Len() > 0;
	}
	
	AFICScene* ScenePtr = GetScene(WorldContext);
	if (!ScenePtr) return false;

	for (UObject* SceneObject : ScenePtr->GetSceneObjects()) {
		UFICCamera* CameraPtr = Cast<UFICCamera>(SceneObject);
		if (CameraPtr && (Camera.Len() < 1 || CameraPtr->GetSceneObjectName() == Camera)) return true;
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
	if (Camera.Len() < 1) return GetScene(WorldContext)->GetActiveCamera(Frame);
	else for (UObject* SceneObject : GetScene(WorldContext)->GetSceneObjects()) {
		UFICCamera* CameraPtr = Cast<UFICCamera>(SceneObject);
		if (CameraPtr && CameraPtr->GetSceneObjectName() == Camera) {
			return CameraPtr;
		}
	}
	return nullptr;
}

#pragma optimize("", off)
FFICCameraSettingsSnapshot FFICCameraReference::GetSnapshot(UObject* WorldContext) const {
	FICFrameFloat Time;
	UFICCamera* CameraPtr = GetCamera(WorldContext, nullptr, &Time);
	if (!CameraPtr) return FFICCameraSettingsSnapshot();
	FFICCameraSettingsSnapshot Snapshot;
	Snapshot.Camera = CameraPtr;
	Snapshot.Location = CameraPtr->Position.Get(Time);
	Snapshot.Rotation = CameraPtr->Rotation.Get(Time);
	Snapshot.FOV = CameraPtr->FOV.GetValue(Time);
	Snapshot.Aperture = CameraPtr->Aperture.GetValue(Time);
	Snapshot.FocusDistance = CameraPtr->FocusDistance.GetValue(Time);
	return Snapshot;
}
#pragma optimize("", on)
