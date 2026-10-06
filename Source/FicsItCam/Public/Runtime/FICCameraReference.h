#pragma once
#include "Camera/CameraTypes.h"
#include "Data/FICTypes.h"

#include "FICCameraReference.generated.h"

class UFICRuntimeProcessPlayScene;
class UFICCamera;
class AFICScene;

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

	FMinimalViewInfo GetViewInfo(UObject* WorldContext) const;
};