#pragma once

#include "FICChangeList.h"
#include "FICEditorCameraCharacter.h"
#include "Data/FICAnimation.h"
#include "Data/FICEditorAttributeGroupDynamic.h"
#include "Data/FICScene.h"
#include "UI/FICEditor.h"
#include "FICEditorContext.generated.h"

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

UCLASS()
class UFICEditorContext : public UObject, public FTickableGameObject {
	GENERATED_BODY()

private:
	UPROPERTY()
	AFICScene* Scene = nullptr;

	FICFrame CurrentFrame = 0;
	FFICFrameRange ActiveRange;

	TSharedPtr<FFICEditorAttributeGroupDynamic> AllAttributes;
	TMap<UObject*, TSharedRef<FFICEditorAttributeBase>> EditorAttributes;

	UPROPERTY()
	ACharacter* OriginalCharacter = nullptr;

	UPROPERTY()
	AFICEditorCameraCharacter* CameraCharacter = nullptr;

	TSharedPtr<SViewport> GameViewport;
	TSharedPtr<SVerticalBox> GameViewportContainer;
	TSharedPtr<SOverlay> GameOverlay;

	EFICAnimPlayerState AnimPlayerState = FIC_PLAY_PAUSED;
	float AnimPlayerDelta = 0.0f;
	float AnimPlayerFactor = 1.0f;
	
public:
	bool bMoveCamera = true;
	bool bShowPath = true;
	bool bAutoKeyframe = false;
	bool bForceResolution = false;

	float SensorWidthAdjust = 1.0f;

	bool IsEditorShown = false;
	bool IsEditorShowing = false;
	bool TempViewportFocus = false;
	FVector2D TempCursorPos;
	
	TSharedPtr<SFICEditor> EditorWidget;

	FFICChangeList ChangeList;

	UFICEditorContext();

	// Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override { return UObject::GetStatID(); }
	// End FTickableGameObject
	
	void SetScene(AFICScene* Scene);
	AFICScene* GetScene() const;
	UFICCamera* GetCamera();
	TSharedPtr<FFICEditorAttributeBase> GetCameraEditor();
	
	void SetCurrentFrame(int64 inFrame);
	int64 GetCurrentFrame() const;

	void SetActiveRange(const FFICFrameRange& InActiveRange);
	FFICFrameRange GetActiveRange();

	void SetFlySpeed(float Speed);
	float GetFlySpeed();

	UFUNCTION()
	void UpdateCharacterValues();

	void ShowEditor();
	void HideEditor();

	void SetAnimPlayer(EFICAnimPlayerState InAnimPlayerState, float InAnimPlayerFactor);
	EFICAnimPlayerState GetAnimPlayer() { return AnimPlayerState; }
	float GetAnimPlayerFactor() { return AnimPlayerFactor; }

	TSharedPtr<FFICEditorAttributeGroupDynamic> GetAllAttributes() { return AllAttributes; }
	const TMap<UObject*, TSharedRef<FFICEditorAttributeBase>>& GetEditorAttributes() { return EditorAttributes; }
};
