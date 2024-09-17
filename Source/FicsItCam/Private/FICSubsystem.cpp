#include "FICSubsystem.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "FGGameUserSettings.h"
#include "FGInputSettings.h"
#include "FICUtils.h"
#include "RHIGPUReadback.h"
#include "RHISurfaceDataConversion.h"
#include "Command/FICCommand.h"
#include "Editor/FICEditorContext.h"
#include "Editor/FICEditorSubsystem.h"
#include "Engine/World.h"
#include "Runtime/FICRuntimeProcessorCharacter.h"
#include "Runtime/Process/FICRuntimeProcess.h"
#include "Runtime/Process/FICRuntimeProcessPlayScene.h"
#include "Util/SequenceExporter.h"

AFICSubsystem* AFICSubsystem::GetFICSubsystem(UObject* WorldContext) {
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFICSubsystem>();
}

AFICSubsystem::AFICSubsystem() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	InputAction_OpenMenu = ConstructorHelpers::FObjectFinder<UInputAction>(L"/FicsItCam/Input/IA_FIC_OpenMenu.IA_FIC_OpenMenu").Object;
}

void AFICSubsystem::FinishDestroy() {
	Super::FinishDestroy();
}

void AFICSubsystem::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) {
	AFICSubsystem* This = CastChecked<AFICSubsystem>(InThis);
}

