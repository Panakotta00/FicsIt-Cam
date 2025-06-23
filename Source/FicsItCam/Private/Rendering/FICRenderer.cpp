#include "Rendering/FICRenderer.h"

#include "EngineModule.h"
#include "FGDockingStationInfo.h"
#include "FICCommandRender.h"
#include "LegacyScreenPercentageDriver.h"
#include "SceneManagement.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

FFICRenderer::~FFICRenderer() {
	ViewState.Destroy();
}

void FFICRenderer::AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) {
	FSceneViewStateInterface* ViewStateInterface = ViewState.GetReference();
	if (ViewStateInterface) {
		ViewStateInterface->AddReferencedObjects(ReferenceCollector);
	}
}

void FFICRenderer::Render(FRenderTarget* RenderTarget, FMinimalViewInfo ViewInfo, UWorld* World) {
	FEngineShowFlags ShowFlags = GEngine->GameViewport->EngineShowFlags;
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, World->Scene, ShowFlags).SetResolveScene(true).SetRealtimeUpdate(true));
	ViewFamily.bIsFirstViewInMultipleViewFamily = true;
	ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(ViewFamily, 1.0));

	//FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetCharacter()->GetActorLocation();
	//Location += FVector(0, 0, 500);
	//FRotator Rotation = FVector(0, 0, -1).Rotation();
	//FRotator Rotation = GetWorld()->GetFirstPlayerController()->GetCharacter()->GetActorRotation();
	//FVector Location = FVector(0, 0, 25000);
	//FRotator Rotation = (PlayerLocation - Location).Rotation();
	//Rotation = FVector(0,0,-1).Rotation();

	//ViewInfo.FOV = 90.0f;
	ViewInfo.ProjectionMode = ECameraProjectionMode::Perspective;
	ViewInfo.PostProcessSettings.AutoExposureSpeedDown = 1000000.0f;
	ViewInfo.PostProcessSettings.AutoExposureSpeedUp = 1000000.0f;
	ViewInfo.PostProcessSettings.bOverride_AutoExposureSpeedDown = false;
	ViewInfo.PostProcessSettings.bOverride_AutoExposureSpeedUp = false;
	ViewInfo.PostProcessBlendWeight = 1000.0f;

	FSceneViewInitOptions ViewInitOptions;
	ViewInitOptions.ViewLocation = ViewInfo.Location;
	ViewInitOptions.ViewRotation = ViewInfo.Rotation;
	ViewInitOptions.ViewFamily = &ViewFamily;
	ViewInitOptions.SetViewRectangle(FIntRect(FIntPoint(0), RenderTarget->GetSizeXY()));
	ViewInitOptions.BackgroundColor = FLinearColor::White;

	//FMinimalViewInfo Cache = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraCacheView();
	//ViewInfo.PostProcessSettings = Cache.PostProcessSettings;
	//ViewInfo.PostProcessBlendWeight = Cache.PostProcessBlendWeight;

	if (ViewState.GetReference() == nullptr) {
		const ERHIFeatureLevel::Type FeatureLevel = World->FeatureLevel.GetValue();
		ViewState.Allocate(FeatureLevel);
	}

	ViewInitOptions.ViewOrigin = ViewInfo.Location;
	ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(ViewInfo.Rotation) * FMatrix(
		FPlane(0,	0,	1,	0),
		FPlane(1,	0,	0,	0),
		FPlane(0,	1,	0,	0),
		FPlane(0,	0,	0,	1));
	ViewInitOptions.ProjectionMatrix = ViewInfo.CalculateProjectionMatrix();
	//ViewInitOptions.ViewActor = AFICSubsystem::GetFICSubsystem(this);
	ViewInitOptions.SceneViewStateInterface = ViewState.GetReference();

	FSceneView* const View = new FSceneView(ViewInitOptions);

	View->StartFinalPostprocessSettings(ViewInfo.Location);

	View->OverridePostProcessSettings(ViewInfo.PostProcessSettings, ViewInfo.PostProcessBlendWeight);

	View->EndFinalPostprocessSettings(ViewInitOptions);

	//View->FinalPostProcessSettings = FinalPostProcessSettings;

	ViewFamily.Views.Add(View);

	FCanvas Canvas(RenderTarget, NULL, World, World->FeatureLevel);
	GetRendererModule().BeginRenderingViewFamily(&Canvas, &ViewFamily);
	Canvas.Flush_GameThread();

	///AFICSubsystem::GetFICSubsystem(this)->ExportRenderTarget(SceneViewExporter.ToSharedRef(), MakeShared<FFICRenderTarget_Raw>(&*Viewport));

	//TSharedRef<FFICRenderRequest> RenderRequest = MakeShared<FFICRenderRequest>(RenderTarget, Exporter, FRHIGPUTextureReadback(TEXT("FICSubsystem Texture Readback")));

	/*FRHIGPUTextureReadback Readback("FIC Test");
	ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)([&](FRHICommandListImmediate& RHICmdList){
		FTexture2DRHIRef Target = Viewport->GetRenderTargetTexture();
		Readback.EnqueueCopy(RHICmdList, Target);
	});
	FlushRenderingCommands();

	FIntPoint TargetSize = Viewport->GetRenderTargetTexture()->GetSizeXY();
	FIntPoint ReadSize;
	ENQUEUE_RENDER_COMMAND(ReadbackFICCameraFootage)( [&](FRHICommandListImmediate& RHICmdList) {
		void* data = Readback.Lock(ReadSize.X, &ReadSize.Y);
		if (data) SceneViewExporter->AddFrame(data, ReadSize, TargetSize);
	});
	FlushRenderingCommands();*/
}
