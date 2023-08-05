﻿#pragma once

#include "FICChangeList.h"
#include "FICEditorCameraCharacter.h"
#include "Data/FICActiveSceneObjectManager.h"
#include "Data/FICScene.h"
#include "FICEditorContext.generated.h"

class FFICEditorAttributeBase;
class FFICEditorAttributeGroupDynamic;

#define FICAttributeCacheFuncs(Name) \
	void Set ## Name (float inVal) { \
		if (Name == inVal) return; \
		Name = inVal; \
		UpdateCharacterValues(); \
	} \
    float Get ## Name () const { return Name; }

class UFICCamera;
class ACharacter;

UENUM()
enum EFICAnimPlayerState {
	FIC_PLAY_PAUSED,
	FIC_PLAY_FORWARDS,
	FIC_PLAY_BACKWARDS,
};

DECLARE_MULTICAST_DELEGATE(FFICSceneObjectsChanged)
DECLARE_MULTICAST_DELEGATE(FFICCurrentFrameChanged)
DECLARE_MULTICAST_DELEGATE(FFICOverlayWidgetsChanged)

UCLASS()
class UFICEditorContext : public UObject, public FTickableGameObject {
	GENERATED_BODY()

private:
	UPROPERTY()
	AFICScene* Scene = nullptr;

	UPROPERTY()
	AFICEditorCameraCharacter* EditorPlayerCharacter = nullptr;
	
	FICFrame CurrentFrame = 0;
	FFICFrameRange ActiveRange;

	TSharedPtr<FFICEditorAttributeGroupDynamic> AllAttributes;

	TMap<UObject*, TSharedRef<FFICEditorAttributeBase>> EditorAttributes;
	TMap<UObject*, FDelegateHandle> DataAttributeOnUpdateDelegateHandles;
	TMap<FFICAttribute*, TSharedRef<FFICEditorAttributeBase>> EditorAttributeMap;

	EFICAnimPlayerState AnimPlayerState = FIC_PLAY_PAUSED;
	float AnimPlayerDelta = 0.0f;
	float AnimPlayerFactor = 1.0f;

	UPROPERTY()
	UObject* SelectedSceneObject = nullptr;

	TArray<TSharedRef<SWidget>> OverlayWidgets;
	
	bool bAutoKeyframe = false;
	bool bMoveCamera = true;
	bool bCameraPreview = false;

	bool bBlockValueUpdate = false;
	void* AutoKeyframeChangeRef = nullptr;

	FFICActiveSceneObjectManager ActiveSceneObjectManager;
	
public:
	bool bShowPath = true;
	bool bForceResolution = false;
	bool bInAutoKeyframeSet = false;

	float SensorWidthAdjust = 1.0f;

	FFICChangeList ChangeList;
	
	FFICSceneObjectsChanged OnSceneObjectsChanged;
	FFICSceneObjectsChanged OnSceneObjectSelectionChanged;
	FFICCurrentFrameChanged OnCurrentFrameChanged;
	FFICOverlayWidgetsChanged OnOverlayWidgetsChanged;
		
	UFICEditorContext();

	// Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override { return UObject::GetStatID(); }
	// End FTickableGameObject

	void LoadSceneObject(UObject* SceneObject);
	void UnloadSceneObject(UObject* SceneObject);
	void AddSceneObject(UObject* SceneObject);
	void RemoveSceneObject(UObject* SceneObject);
	void MoveSceneObject(UObject* SceneObject, int Delta);
	UObject* FindSceneObject(FString SceneObjectName) const;
	
	AFICScene* GetScene() const;
	UFICCamera* GetCamera();
	TSharedPtr<FFICEditorAttributeBase> GetCameraEditor();
	UFICCamera* GetActiveCamera() {
		return GetCamera();
	}
	
	void SetCurrentFrame(int64 inFrame);
	int64 GetCurrentFrame() const;

	void SetActiveRange(const FFICFrameRange& InActiveRange);
	FFICFrameRange GetActiveRange();

	void SetSelectedSceneObject(UObject* SceneObject);
	UObject* GetSelectedSceneObject();

	void SetAutoKeyframe(bool bAutokeyframe);
	bool GetAutoKeyframe() { return bAutoKeyframe; }

	void SetLockCameraToView(bool bInLockCameraToView);
	bool GetLockCameraToView() { return bMoveCamera; }

	void AddOverlayWidget(TSharedRef<SWidget> Widget) {
		OverlayWidgets.Add(Widget);
		OnOverlayWidgetsChanged.Broadcast();
	}
	void RemoveOverlayWidget(TSharedRef<SWidget> Widget) {
		OverlayWidgets.Remove(Widget);
		OnOverlayWidgetsChanged.Broadcast();
	}
	TArray<TSharedRef<SWidget>> GetOverlayWidgets() {
		return OverlayWidgets;
	}

	void SetCameraPreview(bool bInCameraPreview) { bCameraPreview = bInCameraPreview; SetSelectedSceneObject(GetSelectedSceneObject()); }
	bool GetCameraPreview() { return bCameraPreview; }
	
	void SetAnimPlayer(EFICAnimPlayerState InAnimPlayerState, float InAnimPlayerFactor);
	EFICAnimPlayerState GetAnimPlayer() { return AnimPlayerState; }
	float GetAnimPlayerFactor() { return AnimPlayerFactor; }

	TSharedPtr<FFICEditorAttributeGroupDynamic> GetAllAttributes() { return AllAttributes; }
	const TMap<UObject*, TSharedRef<FFICEditorAttributeBase>>& GetEditorAttributes() { return EditorAttributes; }

	AFICEditorCameraCharacter* GetPlayerCharacter() { return EditorPlayerCharacter; }

	bool bViewportCameraControl = false;
	
	bool IsViewportCameraControl() {
		return bViewportCameraControl;
	}

	void ToggleCurrentKeyframes();

	void CommitAutoKeyframe(void* Ref) {
		AutoKeyframeChangeRef = Ref;
	}

	/**
	 * Called after the Context Object got created.
	 * Used to load a scene into the editor, create the editor attributes, load the scene objects etc.
	 * The Context should be handled as invalid until this function is called and after unload is called.
	 */
	void Load(AFICEditorCameraCharacter* InEditorPlayerCharacter, AFICScene* InScene);

	/**
	 * Called before the context object gets destroyed and the editor closed.
	 * Should remove everything used by the editor in the world and cleans up actor states etc.
	 * Should unload scene objects.
	 */
	void Unload();

	/**
	 * Called by the Active Camera to notify the editor character of a value change of the active camera.
	 * This is mainly important for "lock view to camera".
	 * If the "lock view to camera" flag is not set, the function does nothing, otherwise it will redirect the notification to the editor character.
	 */
	void UpdateCharacterValues();
};
