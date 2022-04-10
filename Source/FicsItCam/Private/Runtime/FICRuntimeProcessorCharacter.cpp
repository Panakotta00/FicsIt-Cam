#include "Runtime/FICRuntimeProcessorCharacter.h"

#include "CineCameraComponent.h"
#include "Engine/World.h"
#include "FGPlayerController.h"
#include "FICSubsystem.h"
#include "Runtime/Process/FICRuntimeProcess.h"

void AFICRuntimeProcessorCharacter::StopProcess() {
	AFICSubsystem::GetFICSubsystem(this)->RemoveRuntimeProcess(RuntimeProcess);
}

AFICRuntimeProcessorCharacter::AFICRuntimeProcessorCharacter() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	bSimGravityDisabled = true;
	
	SetActorEnableCollision(false);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AFICRuntimeProcessorCharacter::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
}

void AFICRuntimeProcessorCharacter::BeginPlay() {
	Super::BeginPlay();
}

void AFICRuntimeProcessorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	PlayerInputComponent->BindAction("FicsItCam.StopAnimation", EInputEvent::IE_Pressed, this, &AFICRuntimeProcessorCharacter::StopProcess);
}

void AFICRuntimeProcessorCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	if (bIsRepossesing) return;
	if (NewController) {
		AFGPlayerController* PlayerController = Cast<AFGPlayerController>(NewController);
		NewController->DisableInput(PlayerController);
		PlayerController->GetHUD<AFGHUD>()->SetPumpiMode(true);
	}
}

void AFICRuntimeProcessorCharacter::UnPossessed() {
	AController* OldController = GetController();

	Super::UnPossessed();
	
	if (bIsRepossesing) return;
	
	SetTimeDilation(1);
	
	if (OldController) {
		// Make sure if not able to recover, game is interactable
		AFGPlayerController* PlayerController = Cast<AFGPlayerController>(OldController);
		PlayerController->GetHUD<AFGHUD>()->SetPumpiMode(false);
		OldController->EnableInput(PlayerController);

		// Force Remove Process
		AFICSubsystem* SubSys = AFICSubsystem::GetFICSubsystem(this);
		if (SubSys->GetRuntimeProcessorCharacter() == this) SubSys->RemoveRuntimeProcess(RuntimeProcess);
		// TODO: Try to do recover
	}
}

void AFICRuntimeProcessorCharacter::Initialize(UFICRuntimeProcess* InRuntimeProcess) {
	RuntimeProcess = InRuntimeProcess;
}

void AFICRuntimeProcessorCharacter::Shutdown() {
	SetTimeDilation(1);
}

void AFICRuntimeProcessorCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

void AFICRuntimeProcessorCharacter::SetTimeDilation(float InTimeDilation) {
	if (InTimeDilation < 0.0001) InTimeDilation = 0.0001;
	UGameplayStatics::SetGlobalTimeDilation(this, InTimeDilation);
	CustomTimeDilation = 1.0f/InTimeDilation;
}

void AFICRuntimeProcessorCharacter::SetCamera(bool bEnabled, bool bCinematic) {
	if (Camera) {
		Camera->SetActive(false);
		Camera->DestroyComponent();
		Camera = nullptr;
	}

	if (bEnabled) {
		if (bCinematic) Camera = NewObject<UCineCameraComponent>(this);
		else Camera = NewObject<UCameraComponent>(this);
		Camera->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		Camera->SetActive(true);
	}
}
