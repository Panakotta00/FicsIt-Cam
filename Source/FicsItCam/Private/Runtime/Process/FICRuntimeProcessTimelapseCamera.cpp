#include "Runtime/Process/FICRuntimeProcessTimelapseCamera.h"

#include "FGPlayerController.h"
#include "FICSubsystem.h"
#include "PlatformFileManager.h"
#include "SceneViewport.h"
#include "TextureResource.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Runtime/FICCaptureCamera.h"

void UFICRuntimeProcessTimelapseCamera::PostInitProperties() {
	Super::PostInitProperties();

	if (PreviewTexture) {
		PreviewTexture->OnTextureUpdate.AddDynamic(this, &UFICRuntimeProcessTimelapseCamera::OnTextureUpdate);
	}
}

void UFICRuntimeProcessTimelapseCamera::Start(AFICRuntimeProcessorCharacter* InCharacter) {
	CaptureCamera = GetWorld()->SpawnActor<AFICCaptureCamera>();
	CameraArgument.InitalizeCaptureCamera(CaptureCamera);
	Time = 0.0f;
	
	FString FSP;
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	if (FSP.IsEmpty()) {
		FSP = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/") TEXT("SaveGames/"));
	}
	FSP = FPaths::Combine(FSP, TEXT("FicsItCam/"), CameraArgument.GetSimpleName());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*FSP)) PlatformFile.CreateDirectoryTree(*FSP);

	Exporter = MakeShared<FSequenceImageExporter>(FSP, FIntPoint(CaptureCamera->RenderTarget->SizeX, CaptureCamera->RenderTarget->SizeY));
	Exporter->Init();
}

void UFICRuntimeProcessTimelapseCamera::Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) {
	Time += DeltaSeconds;
	if (Time < SecondsPerFrame) return;
	Time -= SecondsPerFrame;
	
	AFGPlayerController* Controller = Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController());
	//AFGCharacterPlayer* Character = Cast<AFGCharacterPlayer>(Controller->GetCharacter());
	//if (Character) Character->SetThirdPersonMode();
	// TODO: Fix character and creature animation in capture

	CameraArgument.UpdateCameraSettings(CaptureCamera);
	CaptureCamera->UpdateCaptureWithCameraData();
	
	CaptureCamera->CaptureComponent->CaptureScene();

	class FTextureRenderTargetResource* resource = CaptureCamera->RenderTarget->GameThread_GetRenderTargetResource();
	AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(Exporter.ToSharedRef(), MakeShared<FFICRenderTarget_Raw>(resource));

	//if (Character) Character->SetFirstPersonMode();

	OnPreviewUpdate.Broadcast();
}

void UFICRuntimeProcessTimelapseCamera::Stop(AFICRuntimeProcessorCharacter* InCharacter) {
	if (PreviewTexture == nullptr) {
		PreviewTexture = NewObject<UFICProceduralTexture>(this);
		PreviewTexture->OnTextureUpdate.AddDynamic(this, &UFICRuntimeProcessTimelapseCamera::OnTextureUpdate);
	}

	TSharedRef<FSequenceExporterProceduralTexture> TextureExporter = MakeShared<FSequenceExporterProceduralTexture>(PreviewTexture);
	AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(TextureExporter, MakeShared<FFICRenderTarget_Raw>(CaptureCamera->RenderTarget->GameThread_GetRenderTargetResource()));
	
	Exporter->Finish();
	CaptureCamera = nullptr;
}

UTexture* UFICRuntimeProcessTimelapseCamera::GetPreviewTexture() {
	if (CaptureCamera) {
		return CaptureCamera->RenderTarget;
	} else if (PreviewTexture) {
		return PreviewTexture->GetTexture();
	} else {
		return nullptr;
	}
}

void UFICRuntimeProcessTimelapseCamera::OnTextureUpdate() {
	OnPreviewUpdate.Broadcast();
}
