#include "FICSubsystem.h"

#include <Subsystem/SubsystemActorManager.h>

#include "FICCommand.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"

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

AFICSubsystem::AFICSubsystem() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFICSubsystem::BeginPlay() {
	Super::BeginPlay();
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
	
	if (Camera) Camera->Destroy();
	Camera = nullptr;
}

bool AFICSubsystem::ShouldSave_Implementation() const {
	return true;
}

AFICSubsystem* AFICSubsystem::GetFICSubsystem(UObject* WorldContext) {
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFICSubsystem>();
}

void AFICSubsystem::PlayAnimation(AFICAnimation* Path, bool bDoRender) {
	CreateCamera();
	if (!Camera || !Path) return;
	Camera->StartAnimation(Path, bDoRender);
}

void AFICSubsystem::StopAnimation() {
	if (!Camera) return;
	Camera->StopAnimation();
}

void AFICSubsystem::AddVisibleAnimation(AFICAnimation* Path) {
	if (Path) VisibleAnimations.Add(Path);
}

void AFICSubsystem::SetActiveAnimation(AFICAnimation* inActiveAnimation) {
	if (EditorContext) {
		EditorContext->HideEditor();
		EditorContext = nullptr;
	}
	ActiveAnimation = inActiveAnimation;
	if (ActiveAnimation) {
		EditorContext = NewObject<UFICEditorContext>(this);
		EditorContext->SetAnimation(ActiveAnimation);
		EditorContext->ShowEditor();
	}
}

UFICEditorContext* AFICSubsystem::GetEditor() const {
	return EditorContext;
}

void AFICSubsystem::CreateCamera() {
	if (!Camera) Camera = GetWorld()->SpawnActor<AFICCameraCharacter>();
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
