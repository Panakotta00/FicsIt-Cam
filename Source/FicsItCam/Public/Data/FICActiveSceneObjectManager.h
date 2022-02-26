#pragma once

#include "FICScene.h"

DECLARE_DELEGATE_RetVal_TwoParams(bool, FFICISceneObjectActive, UObject*, FICFrameFloat)

class FFICActiveSceneObjectManager {
private:
	TMap<FString, UObject*> ActiveSceneObjects;
	AFICScene* Scene = nullptr;

public:
	FFICISceneObjectActive IsSceneObjectActive;
	
	void Initialize(AFICScene* InScene);
	void UpdateActiveObjects(FICFrameFloat Frame);
	void Shutdown();
};
