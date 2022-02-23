#pragma once

#include "Process/FICRuntimeProcessPlayScene.h"
#include "FICCameraReference.generated.h"

class UFICCamera;
class AFICScene;

USTRUCT()
struct FFICCameraSettingsSnapshot {
	GENERATED_BODY()

	UPROPERTY()
	UFICCamera* Camera = nullptr;
	
	UPROPERTY(SaveGame)
	FVector Location;

	UPROPERTY(SaveGame)
	FRotator Rotation;

	UPROPERTY(SaveGame)
	float FOV;

	UPROPERTY(SaveGame)
	float Aperture;

	UPROPERTY(SaveGame)
	float FocusDistance;

	bool IsValid() { return !!Camera; }
};

USTRUCT()
struct FFICCameraReference {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	bool bUsePlay = false;
	
	UPROPERTY(SaveGame)
	int64 Frame = 0;
	
	UPROPERTY(SaveGame)
	FString Scene;

	UPROPERTY(SaveGame)
	FString Camera;

	UFICRuntimeProcessPlayScene* GetCurrentScenePlay(UObject* WorldContext) const;

public:
	FFICCameraReference() = default;
	FFICCameraReference(bool bUsePlay, int64 Frame, FString Scene, FString Camera) : bUsePlay(bUsePlay), Frame(Frame), Scene(Scene), Camera(Camera) {}

	static FFICCameraReference FromString(UObject* WorldContext, FString ReferenceString, FString* OutName);

	FString ToString() const;
	
	bool IsValid(UObject* WorldContext) const;
	AFICScene* GetScene(UObject* WorldContext) const;
	FICFrameFloat GetTime(UObject* WorldContext, UFICRuntimeProcessPlayScene** OptOutRuntimePlay = nullptr) const;
	UFICCamera* GetCamera(UObject* WorldContext, UFICRuntimeProcessPlayScene** OptOutRuntimePlay = nullptr, FICFrameFloat* OptOutTime = nullptr) const;

	FFICCameraSettingsSnapshot GetSnapshot(UObject* WorldContext) const;
};