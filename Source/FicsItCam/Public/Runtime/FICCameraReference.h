#pragma once
#include "Data/FICTypes.h"

#include "FICCameraReference.generated.h"

class UFICRuntimeProcessPlayScene;
class UFICCamera;
class AFICScene;

USTRUCT(BlueprintType)
struct FFICCameraSettingsSnapshot {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	UFICCamera* Camera = nullptr;
	
	UPROPERTY(SaveGame, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FRotator Rotation;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float FOV;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float Aperture;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float FocusDistance;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FPostProcessSettings PostProcessSettings;

	bool IsValid() { return !!Camera; }
};

USTRUCT(BlueprintType)
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

	UPROPERTY(SaveGame)
	FString Data;

	UFICRuntimeProcessPlayScene* GetCurrentScenePlay(UObject* WorldContext) const;

public:
	FFICCameraReference() = default;
	FFICCameraReference(bool bUsePlay, int64 Frame, FString Scene, FString Camera, FString Data) : bUsePlay(bUsePlay), Frame(Frame), Scene(Scene), Camera(Camera), Data(Data) {}

	static FFICCameraReference FromString(UObject* WorldContext, FString ReferenceString, FString* OutName);

	FString ToString() const;
	
	bool IsValid(UObject* WorldContext) const;
	AFICScene* GetScene(UObject* WorldContext) const;
	FICFrameFloat GetTime(UObject* WorldContext, UFICRuntimeProcessPlayScene** OptOutRuntimePlay = nullptr) const;
	UFICCamera* GetCamera(UObject* WorldContext, UFICRuntimeProcessPlayScene** OptOutRuntimePlay = nullptr, FICFrameFloat* OptOutTime = nullptr) const;
	FString GetData() const { return Data; }
	bool IsAnimated() const { return bUsePlay; }

	FFICCameraSettingsSnapshot GetSnapshot(UObject* WorldContext) const;
};