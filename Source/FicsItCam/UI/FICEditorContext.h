#pragma once

#include "FicsItCam/FICAnimation.h"
#include "FicsItCam/FICEditorCameraCharacter.h"
#include "FICEditor.h"
#include "FicsItCam/FICEditorAttributeBase.h"
#include "FICEditorContext.generated.h"

#define FICAttributeCacheFuncs(Name) \
	void Set ## Name (float inVal) { \
		if (Name == inVal) return; \
		Name = inVal; \
		UpdateCharacterValues(); \
	} \
    float Get ## Name () const { return Name; }

class ACharacter;

UCLASS()
class UFICEditorContext : public UObject {
	GENERATED_BODY()

private:
	UPROPERTY()
	AFICAnimation* Animation = nullptr;

	UPROPERTY()
	int64 CurrentFrame = 0;

	UPROPERTY()
	ACharacter* OriginalCharacter = nullptr;

	UPROPERTY()
	AFICEditorCameraCharacter* CameraCharacter = nullptr;

	TSharedPtr<SFICEditor> EditorWidget;
	TSharedPtr<SViewport> GameViewport;
	TSharedPtr<SVerticalBox> GameViewportContainer;
	TSharedPtr<SOverlay> GameOverlay;

public:
	TFICEditorAttribute<FFICFloatAttribute> PosX;
	TFICEditorAttribute<FFICFloatAttribute> PosY;
	TFICEditorAttribute<FFICFloatAttribute> PosZ;
	TFICEditorAttribute<FFICFloatAttribute> RotPitch;
	TFICEditorAttribute<FFICFloatAttribute> RotYaw;
	TFICEditorAttribute<FFICFloatAttribute> RotRoll;
	TFICEditorAttribute<FFICFloatAttribute> FOV;
	FFICEditorGroupAttribute All;
	FFICEditorGroupAttribute Pos;
	bool bMoveCamera = true;
	bool bShowPath = true;
	bool bAutoKeyframe = false;

	UFICEditorContext();
	
	void SetAnimation(AFICAnimation* Anim);
	AFICAnimation* GetAnimation() const;
	
	void SetCurrentFrame(int64 inFrame);
	int64 GetCurrentFrame() const;

	void SetFlySpeed(float Speed);
	float GetFlySpeed();

	UFUNCTION()
	void UpdateCharacterValues();

	void ShowEditor();
	void HideEditor();
};
