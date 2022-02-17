#include "Editor/FICEditorSubsystem.h"

#include "FGInputLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Editor/FICEditorContext.h"
#include "Engine/GameEngine.h"
#include "Slate/SGameLayerManager.h"

AFICEditorSubsystem::AFICEditorSubsystem() {
	PrimaryActorTick.bCanEverTick = true;
}

void AFICEditorSubsystem::BeginPlay() {
	Super::BeginPlay();
}

void AFICEditorSubsystem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

}

void AFICEditorSubsystem::OpenEditor(AFICScene* InScene) {
	UFICEditorContext* Context = GetActiveEditorContext();
	if (Context) CloseEditor();

	APlayerController* Controller = GetWorld()->GetFirstPlayerController();

	// Save Original Player Character
	OriginalPlayerCharacter = Controller->GetCharacter();
	
	// Create Editor Player character
	AFICEditorCameraCharacter* Character = GetWorld()->SpawnActor<AFICEditorCameraCharacter>(InScene->GetActorLocation(), InScene->GetActorRotation());
	
	// Create & load new Editor Context
	Context = NewObject<UFICEditorContext>(GetWorld());
	Context->Load(Character, InScene);
	
	// Initialize Editor Player Character
	// TODO: Persist "Viewport Camera Transform" sepperately in persistent editor storage for given scene
	Controller->Possess(Character);
	UFGInputLibrary::UpdateInputMappings(Controller);
	Character->SetEditorContext(Context);

	// Get widgets to inject editor UI into and store necessery recovery data
	GEngine->GameViewport->GetGameViewportWidget()->SetRenderDirectlyToWindow(false);
	GEngine->GameViewport->GetGameLayerManager()->SetSceneViewport(nullptr);
	Cast<UGameEngine>(GEngine)->CleanupGameViewport();
	Cast<UGameEngine>(GEngine)->CreateGameViewport(GEngine->GameViewport);
	
	GameViewport = FSlateApplication::Get().GetGameViewport();
	GameViewportContainer = StaticCastSharedPtr<SVerticalBox>(GameViewport->GetParentWidget());
	GameOverlay = StaticCastSharedPtr<SOverlay>(GameViewportContainer->GetParentWidget());
	
	check(GameOverlay->RemoveSlot(GameViewportContainer.ToSharedRef()) == true);

	// Create & Inject Editor Widget
	EditorWidget = SNew(SFICEditor, Context, GameViewportContainer, GameViewport);
	GameOverlay->AddSlot()[
		EditorWidget.ToSharedRef()
	];

	// Finish Editor Opening
	ActiveEditorContext = Context;
	EditorPlayerCharacter = Character;
}

void AFICEditorSubsystem::CloseEditor() {
	UFICEditorContext* Context = GetActiveEditorContext();
	AFICEditorCameraCharacter* Character = GetEditorPlayerCharacter();
	ActiveEditorContext = nullptr;
	EditorPlayerCharacter = nullptr;
	
	// Don't do anything if no scene is opened in editor
	if (!Context) return;
	
	AFICScene* Scene = Context->GetScene();

	// Unload loaded Editor Context
	Context->Unload();
	
	// Swap PlayerCharacter to FG Player Character 
	GetWorld()->GetFirstPlayerController()->Possess(OriginalPlayerCharacter);
	UFGInputLibrary::UpdateInputMappings(GetWorld()->GetFirstPlayerController());
	
	// Remove Editor UI
	GameOverlay->RemoveSlot(EditorWidget.ToSharedRef());
	GameOverlay->AddSlot()[
		GameViewportContainer.ToSharedRef()
	];
	GEngine->GameViewport->GetGameViewportWidget()->SetRenderDirectlyToWindow(true);
	GEngine->GameViewport->GetGameLayerManager()->SetSceneViewport(nullptr);
	Cast<UGameEngine>(GEngine)->CleanupGameViewport();
	Cast<UGameEngine>(GEngine)->CreateGameViewport(GEngine->GameViewport);

	// Enabled Game Inputs/WorldControl
	APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(Controller);
	UGameplayStatics::SetGamePaused(this, false);

	// Cleanup Editor Objects
	EditorWidget = nullptr;
	Character->Destroy();
}
