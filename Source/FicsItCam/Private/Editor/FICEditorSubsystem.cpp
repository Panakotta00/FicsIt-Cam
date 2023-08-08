#include "Editor/FICEditorSubsystem.h"

#include "ContextObjectStore.h"
#include "EnhancedInputSubsystems.h"
#include "FGGameUserSettings.h"
#include "BaseGizmos/GizmoViewContext.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Editor/FICEditorContext.h"
#include "Editor/ITF/FICGrabTool.h"
#include "Editor/ITF/FICSelectionInteraction.h"
#include "Editor/ITF/FICToolsContextQueries.h"
#include "Editor/ITF/FICToolsContextRender.h"
#include "Editor/ITF/FICTransformInteraction.h"
#include "Engine/GameEngine.h"
#include "Input/FGInputMappingContext.h"
#include "Slate/SceneViewport.h"
#include "Slate/SGameLayerManager.h"

void AFICEditorSubsystem::InitInteractiveTools() {
	ToolsContext = NewObject<UInteractiveToolsContext>();

	PDIRenderActor = GetWorld()->SpawnActor<AActor>();
	PDIRenderComponent = NewObject<UFICToolsContextRenderComponent>(PDIRenderActor);
	PDIRenderActor->SetRootComponent(PDIRenderComponent);
	PDIRenderComponent->RegisterComponent();
	LineBatchComponent = NewObject<ULineBatchComponent>(PDIRenderActor);
	LineBatchComponent->AttachToComponent(PDIRenderActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

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

	// register tools
	ToolsContext->ToolManager->RegisterToolType("Grab", NewObject<UFICGrabToolBuilder>(ToolsContext->ToolManager));

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

	//FString nice = TestLibWrapper_foo();
	//UE_LOG(LogTemp, Warning, TEXT("Nice!"));
}

void AFICEditorSubsystem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (ActiveEditorContext) {
		//double start = FPlatformTime::Seconds();
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

			InputState.Mouse.Delta2D = ViewportMousePos - PrevMousePosition;
			PrevMousePosition = InputState.Mouse.Position2D = ViewportMousePos;
			InputState.Mouse.WorldRay = FRay(Origin, Direction);

			ToolsContext->ToolManager->GetContextObjectStore()->FindContext<UGizmoViewContext>()->ResetFromSceneView(*SceneView);

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
				//GetWorld()->LineBatcher->DrawLine(InputState.Mouse.WorldRay.Origin, InputState.Mouse.WorldRay.Origin + InputState.Mouse.WorldRay.Direction * 1000, FColor::Red, 0, 10, 10000);
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
			FRuntimeToolsFrameworkRenderImpl RenderAPI(LineBatchComponent, SceneView, CurrentViewCameraState);
			ToolsContext->ToolManager->Render(&RenderAPI);
			ToolsContext->GizmoManager->Render(&RenderAPI);

			// force rendering flush so that PDI lines get drawn
			FlushRenderingCommands();
		}
		//double end = FPlatformTime::Seconds();
		//UE_LOG(LogTemp, Warning, TEXT("code executed in %f seconds."), end-start);
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
	Context->SetAutoKeyframe(bAutoKeyframe);
	Context->SetLockCameraToView(bLockCameraToView);
	Context->SetCameraPreview(bCameraPreview);
	Context->bShowPath = bShowCameraPath;
	Context->bForceResolution = bForceResolution;
	
	// Initialize Editor Player Character
	// TODO: Persist "Viewport Camera Transform" sepperately in persistent editor storage for given scene
	Controller->Possess(Character);
	Character->SetEditorContext(Context);
	Cast<AFGPlayerController>(Controller)->GetHUD<AFGHUD>()->SetHUDVisibility(false);
	Cast<AFGPlayerController>(Controller)->GetHUD<AFGHUD>()->SetHiddenHUDMode(true);
	
	// Get widgets to inject editor UI into and store necessery recovery data
	PrevResolution = GSystemResolution;
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

	// Add Editor Input Context
	UFGInputMappingContext* InputMappingContext = LoadObject<UFGInputMappingContext>(nullptr, TEXT("/FicsItCam/Input/IC_FIC_Editor.IC_FIC_Editor"));
	Controller->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->AddMappingContext(InputMappingContext, -2);
		
	InitInteractiveTools();
}

void AFICEditorSubsystem::CloseEditor() {
	UFICEditorContext* Context = GetActiveEditorContext();
	AFICEditorCameraCharacter* Character = GetEditorPlayerCharacter();
	ActiveEditorContext = nullptr;
	EditorPlayerCharacter = nullptr;
	
	// Don't do anything if no scene is opened in editor
	if (!Context) return;

	APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	
	// Copy Editor Settings
	bAutoKeyframe = Context->GetAutoKeyframe();
	bLockCameraToView = Context->GetLockCameraToView();
	bCameraPreview = Context->GetCameraPreview();
	bShowCameraPath = Context->bShowPath;
	bForceResolution = Context->bForceResolution;
	
	AFICScene* Scene = Context->GetScene();

	// Shutdown Interactive Tools
	ShutdownInteractiveTools();

	// Remove Editor Input Context
	UFGInputMappingContext* InputMappingContext = LoadObject<UFGInputMappingContext>(nullptr, TEXT("/FicsItCam/Input/IC_FIC_Editor.IC_FIC_Editor"));
	Controller->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->RemoveMappingContext(InputMappingContext);

	// Focus back to viewport
	Character->SetControlView(true);
	
	// Remove Editor UI
	GameOverlay->RemoveSlot(EditorWidget.ToSharedRef());
	
	// Unload loaded Editor Context
	Context->Unload();
	
	// Swap PlayerCharacter to FG Player Character 
	GetWorld()->GetFirstPlayerController()->Possess(OriginalPlayerCharacter);

	// Add Game Viewport Back
	GameOverlay->AddSlot()[
		GameViewportContainer.ToSharedRef()
	];
	GEngine->GameViewport->GetGameViewportWidget()->SetRenderDirectlyToWindow(true);
	GEngine->GameViewport->GetGameLayerManager()->SetSceneViewport(nullptr);
	Cast<UGameEngine>(GEngine)->CleanupGameViewport();
	GSystemResolution = PrevResolution;
	Cast<UGameEngine>(GEngine)->CreateGameViewport(GEngine->GameViewport);

	// Enabled Game Inputs/WorldControl
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(Controller);
	UGameplayStatics::SetGamePaused(this, false);
	Cast<AFGPlayerController>(Controller)->GetHUD<AFGHUD>()->SetHUDVisibility(true);
	Cast<AFGPlayerController>(Controller)->GetHUD<AFGHUD>()->SetHiddenHUDMode(false);
	
	// Cleanup Editor Objects
	EditorWidget = nullptr;
	Character->Destroy();
}
