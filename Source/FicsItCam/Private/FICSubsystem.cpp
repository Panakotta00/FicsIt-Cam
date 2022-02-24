#include "FICSubsystem.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Command/FICCommand.h"
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

	// Discover Commands
	for (TObjectIterator<UClass> Class; Class; ++Class) {
		if (!Class->IsChildOf<UFICCommand>() || *Class == UFICCommand::StaticClass()) continue;
		UFICCommand* CMD = Class->GetDefaultObject<UFICCommand>();
		Commands.FindOrAdd(CMD->ParentCommand).Add(CMD->CommandName, CMD);
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

	for (UFICRuntimeProcess* RuntimeProcess : ActiveRuntimeProcesses) {
		RuntimeProcess->Tick(RuntimeProcess->NeedsRuntimeProcessCharacter() ? GetRuntimeProcessorCharacter() : nullptr, DeltaSeconds);
	}
}

void AFICSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
}

void AFICSubsystem::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	TSet<UFICRuntimeProcess*> RunningProcesses = ActiveRuntimeProcesses;
	for (UFICRuntimeProcess* Process : RunningProcesses) {
		StopRuntimeProcess(Process);
	}

	TMap<FString, UFICRuntimeProcess*> Processes = RuntimeProcesses;
	for (const TPair<FString, UFICRuntimeProcess*>& Process : Processes) {
		if (Process.Value->IsPersistent()) {
			Process.Value->PreSave();
		} else {
			RemoveRuntimeProcess(Process.Value);
		}
	}
}

bool AFICSubsystem::ShouldSave_Implementation() const {
	return true;
}

void AFICSubsystem::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	ActiveRuntimeProcesses.Remove(nullptr);
	TMap<FString, UFICRuntimeProcess*> Processes = RuntimeProcesses;
	for (const TPair<FString, UFICRuntimeProcess*>& Process : Processes) {
		if (Process.Value) {
			Process.Value->PostLoad();
		} else {
			RuntimeProcesses.Remove(Process.Key);
		}
	}
	
	TSet<UFICRuntimeProcess*> RunningProcesses = ActiveRuntimeProcesses;
	ActiveRuntimeProcesses.Empty();
	for (UFICRuntimeProcess* Process : RunningProcesses) {
		StartRuntimeProcess(Process);
	}
}

bool AFICSubsystem::CreateRuntimeProcess(FString Key, UFICRuntimeProcess* InProcess, bool bStartAutomatically) {
	RemoveRuntimeProcess(InProcess);
	RuntimeProcesses.Add(Key, InProcess);
	InProcess->Initialize();
	if (bStartAutomatically) {
		if (!StartRuntimeProcess(InProcess)) {
			RemoveRuntimeProcess(InProcess);
			return false;
		}
	}
	return true;
}

bool AFICSubsystem::RemoveRuntimeProcess(UFICRuntimeProcess* Process) {
	if (!Process) return false;

	StopRuntimeProcess(Process);

	ActiveRuntimeProcesses.Remove(Process);
	Process->Shutdown();
	
	RuntimeProcesses.Remove(FindRuntimeProcessKey(Process));
	
	return true;
}

bool AFICSubsystem::StartRuntimeProcess(UFICRuntimeProcess* Process) {
	if (!Process) return false;

	AFICRuntimeProcessorCharacter* Character = nullptr;
	if (Process->NeedsRuntimeProcessCharacter()) {
		Character = GetRuntimeProcessorCharacter();
		if (Character) return false;
		CreateRuntimeProcessorCharacter(Process);
		Character = GetRuntimeProcessorCharacter();
	}
	
	ActiveRuntimeProcesses.Add(Process);
	
	Process->Start(Character);

	return true;
}

bool AFICSubsystem::StopRuntimeProcess(UFICRuntimeProcess* Process) {
	if (!Process) return false;

	if (!ActiveRuntimeProcesses.Contains(Process)) return false;
	
	AFICRuntimeProcessorCharacter* Character = nullptr;
	if (Process->NeedsRuntimeProcessCharacter()) {
		Character = RuntimeProcessorCharacter;
		RuntimeProcessorCharacter = nullptr;
	}

	Process->Stop(Character);
	ActiveRuntimeProcesses.Remove(Process);
	
	if (Character) DestoryRuntimeProcessorCharacter(Character);
	
	return true;
}

void AFICSubsystem::CreateRuntimeProcessorCharacter(UFICRuntimeProcess* RuntimeProcess) {
	OriginalPlayerCharacter = GetWorld()->GetFirstPlayerController()->GetCharacter();
	RuntimeProcessorCharacter = GetWorld()->SpawnActor<AFICRuntimeProcessorCharacter>();
	GetWorld()->GetFirstPlayerController()->Possess(RuntimeProcessorCharacter);
	RuntimeProcessorCharacter->Initialize(RuntimeProcess);
}

void AFICSubsystem::DestoryRuntimeProcessorCharacter(AFICRuntimeProcessorCharacter* Character) {
	Character->Shutdown();
	GetWorld()->GetFirstPlayerController()->Possess(OriginalPlayerCharacter);
	Character->Destroy();
	OriginalPlayerCharacter = nullptr;
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

UFICRuntimeProcess* AFICSubsystem::FindRuntimeProcess(const FString& InKey) {
	for (const TPair<FString, UFICRuntimeProcess*>& Process : GetRuntimeProcesses()) {
		if (Process.Key == InKey) return Process.Value;
	}
	return nullptr;
}

FString AFICSubsystem::FindRuntimeProcessKey(UFICRuntimeProcess* InProcess) {
	for (const TPair<FString, UFICRuntimeProcess*> RuntimeProcess : RuntimeProcesses) {
		if (RuntimeProcess.Value == InProcess) {
			return RuntimeProcess.Key;
		}
	}
	return TEXT("");
}
