#pragma once

#include "CoreMinimal.h"
#include "FGPlayerController.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "UI/FICEditor.h"
#include "FICEditorSubsystem.generated.h"

class AFICEditorCameraCharacter;
class AFICScene;
class UFICEditorContext;

UCLASS()
class FICSITCAM_API AFICEditorSubsystem : public AModSubsystem {
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
	
public:
	static AFICEditorSubsystem* GetFICEditorSubsystem(UObject* WorldContext) {
		USubsystemActorManager* SubsystemActorManager = WorldContext->GetWorld()->GetSubsystem<USubsystemActorManager>();
		check(SubsystemActorManager);
		return SubsystemActorManager->GetSubsystemActor<AFICEditorSubsystem>();
	}
	
	AFICEditorSubsystem();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	// End AActor

	void OpenEditor(AFICScene* InScene);
	void CloseEditor();

	UFICEditorContext* GetActiveEditorContext() { return ActiveEditorContext; }
	AFICEditorCameraCharacter* GetEditorPlayerCharacter() { return EditorPlayerCharacter; }
	TSharedPtr<SFICEditor> GetEditorWidget() { return EditorWidget; }
};
