#pragma once

#include "CoreMinimal.h"
#include "FICSceneObject3D.h"
#include "Data/Attributes/FICAttributeBool.h"
#include "Data/Objects/FICSceneObject.h"
#include "Editor/ITF/FICSelectionInteraction.h"
#include "Components/BoxComponent.h"
#include "FICDismantleZone.generated.h"

class AFICParticleSystemActor;

UCLASS()
class FICSITCAM_API UFICDismantleZone : public UObject, public FTickableGameObject, public IFGSaveInterface, public IFICSceneObject, public IFICSceneObject3D {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FString SceneObjectName = TEXT("DismantleZone");
	
	UPROPERTY(SaveGame)
	FFICAttributeBool Active;
	
	UPROPERTY(SaveGame)
	FVector Position;
	UPROPERTY(SaveGame)
	FRotator Rotation;
	UPROPERTY(SaveGame)
	FVector Size;
	UPROPERTY(SaveGame)
	int64 DurationFrames = 600;

	UPROPERTY()
	AFICDismantleZoneActor* DismantleZoneActor = nullptr;

	TOptional<FICFrameFloat> AnimationProgression;

	TSet<TTuple<UClass*,int64>> DismantledInstances;
	TMap<UObject*, TTuple<UClass*,int64>> DismantledObjects;

	UPROPERTY()
	UFICEditorContext* EditorContext = nullptr;

	UFICDismantleZone() {
		Active.SetDefaultValue(false);
		
		RootAttribute.AddChildAttribute(TEXT("Active"), &Active);

		Size = FVector(1000);
	}

	// Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
	virtual TStatId GetStatId() const override { return UObject::GetStatID(); }
	// End FTickableGameObject

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
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

	virtual void InitAnimation() override;
	virtual void TickAnimation(FICFrameFloat Frame) override;
	virtual void ShutdownAnimation() override;
	// End IFICSceneObject-Interface

	// Begin IFICSceneObject3D
	virtual bool Is3DSceneObject() override { return true; }
	virtual ETransformGizmoSubElements GetGizmoSubElements() { return ETransformGizmoSubElements::FullTranslateRotateScale; }
	virtual FTransform GetSceneObjectTransform() override;
	virtual void SetSceneObjectTransform(FTransform InTransform) override;
	virtual AActor* GetActor() override;
	// End IFICSceneObject3D

	void HandleProgression(TOptional<FICFrameFloat> frame, bool bDrawProgression = false);
	void UpdateActivation();
};

UCLASS()
class FICSITCAM_API AFICDismantleZoneActor : public AActor, public IFICSelectionInteractionTarget {
	GENERATED_BODY()

public:
	UPROPERTY()
	UFICDismantleZone* DismantleZone = nullptr;
	UPROPERTY()
	UBoxComponent* Collision = nullptr;

	AFICDismantleZoneActor();

	virtual void Tick(float DeltaSeconds) override;

	// Begin IFICSelectionInteractionTarget
	virtual UObject* Select() { return DismantleZone; }
	// End IFICSelectionInteractionTarget
};