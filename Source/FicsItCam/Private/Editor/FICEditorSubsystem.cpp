#include "Editor/FICEditorSubsystem.h"

#include "FGColoredInstanceMeshProxy.h"
#include "FGInputLibrary.h"
#include "FGProductionIndicatorInstanceComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/ProxyInstancedStaticMeshComponent.h"
#include "Editor/FICEditorContext.h"
#include "Editor/ITF/FICSelectionInteraction.h"
#include "Editor/ITF/FICToolsContextQueries.h"
#include "Editor/ITF/FICToolsContextRender.h"
#include "Editor/ITF/FICTransformInteraction.h"
#include "Engine/GameEngine.h"
#include "Slate/SceneViewport.h"
#include "Slate/SGameLayerManager.h"

void AFICEditorSubsystem::InitInteractiveTools() {
	ToolsContext = NewObject<UInteractiveToolsContext>();

	PDIRenderActor = GetWorld()->SpawnActor<AActor>();
	PDIRenderComponent = NewObject<UFICToolsContextRenderComponent>(PDIRenderActor);
	PDIRenderActor->SetRootComponent(PDIRenderComponent);
	PDIRenderComponent->RegisterComponent();

	ToolsQueries = MakeShared<FFICToolsContextQueries>(ToolsContext, GetWorld());
	ToolsTransactions = MakeShared<FFICToolsContextTransactions>();

	ToolsContext->Initialize(ToolsQueries.Get(), ToolsTransactions.Get());
	
	// register selection interaction
	SelectionInteraction = NewObject<UFICSelectionInteraction>(this);
	SelectionInteraction->Initialize(GetActiveEditorContext());
	ToolsContext->InputRouter->RegisterSource(SelectionInteraction);

	// create transform interaction
	TransformInteraction = NewObject<UFICTransformInteraction>(this);
	TransformInteraction->Initialize(GetActiveEditorContext());
}

void AFICEditorSubsystem::ShutdownInteractiveTools() {
	if (ToolsContext != nullptr) {
		TransformInteraction->Shutdown();
		ToolsContext->Shutdown();
	}

	if (PDIRenderActor) {
		PDIRenderActor->Destroy();
		PDIRenderActor = nullptr;
		PDIRenderComponent = nullptr;
	}

	ToolsContext = nullptr;
	ToolsQueries = nullptr;
	ToolsTransactions = nullptr;

	SelectionInteraction = nullptr;
	TransformInteraction = nullptr;
}

void AFICEditorSubsystem::OnLeftMouseDown() {
	CurrentMouseState.Mouse.Left.SetStates(true, false, false);
	bPendingMouseStateChange = true;
}

void AFICEditorSubsystem::OnLeftMouseUp() {
	CurrentMouseState.Mouse.Left.SetStates(false, false, true);
	bPendingMouseStateChange = true;
}

AFICEditorSubsystem::AFICEditorSubsystem() {
	PrimaryActorTick.bCanEverTick = true;
}

void AFICEditorSubsystem::BeginPlay() {
	Super::BeginPlay();
}

