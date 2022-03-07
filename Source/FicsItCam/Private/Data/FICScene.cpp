#include "Data/FICScene.h"

#include "FICUtils.h"

void AFICScene::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	IFGSaveInterface::PostLoadGame_Implementation(saveVersion, gameVersion);

	SceneObjects.Remove(nullptr);
}

void AFICScene::MoveSceneObject(UObject* Object, int Delta) {
	int Index = SceneObjects.Find(Object);
	SceneObjects.RemoveAt(Index);
	SceneObjects.Insert(Object, UFICUtils::Modulo(Index + Delta, SceneObjects.Num()+1));
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
