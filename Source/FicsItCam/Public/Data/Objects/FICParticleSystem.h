#pragma once

#pragma once

#include "CoreMinimal.h"
#include "FICSceneObject3D.h"
#include "Data/Attributes/FICAttributeBool.h"
#include "Data/Attributes/FICAttributePosition.h"
#include "Data/Attributes/FICAttributeRotation.h"
#include "Data/Objects/FICSceneObject.h"
#include "Editor/ITF/FICSelectionInteraction.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "FICParticleSystem.generated.h"

class AFICParticleSystemActor;
class USphereComponent;

UCLASS()
class FICSITCAM_API UFICParticleSystem : public UObject, public FTickableGameObject, public IFICSceneObject, public IFICSceneObject3D {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FString SceneObjectName = TEXT("ParticleSystem");
	
	UPROPERTY(SaveGame)
	FFICAttributeBool Active;
	
	UPROPERTY(SaveGame)
	FFICAttributePosition Position;
	UPROPERTY(SaveGame)
	FFICAttributeRotation Rotation;

	UPROPERTY(SaveGame)
	UParticleSystem* ParticleSystem = nullptr;
	
	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;
	UPROPERTY()
	AFICParticleSystemActor* ParticleSystemActor = nullptr;

	UFICParticleSystem() {
		Active.SetDefaultValue(true);
		
		RootAttribute.AddChildAttribute(TEXT("Active"), &Active);
		RootAttribute.AddChildAttribute(TEXT("Position"), &Position);
		RootAttribute.AddChildAttribute(TEXT("Rotation"), &Rotation);
	}

	// Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { return UObject::GetStatID(); }
	// End FTickableGameObject

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

	virtual void InitAnimation() override;
	virtual void TickAnimation(FICFrameFloat Frame) override;
	virtual void ShutdownAnimation() override;
	// End IFICSceneObject-Interface

	// Begin IFICSceneObject3D
	virtual bool Is3DSceneObject() override { return !!EditorContext; }
	virtual ETransformGizmoSubElements GetGizmoSubElements() { return ETransformGizmoSubElements::StandardTranslateRotate; }
	virtual FTransform GetSceneObjectTransform() override;
	virtual void SetSceneObjectTransform(FTransform InTransform) override;
	// End IFICSceneObject3D

	void SetParticleSystem(UParticleSystem* InParticleSystem);
};

UCLASS()
class FICSITCAM_API AFICParticleSystemActor : public AActor, public IFICSelectionInteractionTarget {
	GENERATED_BODY()

public:
	UPROPERTY()
	UParticleSystemComponent* ParticleSystemComponent;
	UPROPERTY()
	USphereComponent* Collision;

	UPROPERTY()
	class UFICParticleSystem* ParticleSystem = nullptr;

	AFICParticleSystemActor();

	// Begin IFICSelectionInteractionTarget
	virtual UObject* Select() { return ParticleSystem; }
	// End IFICSelectionInteractionTarget
};