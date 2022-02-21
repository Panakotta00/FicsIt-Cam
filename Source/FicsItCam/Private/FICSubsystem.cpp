#include "FICSubsystem.h"

#include <Subsystem/SubsystemActorManager.h>

#include "FICCommand.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Editor/FICEditorContext.h"
#include "Editor/FICEditorSubsystem.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Runtime/FICRuntimeProcessorCharacter.h"
#include "Runtime/FICTimelapseCamera.h"
#include "Runtime/Process/FICRuntimeProcess.h"

FFICAsyncImageCompressAndSave::FFICAsyncImageCompressAndSave(TSharedPtr<IImageWrapper> Image, FString Path) : Image(Image), Path(Path) {}

FFICAsyncImageCompressAndSave::~FFICAsyncImageCompressAndSave() {}

void FFICAsyncImageCompressAndSave::DoWork() {
	double start = FPlatformTime::Seconds();
	TArray64<uint8> CompressedData = Image->GetCompressed(100);
	UE_LOG(LogTemp, Warning, TEXT("Compress in %f seconds."), FPlatformTime::Seconds()-start);
	start = FPlatformTime::Seconds();
	FFileHelper::SaveArrayToFile(CompressedData, *Path);
	UE_LOG(LogTemp, Warning, TEXT("Save in %f seconds."), FPlatformTime::Seconds()-start);
}

AFICSubsystem* AFICSubsystem::GetFICSubsystem(UObject* WorldContext) {
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFICSubsystem>();
}

AFICSubsystem::AFICSubsystem() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFICSubsystem::BeginPlay() {
	Super::BeginPlay();

	// Convert deprecated AFICAnimation Actors to Scene Actors
	for (TActorIterator<AFICAnimation> Animation(GetWorld()); Animation; ++Animation) {
		Animation->CreateScene();
		Animation->Destroy();
	}

	for (TTuple<FString, AFICTimelapseCamera*> Cam : TimelapseCameras) {
		Cam.Value->Name = Cam.Key;
	}
}

void AFICSubsystem::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (!RenderRequestQueue.IsEmpty()) {
		TSharedPtr<FFICRenderRequest> NextRequest = *RenderRequestQueue.Peek();
		if (NextRequest) {
			if (NextRequest->RenderFence.IsFenceComplete()) {
				double start = FPlatformTime::Seconds();
				IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
				TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	
				bool bRaw = ImageWrapper->SetRaw(NextRequest->Image.GetData(), NextRequest->Image.GetTypeSize() * NextRequest->Image.Num(), NextRequest->RenderTarget->SizeX, NextRequest->RenderTarget->SizeY, ERGBFormat::BGRA, 8);
				if (!bRaw) return;
				RenderRequestQueue.Pop();
	
				(new FAutoDeleteAsyncTask<FFICAsyncImageCompressAndSave>(ImageWrapper, NextRequest->Path))->StartBackgroundTask();
			}
		}
	}
}

void AFICSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
}

bool AFICSubsystem::ShouldSave_Implementation() const {
	return true;
}

void AFICSubsystem::StartProcess(UFICRuntimeProcess* InProcess) {
	StopProcess();
	ActiveRuntimeProcess = InProcess;
	OriginalPlayerCharacter = GetWorld()->GetFirstPlayerController()->GetCharacter();
	RuntimeProcessorCharacter = GetWorld()->SpawnActor<AFICRuntimeProcessorCharacter>();
	GetWorld()->GetFirstPlayerController()->Possess(RuntimeProcessorCharacter);
	RuntimeProcessorCharacter->Initialize(ActiveRuntimeProcess);
	ActiveRuntimeProcess->Initialize(RuntimeProcessorCharacter);
}

void AFICSubsystem::StopProcess() {
	if (!ActiveRuntimeProcess) return;

	AFICRuntimeProcessorCharacter* Character = RuntimeProcessorCharacter;
	RuntimeProcessorCharacter = nullptr;

	ActiveRuntimeProcess->Shutdown(Character);
	Character->Shutdown();
	GetWorld()->GetFirstPlayerController()->Possess(OriginalPlayerCharacter);
	Character->Destroy();
	
	OriginalPlayerCharacter = nullptr;
	ActiveRuntimeProcess = nullptr;
}

void AFICSubsystem::SaveRenderTargetAsJPG(const FString& FilePath, UTextureRenderTarget2D* RenderTarget) {
	double start = FPlatformTime::Seconds();
	
	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	struct FReadSurfaceContext{
		FRenderTarget* SrcRenderTarget;
		TArray<FColor>* OutData;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
	};

	TSharedRef<FFICRenderRequest> RenderRequest = MakeShared<FFICRenderRequest>();
	RenderRequest->Path = FilePath;
	RenderRequest->RenderTarget = RenderTarget;
	
	FReadSurfaceContext ReadSurfaceContext = {
		RenderTargetResource,
		&RenderRequest->Image,
		FIntRect(0,0,RenderTargetResource->GetSizeXY().X, RenderTargetResource->GetSizeXY().Y),
		FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX)
	};
	
	ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
		[ReadSurfaceContext](FRHICommandListImmediate& RHICmdList){
			RHICmdList.ReadSurfaceData(
				ReadSurfaceContext.SrcRenderTarget->GetRenderTargetTexture(),
				ReadSurfaceContext.Rect,
				*ReadSurfaceContext.OutData,
				ReadSurfaceContext.Flags
			);
		});

	RenderRequestQueue.Enqueue(RenderRequest);
	RenderRequest->RenderFence.BeginFence();
}

AFICScene* AFICSubsystem::FindSceneByName(const FString& InSceneName) {
	for (TActorIterator<AFICScene> Scene(GetWorld()); Scene; ++Scene) {
		if (Scene->SceneName == InSceneName) return *Scene;
	}
	return nullptr;
}
