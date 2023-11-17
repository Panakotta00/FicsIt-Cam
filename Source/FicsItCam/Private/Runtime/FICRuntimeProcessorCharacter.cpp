#include "Runtime/FICRuntimeProcessorCharacter.h"

#include "CineCameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FGInputSettings.h"
#include "Engine/World.h"
#include "FGPlayerController.h"
#include "FGPlayerInput.h"
#include "FICSubsystem.h"
#include "Input/FGEnhancedInputComponent.h"
#include "Input/FGInputMappingContext.h"
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

	InputAction_StopPlayback = ConstructorHelpers::FObjectFinder<UInputAction>(L"/FicsItCam/Input/IA_FIC_Playback_StopAnimation.IA_FIC_Playback_StopAnimation").Object;
}

void AFICRuntimeProcessorCharacter::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
}

void AFICRuntimeProcessorCharacter::BeginPlay() {
	Super::BeginPlay();
}

void AFICRuntimeProcessorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	UFGEnhancedInputComponent* EnhancedInputComponent = Cast<UFGEnhancedInputComponent>(PlayerInputComponent);
	const UFGInputSettings* Settings = UFGInputSettings::Get();
	
	EnhancedInputComponent->BindAction(InputAction_StopPlayback, ETriggerEvent::Triggered, this, &AFICRuntimeProcessorCharacter::StopProcess);
}

void AFICRuntimeProcessorCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	if (bIsRepossesing) return;
	if (NewController) {
		AFGPlayerController* PlayerController = Cast<AFGPlayerController>(NewController);
		NewController->DisableInput(PlayerController);
		PlayerController->GetHUD<AFGHUD>()->SetPumpiMode(true);

		UFGInputMappingContext* InputMappingContext = LoadObject<UFGInputMappingContext>(nullptr, TEXT("/FicsItCam/Input/IC_FIC_Playback.IC_FIC_Playback"));
		PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->AddMappingContext(InputMappingContext, -2);
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
		UFGInputMappingContext* InputMappingContext = LoadObject<UFGInputMappingContext>(nullptr, TEXT("/FicsItCam/Input/IC_FIC_Playback.IC_FIC_Playback"));
		PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->RemoveMappingContext(InputMappingContext);
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
