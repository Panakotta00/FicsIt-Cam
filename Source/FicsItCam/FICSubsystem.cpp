#include "FICSubsystem.h"

#include <Subsystem/SubsystemActorManager.h>

#include "FICCommand.h"
#include "Engine/World.h"

AFICSubsystem::AFICSubsystem() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFICSubsystem::BeginPlay() {
	Super::BeginPlay();
}

void AFICSubsystem::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

void AFICSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	
	if (Camera) Camera->Destroy();
	Camera = nullptr;
}

bool AFICSubsystem::ShouldSave_Implementation() const {
	return true;
}

AFICSubsystem* AFICSubsystem::GetFICSubsystem(UObject* WorldContext) {
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFICSubsystem>();
}

void AFICSubsystem::PlayAnimation(AFICAnimation* Path, bool bDoRender) {
	CreateCamera();
	if (!Camera || !Path) return;
	Camera->StartAnimation(Path, bDoRender);
}

void AFICSubsystem::StopAnimation() {
	if (!Camera) return;
	Camera->StopAnimation();
}

void AFICSubsystem::AddVisibleAnimation(AFICAnimation* Path) {
	if (Path) VisibleAnimations.Add(Path);
}

void AFICSubsystem::SetActiveAnimation(AFICAnimation* inActiveAnimation) {
	if (EditorContext) {
		EditorContext->HideEditor();
		EditorContext = nullptr;
	}
	ActiveAnimation = inActiveAnimation;
	if (ActiveAnimation) {
		EditorContext = NewObject<UFICEditorContext>(this);
		EditorContext->SetAnimation(ActiveAnimation);
		EditorContext->ShowEditor();
	}
}

UFICEditorContext* AFICSubsystem::GetEditor() const {
	return EditorContext;
}

void AFICSubsystem::CreateCamera() {
	if (!Camera) Camera = GetWorld()->SpawnActor<AFICCameraCharacter>();
}
