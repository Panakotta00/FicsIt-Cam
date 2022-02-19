#pragma once

#include "CoreMinimal.h"
#include "FGPlayerController.h"
#include "InteractiveToolsContext.h"
#include "ITF/FICToolsContextQueries.h"
#include "ITF/FICToolsContextRenderComponent.h"
#include "ITF/FICToolsContextTransactions.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "UI/FICEditor.h"
#include "FICEditorSubsystem.generated.h"

class UFICTransformInteraction;
class UFICSelectionInteraction;
class AFICEditorCameraCharacter;
class AFICScene;
class UFICEditorContext;

UCLASS()
class FICSITCAM_API AFICEditorSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFICEditorContext* ActiveEditorContext = nullptr;
	UPROPERTY()
	AFICEditorCameraCharacter* EditorPlayerCharacter = nullptr;
	TSharedPtr<SFICEditor> EditorWidget;
	
	UPROPERTY()
	ACharacter* OriginalPlayerCharacter = nullptr;
	TSharedPtr<SViewport> GameViewport;
	TSharedPtr<SVerticalBox> GameViewportContainer;
	TSharedPtr<SOverlay> GameOverlay;

	// Interactive Tools
	UPROPERTY()
	AActor* PDIRenderActor;
	UPROPERTY()
	UFICToolsContextRenderComponent* PDIRenderComponent;
	UPROPERTY()
	UFICSelectionInteraction* SelectionInteraction;
	UPROPERTY()
	UFICTransformInteraction* TransformInteraction;
	FVector2D PrevMousePosition = FVector2D::ZeroVector;
	FInputDeviceState CurrentMouseState;
	bool bPendingMouseStateChange = false;
	FViewCameraState CurrentViewCameraState;
	TSharedPtr<FFICToolsContextTransactions> ToolsTransactions;
	TSharedPtr<FFICToolsContextQueries> ToolsQueries;
	UPROPERTY()
	AActor* Selection = nullptr;

	void InitInteractiveTools();
	void ShutdownInteractiveTools();

public:
	UPROPERTY()
	UInteractiveToolsContext* ToolsContext;
	DECLARE_MULTICAST_DELEGATE(FSelectionChanged)
	FSelectionChanged OnSelectionChanged;
	void OnLeftMouseDown();
	void OnLeftMouseUp();
	
	static AFICEditorSubsystem* GetFICEditorSubsystem(UObject* WorldContext) {
		USubsystemActorManager* SubsystemActorManager = WorldContext->GetWorld()->GetSubsystem<USubsystemActorManager>();
		check(SubsystemActorManager);
		return SubsystemActorManager->GetSubsystemActor<AFICEditorSubsystem>();
	}

	UPROPERTY(SaveGame)
	TMap<FString, FString> EditorLayouts;
	UPROPERTY(SaveGame)
	FString LastEditorLayout;
	
	AFICEditorSubsystem();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	// End IFGSaveInterface

	void OpenEditor(AFICScene* InScene);
	void CloseEditor();

	UFICEditorContext* GetActiveEditorContext() { return ActiveEditorContext; }
	AFICEditorCameraCharacter* GetEditorPlayerCharacter() { return EditorPlayerCharacter; }
	TSharedPtr<SFICEditor> GetEditorWidget() { return EditorWidget; }

	void SetMeshInstanced(UMeshComponent* MeshComp, bool Instanced);
	void PatchActor(AActor* Actor, bool Instanced) {
		for (UActorComponent* Comp : Actor->GetComponents()) {
			if (UMeshComponent* MeshComp = Cast<UMeshComponent>(Comp)) {
				SetMeshInstanced(MeshComp, Instanced);
			}
		}
	}

	void SetSelection(AActor* Actor) {
		if (Selection == Actor) return;
		if (Selection) {
			PatchActor(Selection, true);
		}
		Selection = Actor;
		if (Selection) {
			PatchActor(Selection, false);
		}
		OnSelectionChanged.Broadcast();
	}

	AActor* GetSelection() {
		return Selection;
	}
};
