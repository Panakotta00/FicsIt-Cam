#pragma once

#include "CoreMinimal.h"
#include "Data/Attributes/FICAttributeBool.h"
#include "Data/Attributes/FICAttributePosition.h"
#include "Data/Attributes/FICAttributeRotation.h"
#include "Data/Objects/FICSceneObject.h"
#include "FICCamera.generated.h"

class AFICEditorCameraActor;

UCLASS()
class FICSITCAM_API UFICCamera : public UObject, public FTickableGameObject, public IFICSceneObject {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FString SceneObjectName = TEXT("Camera");
	
	UPROPERTY(SaveGame)
	FFICAttributeBool Active;
	
	UPROPERTY(SaveGame)
	FFICAttributePosition Position;
	UPROPERTY(SaveGame)
	FFICAttributeRotation Rotation;
	
	UPROPERTY(SaveGame)
	FFICFloatAttribute FOV;
	UPROPERTY(SaveGame)
	FFICFloatAttribute Aperture;
	UPROPERTY(SaveGame)
	FFICFloatAttribute FocusDistance;

	FFICGroupAttribute LensSettings;
	FFICGroupAttribute RootAttribute;

	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;
	UPROPERTY()
	AFICEditorCameraActor* EditorCameraActor = nullptr;

	UFICCamera() {
		Active.SetDefaultValue(true);
		
		LensSettings.AddChildAttribute(TEXT("FOV"), &FOV);
		LensSettings.AddChildAttribute(TEXT("Aperture"), &Aperture);
		LensSettings.AddChildAttribute(TEXT("Focus Distance"), &FocusDistance);

		RootAttribute.AddChildAttribute(TEXT("Active"), &Active);
		RootAttribute.AddChildAttribute(TEXT("Position"), &Position);
		RootAttribute.AddChildAttribute(TEXT("Rotation"), &Rotation);
		RootAttribute.AddChildAttribute(TEXT("Lens Settings"), &LensSettings);
	}

	// Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { return UObject::GetStatID(); }
	// End FTickableGameObject

	// Begin IFICEditorSceneObject-Interface
	virtual FString GetSceneObjectName() override {
		return SceneObjectName;
	}

	virtual void SetSceneObjectName(FString Name) override;
	
	virtual FFICAttribute& GetRootAttribute() override {
		return RootAttribute;
	}

	virtual void InitEditor(UFICEditorContext* Context) override;
	virtual void UnloadEditor(UFICEditorContext* Context) override;
	virtual void EditorUpdate(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) override;

	virtual bool Is3DSceneObject() override;
	virtual ETransformGizmoSubElements GetGizmoSubElements() { return ETransformGizmoSubElements::StandardTranslateRotate; }
	virtual FTransform GetSceneObjectTransform() override;
	virtual void SetSceneObjectTransform(FTransform InTransform) override;
	// End IFICEditorSceneObject-Interface
};