void AFICSubsystem::BeginPlay() {
	Super::BeginPlay();

	EnableInput(GetWorld()->GetFirstPlayerController());
	
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	const UFGInputSettings* Settings = UFGInputSettings::Get();
	EnhancedInputComponent->BindAction(InputAction_OpenMenu, ETriggerEvent::Triggered, this, &AFICSubsystem::OpenMenu);
	
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

	// Add Scenes to Scene List
	for (TActorIterator<AFICScene> Scene(GetWorld()); Scene; ++Scene) {
		Scenes.Add(*Scene);
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
				HandleRenderRequest(NextRequest);
				RenderRequestQueue.Pop();
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
			if (!Process.Value->PreSaveProcess()) {
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
			Process.Value->PostLoadProcess();
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
	OnRuntimeProcessCreated.Broadcast(Key, InProcess);
	return true;
}

bool AFICSubsystem::RemoveRuntimeProcess(UFICRuntimeProcess* Process) {
	if (!Process) return false;

	StopRuntimeProcess(Process);

	ActiveRuntimeProcesses.Remove(Process);
	Process->Shutdown();

	FString Key = FindRuntimeProcessKey(Process);
	RuntimeProcesses.Remove(Key);

	OnRuntimeProcessDeleted.Broadcast(Key, Process);
	
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

	FString Key = FindRuntimeProcessKey(Process);
	OnRuntimeProcessStarted.Broadcast(Key, Process);

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

	FString Key = FindRuntimeProcessKey(Process);
	OnRuntimeProcessStopped.Broadcast(Key, Process);
	
	return true;
}

bool AFICSubsystem::IsRuntimeProcessActive(UFICRuntimeProcess* Process) {
	return ActiveRuntimeProcesses.Contains(Process);
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

void AFICSubsystem::ExportRenderTarget(TSharedRef<FSequenceExporter> Exporter, TSharedRef<FFICRenderTarget> RenderTarget, bool bInstant) {
	TSharedRef<FFICRenderRequest> RenderRequest = MakeShared<FFICRenderRequest>(RenderTarget, Exporter, FRHIGPUTextureReadback(TEXT("FICSubsystem Texture Readback")));

	ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)([this, RenderTarget, RenderRequest](FRHICommandListImmediate& RHICmdList){
		FTextureRHIRef Target = RenderTarget->GetRenderTarget();
		RenderRequest->Readback.EnqueueCopy(RHICmdList, Target);
	});

	if (bInstant) {
		FlushRenderingCommands();
		RenderRequest->RenderFence.BeginFence(true);
		RenderRequest->RenderFence.Wait(true);
		HandleRenderRequest(RenderRequest);
	} else {
		RenderRequestQueue.Enqueue(RenderRequest);
		RenderRequest->RenderFence.BeginFence(true);
	}
}

void AFICSubsystem::HandleRenderRequest(TSharedPtr<FFICRenderRequest> InRequest) {
	FTextureRHIRef Target = InRequest->RenderTarget->GetRenderTarget();
	FIntPoint Size = Target->GetSizeXY();
	FIntPoint ReadSize;
	ENQUEUE_RENDER_COMMAND(ReadbackFICCameraFootage)( [&](FRHICommandListImmediate& RHICmdList) {
		void* data = InRequest->Readback.Lock(ReadSize.X, &ReadSize.Y);
		if (!data) return;
		if (Target->GetFormat() == PF_A2B10G10R10) {
			FColor* ptrLinearColor = (FColor*)FMemory::Malloc(ReadSize.X * ReadSize.Y * sizeof(FColor));
			ConvertRawB10G10R10A2DataToFColor(ReadSize.X, ReadSize.Y, (uint8*)data, ReadSize.X * 4, ptrLinearColor);
			//ConvertFLinearColorsToFColorSRGB(ptrLinearColor, (FColor*)data, ReadSize.X + ReadSize.Y);
			InRequest->Exporter->AddFrame(PF_R8G8B8A8, ptrLinearColor, ReadSize, Size);
			FMemory::Free(ptrLinearColor);
		} else {
			InRequest->Exporter->AddFrame(PF_R8G8B8A8, data, ReadSize, Size);
	}
	});
	FlushRenderingCommands();
}

TSet<AFICScene*> AFICSubsystem::GetScenes() const {
	return Scenes;
}

AFICScene* AFICSubsystem::FindSceneByName(const FString& InSceneName) {
	for (TActorIterator<AFICScene> Scene(GetWorld()); Scene; ++Scene) {
		if (Scene->SceneName == InSceneName) return *Scene;
	}
	return nullptr;
}

AFICScene* AFICSubsystem::CreateScene(const FString& Name) {
	if (!UFICUtils::IsValidFICObjectName(Name)) return nullptr;
	if (FindSceneByName(Name)) return nullptr;
	
	AFICScene* Scene = GetWorld()->SpawnActor<AFICScene>();
		
	FIntPoint Resolution = UFGGameUserSettings::GetFGGameUserSettings()->GetScreenResolution();
	Scene->ResolutionWidth = Resolution.X + (Resolution.X % 2 != 0 ? 1 : 0);
	Scene->ResolutionHeight = Resolution.Y + (Resolution.Y % 2 != 0 ? 1 : 0);
	Scene->SceneName = Name;
	UFICCamera* CDO = UFICCamera::StaticClass()->GetDefaultObject<UFICCamera>();
	if (CDO) Scene->AddSceneObject(CDO->CreateNewObject(this, Scene));
	
	Scenes.Add(Scene);
	OnSceneCreated.Broadcast(Scene);

	return Scene;
}

void AFICSubsystem::DeleteScene(AFICScene* Scene) {
	UFICRuntimeProcess* Process = FindRuntimeProcess(AFICScene::GetSceneProcessKey(Scene->SceneName));
	if (Process) RemoveRuntimeProcess(Process);
	
	Scenes.Remove(Scene);
	OnSceneDeleted.Broadcast(Scene);

	Scene->Destroy();
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

void AFICSubsystem::FilterAndSortRuntimeProcesses(const TArray<UFICRuntimeProcess*>& InRuntimeProcesses, TArray<UFICRuntimeProcess*>& OutActive, TArray<UFICRuntimeProcess*>& OutInactive) {
	for (UFICRuntimeProcess* Process : InRuntimeProcesses) {
		if (IsRuntimeProcessActive(Process)) {
			OutActive.Add(Process);
		} else {
			OutInactive.Add(Process);
		}
	}
	auto Predicate = [this](UFICRuntimeProcess* P1, UFICRuntimeProcess* P2) {
		FString K1 = FindRuntimeProcessKey(P1);
		FString K2 = FindRuntimeProcessKey(P2);
		return K1 < K2;
	};
	Algo::Sort(OutActive, Predicate);
	Algo::Sort(OutInactive, Predicate);
}

void AFICSubsystem::FilterAndSortScenes(const TArray<AFICScene*>& InScenes, TArray<AFICScene*>& OutActive, TArray<AFICScene*>& OutPaused, TArray<AFICScene*>& OutInactive) {
	OutInactive = InScenes;
	for (const TPair<FString, UFICRuntimeProcess*>& Process : GetRuntimeProcesses()) {
		UFICRuntimeProcessPlayScene* PlaySceneProcess = Cast<UFICRuntimeProcessPlayScene>(Process.Value);
		if (!PlaySceneProcess) continue;
		AFICScene* Scene = PlaySceneProcess->Scene;
		OutInactive.Remove(Scene);
		if (IsRuntimeProcessActive(Process.Value)) {
			OutActive.Add(Scene);
		} else {
			OutPaused.Add(Scene);
		}
	}
	auto Predicate = [this](AFICScene* P1, AFICScene* P2) {
		return P1->SceneName < P2->SceneName;
	};
	Algo::Sort(OutActive, Predicate);
	Algo::Sort(OutPaused, Predicate);
	Algo::Sort(OutInactive, Predicate);
}

void AFICSubsystem::OpenMenu() {
	TSubclassOf<UFGInteractWidget> Widget = LoadObject<UClass>(nullptr, TEXT("/FicsItCam/UI/Widget_FIC_Menu.Widget_FIC_Menu_C"));
	Cast<AFGHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())->OpenInteractUI(Widget, nullptr);
}
