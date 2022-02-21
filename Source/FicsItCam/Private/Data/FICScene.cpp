#include "Data/FICScene.h"

void AFICScene::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	IFGSaveInterface::PostLoadGame_Implementation(saveVersion, gameVersion);

	SceneObjects.Remove(nullptr);
}

UFICCamera* AFICScene::GetActiveCamera(FICFrameFloat Time) {
	for (UObject* SceneObject : GetSceneObjects()) {
		UFICCamera* Camera = Cast<UFICCamera>(SceneObject);
		if (Camera) {
			if (Camera->Active.GetValue(Time)) return Camera;
		}
	}
	return nullptr;
}