void AFICEditorSubsystem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (ActiveEditorContext) {
		FInputDeviceState InputState = CurrentMouseState;
		InputState.InputDevice = EInputDevices::Mouse;

		FVector2D MousePosition = FSlateApplication::Get().GetCursorPos();
		FVector2D LastMousePosition = FSlateApplication::Get().GetLastCursorPos();
		FModifierKeysState ModifierState = FSlateApplication::Get().GetModifierKeys();

		UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
		TSharedPtr<IGameLayerManager> LayerManager = ViewportClient->GetGameLayerManager();
		FGeometry ViewportGeometry;
		if (ensure(LayerManager.IsValid()))
		{
			ViewportGeometry = LayerManager->GetViewportWidgetHostGeometry();
		}
		// why do we need this scale here? what is it for?
		FVector2D ViewportMousePos = ViewportGeometry.Scale * ViewportGeometry.AbsoluteToLocal(MousePosition);


		// update modifier keys
		InputState.SetModifierKeyStates(
			ModifierState.IsLeftShiftDown(),
			ModifierState.IsAltDown(),
			ModifierState.IsControlDown(),
			ModifierState.IsCommandDown());

		if (ViewportClient) {
			FSceneViewport* Viewport = ViewportClient->GetGameViewport();
		
			FEngineShowFlags* ShowFlags = ViewportClient->GetEngineShowFlags();
			FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
				ViewportClient->Viewport,
				GetWorld()->Scene,
				*ShowFlags)
				.SetRealtimeUpdate(true));

			ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Cast<APlayerController>(EditorPlayerCharacter->GetController())->Player);
			FVector ViewLocation;
			FRotator ViewRotation;
			FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily,  /*out*/ ViewLocation, /*out*/ ViewRotation, LocalPlayer->ViewportClient->Viewport);
			if (SceneView == nullptr)
			{ 
				return;		// abort abort
			}

			CurrentViewCameraState.Position = ViewLocation;
			CurrentViewCameraState.Orientation = ViewRotation.Quaternion();
			CurrentViewCameraState.HorizontalFOVDegrees = SceneView->FOV;
			CurrentViewCameraState.AspectRatio = Viewport->GetDesiredAspectRatio(); //ViewportClient->AspectRatio;
			CurrentViewCameraState.bIsOrthographic = false;
			CurrentViewCameraState.bIsVR = false;
			CurrentViewCameraState.OrthoWorldCoordinateWidth = 1;

			FVector4 ScreenPos = SceneView->PixelToScreen(ViewportMousePos.X, ViewportMousePos.Y, 0);

			const FMatrix InvViewMatrix = SceneView->ViewMatrices.GetInvViewMatrix();
			const FMatrix InvProjMatrix = SceneView->ViewMatrices.GetInvProjectionMatrix();

			const float ScreenX = ScreenPos.X;
			const float ScreenY = ScreenPos.Y;

			FVector Origin;
			FVector Direction;
			if (! ViewportClient->IsOrtho())
			{
				Origin = SceneView->ViewMatrices.GetViewOrigin();
				Direction = InvViewMatrix.TransformVector(FVector(InvProjMatrix.TransformFVector4(FVector4(ScreenX * GNearClippingPlane, ScreenY * GNearClippingPlane, 0.0f, GNearClippingPlane)))).GetSafeNormal();
			}
			else
			{
				Origin = InvViewMatrix.TransformFVector4(InvProjMatrix.TransformFVector4(FVector4(ScreenX, ScreenY, 0.5f, 1.0f)));
				Direction = InvViewMatrix.TransformVector(FVector(0, 0, 1)).GetSafeNormal();
			}

			// fudge factor so we don't hit actor...
			Origin += 1.0 * Direction;

			InputState.Mouse.Position2D = ViewportMousePos;
			InputState.Mouse.Delta2D = CurrentMouseState.Mouse.Position2D - PrevMousePosition;
			PrevMousePosition = InputState.Mouse.Position2D;
			InputState.Mouse.WorldRay = FRay(Origin, Direction);


			// if we are in camera control we don't send any events
			bool bInCameraControl = ActiveEditorContext->IsViewportCameraControl();
			if (bInCameraControl)
			{
				ensure(bPendingMouseStateChange == false);
				ensure(ToolsContext->InputRouter->HasActiveMouseCapture() == false);
				//ToolsContext->InputRouter->PostHoverInputEvent(InputState);
			}
			else if (bPendingMouseStateChange || ToolsContext->InputRouter->HasActiveMouseCapture())
			{
				ToolsContext->InputRouter->PostInputEvent(InputState);
			}
			else
			{
				ToolsContext->InputRouter->PostHoverInputEvent(InputState);
			}

			// clear down or up flags now that we have sent event
			if (bPendingMouseStateChange)
			{
				if (CurrentMouseState.Mouse.Left.bDown)
				{
					CurrentMouseState.Mouse.Left.SetStates(false, true, false);
				}
				else
				{
					CurrentMouseState.Mouse.Left.SetStates(false, false, false);
				}
				bPendingMouseStateChange = false;
			}


			// tick things
			ToolsContext->ToolManager->Tick(DeltaTime);
			ToolsContext->GizmoManager->Tick(DeltaTime);

			// render things
			FRuntimeToolsFrameworkRenderImpl RenderAPI(PDIRenderComponent, SceneView, CurrentViewCameraState);
			ToolsContext->ToolManager->Render(&RenderAPI);
			ToolsContext->GizmoManager->Render(&RenderAPI);

			// force rendering flush so that PDI lines get drawn
			FlushRenderingCommands();
		}
	}
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

	InitInteractiveTools();
}

void AFICEditorSubsystem::CloseEditor() {
	UFICEditorContext* Context = GetActiveEditorContext();
	AFICEditorCameraCharacter* Character = GetEditorPlayerCharacter();
	ActiveEditorContext = nullptr;
	EditorPlayerCharacter = nullptr;
	
	// Don't do anything if no scene is opened in editor
	if (!Context) return;
	
	AFICScene* Scene = Context->GetScene();

	// Shutdown Interactive Tools
	ShutdownInteractiveTools();
	
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

void AFICEditorSubsystem::SetMeshInstanced(UMeshComponent* MeshComp, bool Instanced) {
	auto StaticMeshProxy = Cast<UProxyInstancedStaticMeshComponent>(MeshComp);
	if (StaticMeshProxy) {
		StaticMeshProxy->SetInstanced(Instanced);
        
	} else {
		auto ColoredMeshProxy = Cast<UFGColoredInstanceMeshProxy>(MeshComp);
		if (ColoredMeshProxy) {
			ColoredMeshProxy->mBlockInstancing = !Instanced;
			ColoredMeshProxy->SetInstanced(Instanced);
		} else {
			auto ProdIndInst = Cast<UFGProductionIndicatorInstanceComponent>(MeshComp);
			if (ProdIndInst) {
				ProdIndInst->SetInstanced(Instanced);
			}
		}
	}
}
