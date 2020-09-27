#include "FICSubsystem.h"


#include "FICCommand.h"
#include "FICSubsystemHolder.h"
#include "mod/ModSubsystems.h"
#include "UI/FICEditor.h"

AFICSubsystem::AFICSubsystem() {
	Input = CreateDefaultSubobject<UInputComponent>("Input");
	Input->BindKey(FInputChord(EKeys::I, false, true, false, false), EInputEvent::IE_Pressed, this, &AFICSubsystem::CreateKeypointPressed);
	
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFICSubsystem::BeginPlay() {
	Super::BeginPlay();

	Camera = GetWorld()->SpawnActor<AFICCamera>();

	AChatCommandSubsystem::Get(this)->RegisterCommand(AFICCommand::StaticClass());
}

void AFICSubsystem::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	GetWorld()->GetFirstPlayerController()->PushInputComponent(Input);
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
	return GetSubsystemHolder<UFICSubsystemHolder>(WorldContext)->Subsystem;
}

void AFICSubsystem::PlayAnimation(UFICAnimation* Path) {
	if (!Camera || !Path) return;
	Camera->StartAnimation(Path);
}

void AFICSubsystem::StopAnimation() {
	if (!Camera) return;
	Camera->StopAnimation();
}

void AFICSubsystem::AddVisibleAnimation(UFICAnimation* Path) {
	if (Path) VisibleAnimations.Add(Path);
}

void AFICSubsystem::CreateKeypointPressed() {
	
}

void AFICSubsystem::SetActiveAnimation(UFICAnimation* inActiveAnimation) {
	if (EditorContext) {
		EditorContext->HideEditor();
		EditorContext = nullptr;
	}
	ActiveAnimation = inActiveAnimation;
	if (ActiveAnimation) {
		EditorContext = NewObject<UFICEditorContext>(this);
		EditorContext->Animation = ActiveAnimation;
		EditorContext->ShowEditor();
	}
}

UFICAnimation* AFICSubsystem::GetActiveAnimation() const {
	return ActiveAnimation;
}
