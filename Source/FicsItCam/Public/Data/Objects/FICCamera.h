#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FICSceneObject3D.h"
#include "FICSceneObjectActive.h"
#include "Data/Attributes/FICAttributeBool.h"
#include "Data/Attributes/FICAttributePosition.h"
#include "Data/Attributes/FICAttributeRotation.h"
#include "Data/Objects/FICSceneObject.h"
#include "FICCamera.generated.h"

class AFICScene;
class AFICEditorCameraActor;

UCLASS()
class FICSITCAM_API UFICCamera : public UObject, public FTickableGameObject, public IFICSceneObject, public IFICSceneObject3D, public IFICSceneObjectActive, public IFGSaveInterface {
	GENERATED_BODY()
private:
	TSharedPtr<SWidget> CameraPreviewWidget;
	
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
	
	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;
	UPROPERTY()
	AFICEditorCameraActor* EditorCameraActor = nullptr;
	
	UFICCamera() {
		Active.SetDefaultValue(true);
		Aperture.SetDefaultValue(10);
		FocusDistance.SetDefaultValue(10000);
		
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

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return GetWorld() != nullptr; }
	// End IFGSaveInterface

	// Begin IFICSceneObject-Interface
	virtual FString GetSceneObjectName() override { return SceneObjectName; }
	virtual void SetSceneObjectName(FString Name) override { SceneObjectName = Name; }
	
	virtual UObject* CreateNewObject(UObject* InOuter, AFICScene* InScene) override;
	virtual TSharedRef<SWidget> CreateDetailsWidget(UFICEditorContext* InContext) override;
	
	virtual void InitEditor(UFICEditorContext* Context) override;
	virtual void ShutdownEditor(UFICEditorContext* Context) override;
	virtual void EditorUpdate(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) override;
	virtual void Select(UFICEditorContext* Context) override;
	virtual void Unselect(UFICEditorContext* Context) override;
	// End IFICSceneObject-Interface

	// Begin IFICSceneObject3D
	virtual bool Is3DSceneObject() override { return !!EditorContext; }
	virtual ETransformGizmoSubElements GetGizmoSubElements() { return ETransformGizmoSubElements::StandardTranslateRotate; }
	virtual FTransform GetSceneObjectTransform() override;
	virtual void SetSceneObjectTransform(FTransform InTransform) override;
	virtual AActor* GetActor() override;
	// End IFICSceneObject3D

	// Begin IFICSceneObjectActive
	virtual FString GetActiveType() { return TEXT("Camera"); }
	virtual FFICAttributeBool& GetActiveAttribute() { return Active; }
	// End IFICSceneObjectActive
};
