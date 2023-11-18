#include "Util/FICSceneViewExtension.h"

#include "EngineModule.h"
#include "FICSubsystem.h"
#include "GameFramework/Character.h"
#include "Renderer/Private/ScenePrivate.h"

FFICSceneViewExtension::FFICSceneViewExtension(const FAutoRegister& Register, ULocalPlayer* Player) : Player(Player), FSceneViewExtensionBase(Register) {
	Exporter = MakeShared<FSequenceImageExporter>(TEXT("C:/Users/Yannic/Desktop"), FIntPoint(1000, 1000));
	Exporter->Init();
}

void FFICSceneViewExtension::SetupViewFamily(FSceneViewFamily& InViewFamily) {
	FVector Location = Player->GetWorld()->GetFirstPlayerController()->GetCharacter()->GetActorLocation();
	Location += FVector(0, 0, 500);
	FRotator Rotation = FVector(0, 0, -1).Rotation();

	FSceneViewInitOptions ViewInitOptions;
	ViewInitOptions.ViewLocation = Location;
	ViewInitOptions.ViewRotation = Rotation;
	ViewInitOptions.ViewFamily = &InViewFamily;
	ViewInitOptions.bSceneCaptureUsesRayTracing = false;
	ViewInitOptions.SetViewRectangle(FIntRect(10, 100));

	FMinimalViewInfo ViewInfo;
	ViewInfo.Location = Location;
	ViewInfo.Rotation = Rotation;
	ViewInfo.FOV = 90.0f;
	ViewInfo.ProjectionMode = ECameraProjectionMode::Perspective;
	
	ViewInitOptions.ViewOrigin = Location;
	ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(Rotation) * FMatrix(
		FPlane(0,	0,	1,	0),
		FPlane(1,	0,	0,	0),
		FPlane(0,	1,	0,	0),
		FPlane(0,	0,	0,	1));
	ViewInitOptions.ProjectionMatrix = ViewInfo.CalculateProjectionMatrix();
	ViewInitOptions.ViewActor = Player->GetWorld()->GetFirstPlayerController()->GetCharacter();
	FSceneViewStateInterface* State = nullptr;
	State = GetRendererModule().AllocateViewState(InViewFamily.GetFeatureLevel(), nullptr);
	ViewInitOptions.SceneViewStateInterface = State;
	
	//FSceneView* const View = new FSceneView(ViewInitOptions);
	//InViewFamily.Views.Add(View);
	
	//View->StartFinalPostprocessSettings(Location);
	//View->EndFinalPostprocessSettings(ViewInitOptions);
}

void FFICSceneViewExtension::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) {
	
}

void FFICSceneViewExtension::SetupViewPoint(APlayerController* InPlayer, FMinimalViewInfo& InViewInfo) {
	FVector Location = Player->GetWorld()->GetFirstPlayerController()->GetCharacter()->GetActorLocation();
	//Location += FVector(0, 0, 500);
	//FRotator Rotation = FVector(0, 0, -1).Rotation();
	FRotator Rotation = Location.Rotation();
	Location = FVector(0, 0, 30000);
	
	//InViewInfo.Location = Location;
	//InViewInfo.Rotation = Rotation;
}

void FFICSceneViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily) {
	//InViewFamily.ViewMode = VMI_VisualizeLumen;
}

void FFICSceneViewExtension::PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) {
	//AFICSubsystem::GetFICSubsystem(Player)->FinalPostProcessSettings = InView.FinalPostProcessSettings;
}

void FFICSceneViewExtension::PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) {
	//AFICSubsystem::GetFICSubsystem(Player)->ExportRenderTarget(Exporter.ToSharedRef(), MakeShared<FFICRenderTarget_Raw>(const_cast<FRenderTarget*>(InViewFamily.RenderTarget)));
}
