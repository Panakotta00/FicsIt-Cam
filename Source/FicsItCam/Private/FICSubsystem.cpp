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
#include "Runtime/Process/FICRuntimeProcessTimelapseCamera.h"

FFICAsyncImageCompressAndSave::FFICAsyncImageCompressAndSave(TSharedPtr<IImageWrapper> Image, FString Path) : Image(Image), Path(Path) {}

FFICAsyncImageCompressAndSave::~FFICAsyncImageCompressAndSave() {}

void FFICAsyncImageCompressAndSave::DoWork() {
	TArray64<uint8> CompressedData = Image->GetCompressed(100);
	FFileHelper::SaveArrayToFile(CompressedData, *Path);
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

	// Resume Persisted Runtime Processes
	ActiveRuntimeProcesses.Empty();
	for (UFICRuntimeProcess* Process : PersistentActiveRuntimeProcesses) {
		StartRuntimeProcess(Process);
	}

	// Convert deprecated AFICAnimation Actors to Scene Actors
	for (TActorIterator<AFICAnimation> Animation(GetWorld()); Animation; ++Animation) {
		Animation->CreateScene();
		Animation->Destroy();
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
			if (NextRequest->RenderFence.IsFenceComplete() && NextRequest->Readback.IsReady()) {
				IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
				TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

				FRenderTarget* Target = NextRequest->RenderTarget->GetRenderTarget();

				FIntPoint Size = NextRequest->RenderTarget->GetRenderTarget()->GetSizeXY();
				int64 RawSize = Size.X * Size.Y * sizeof(FColor);
				bool bRaw = ImageWrapper->SetRaw(NextRequest->Readback.Lock(RawSize), RawSize, Target->GetSizeXY().X, Target->GetSizeXY().Y, ERGBFormat::RGBA, 8);
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

	TSet<UFICRuntimeProcess*> RunningProcesses = ActiveRuntimeProcesses;
	for (UFICRuntimeProcess* Process : RunningProcesses) {
		StopRuntimeProcess(Process);
	}
}

void AFICSubsystem::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	PersistentActiveRuntimeProcesses = ActiveRuntimeProcesses;
	for (const TPair<FString, UFICRuntimeProcess*>& Process : RuntimeProcesses) {
		if (Process.Value->IsPersistent()) {
			if (!Process.Value->PreSave()) {
				PersistentActiveRuntimeProcesses.Remove(Process.Value);
			}
		}
	}
}

bool AFICSubsystem::ShouldSave_Implementation() const {
	return true;
}

void AFICSubsystem::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	PersistentActiveRuntimeProcesses.Remove(nullptr);
	TMap<FString, UFICRuntimeProcess*> Processes = RuntimeProcesses;
	for (const TPair<FString, UFICRuntimeProcess*>& Process : Processes) {
		if (Process.Value) {
			Process.Value->PostLoad();
		} else {
			RuntimeProcesses.Remove(Process.Key);
		}
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

void AFICSubsystem::SaveRenderTargetAsJPG(const FString& FilePath, TSharedRef<FFICRenderTarget> RenderTarget) {
	TSharedRef<FFICRenderRequest> RenderRequest = MakeShared<FFICRenderRequest>(RenderTarget, FilePath, FRHIGPUTextureReadback(TEXT("FICSubsystem Texture Readback")));
		
	ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)([this, RenderTarget, RenderRequest](FRHICommandListImmediate& RHICmdList){
		FTexture2DRHIRef Target = RenderTarget->GetRenderTarget()->GetRenderTargetTexture();
		RenderRequest->Readback.EnqueueCopy(RHICmdList, Target);
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
