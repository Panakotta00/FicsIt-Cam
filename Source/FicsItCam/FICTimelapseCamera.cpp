#include "FICTimelapseCamera.h"

#include "FGGameUserSettings.h"
#include "FGPlayerController.h"
#include "FICUtils.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

AFICTimelapseCamera::AFICTimelapseCamera() {
	RenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>("RenderTarget");
	
	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>("CaptureComponent");
	CaptureComponent->SetupAttachment(RootComponent);
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
	CaptureComponent->bUseRayTracingIfEnabled = true;
	CaptureComponent->ShowFlags.SetTemporalAA(true);
	CaptureComponent->TextureTarget = RenderTarget;
}

void AFICTimelapseCamera::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);

	FIntPoint Resolution = UFGGameUserSettings::GetFGGameUserSettings()->GetScreenResolution();
	RenderTarget->InitCustomFormat(Resolution.X, Resolution.Y, EPixelFormat::PF_R8G8B8A8, false);
}

void AFICTimelapseCamera::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	StopTimelapse();
}

void AFICTimelapseCamera::CaptureTick() {
	AFGPlayerController* Controller = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
	AFGCharacterPlayer* Character = Cast<AFGCharacterPlayer>(Controller->GetCharacter());
	if (Character) Character->SetThirdPersonMode();
	
	CaptureComponent->CaptureScene();

	FString FSP;
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	if (FSP.IsEmpty()) {
		FSP = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/") TEXT("SaveGames/"));
	}

	FSP = FPaths::Combine(FSP, TEXT("FicsItCam/"), GetName());

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*FSP)) PlatformFile.CreateDirectoryTree(*FSP);

	FSP = FPaths::Combine(FSP, FString::Printf(TEXT("%s-%i.jpg"), *CaptureStart.ToString(), CaptureIncrement));

	bool bSuccess = FIC_SaveRenderTargetAsJPG(FSP, RenderTarget);
	if (bSuccess) ++CaptureIncrement;

	if (Character) Character->SetFirstPersonMode();
}

void AFICTimelapseCamera::StartTimelapse() {
	CaptureIncrement = 0;
	CaptureStart = FDateTime::Now();
	GetWorld()->GetTimerManager().SetTimer(CaptureTimer, this, &AFICTimelapseCamera::CaptureTick, Frequency, true);
}

void AFICTimelapseCamera::StopTimelapse() {
	GetWorld()->GetTimerManager().ClearTimer(CaptureTimer);
}

