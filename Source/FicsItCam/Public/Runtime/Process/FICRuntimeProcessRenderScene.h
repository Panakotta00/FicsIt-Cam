#pragma once

#include "CoreMinimal.h"
#include "CanvasTypes.h"
#include "FICRuntimeProcessPlayScene.h"
#include "FICSubsystem.h"
#include "Util/SequenceExporter.h"
#include "FICRUntimeProcessRenderScene.generated.h"

inline FName NAME_FICRendererViewport = TEXT("FICRendererViewport");

class FFICRendererViewport : public FViewport, public FFICRenderTarget {
public:
	FFICRendererViewport(FViewportClient* InViewportClient, int SizeX, int SizeY) : FViewport(InViewportClient), DebugCanvas(NULL) {
		this->SizeX = SizeX;
		this->SizeY = SizeY;
		ViewportType = NAME_FICRendererViewport;
		UWorld* CurWorld = (InViewportClient != NULL ? InViewportClient->GetWorld() : NULL);
		DebugCanvas = new FCanvas(this, NULL, CurWorld, (CurWorld != NULL ? CurWorld->GetFeatureLevel() : GMaxRHIFeatureLevel));
		
		DebugCanvas->SetAllowedModes(0);

		BeginInitResource(this);
	}

	~FFICRendererViewport() {
		BeginReleaseResource(this);
		FlushRenderingCommands();
		
		if (DebugCanvas != NULL) {
			delete DebugCanvas;
			DebugCanvas = NULL;
		}
	}
	
	// Begin FViewport
	virtual void BeginRenderFrame(FRHICommandListImmediate& RHICmdList) override {
		check( IsInRenderingThread() );
	};

	virtual void EndRenderFrame(FRHICommandListImmediate& RHICmdList, bool bPresent, bool bLockToVsync) override {
		check( IsInRenderingThread() );
	}

	virtual void* GetWindow() override { return 0; }
	virtual void MoveWindow(int32 NewPosX, int32 NewPosY, int32 NewSizeX, int32 NewSizeY) override {}
	virtual void Destroy() override {}
	virtual bool SetUserFocus(bool bFocus) override { return false; }
	virtual bool KeyState(FKey Key) const override { return false; }
	virtual int32 GetMouseX() const override { return 0; }
	virtual int32 GetMouseY() const override { return 0; }
	virtual void GetMousePos( FIntPoint& MousePosition, const bool bLocalPosition = true) override { MousePosition = FIntPoint(0, 0); }
	virtual void SetMouse(int32 x, int32 y) override { }
	virtual void ProcessInput( float DeltaTime ) override { }
	virtual FVector2D VirtualDesktopPixelToViewport(FIntPoint VirtualDesktopPointPx) const override { return FVector2D::ZeroVector; }
	virtual FIntPoint ViewportToVirtualDesktopPixel(FVector2D ViewportCoordinate) const override { return FIntPoint::ZeroValue; }
	virtual void InvalidateDisplay() override { }
	virtual void DeferInvalidateHitProxy() override { }
	virtual FViewportFrame* GetViewportFrame() override { return 0; }
	virtual FCanvas* GetDebugCanvas() override { return DebugCanvas; }
	// End FViewport

	// Begin FRenderResource
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override {
		const FRHITextureCreateDesc Desc =
			FRHITextureCreateDesc::Create2D(TEXT("FIC Renderer Surface"))
			.SetExtent(SizeX, SizeY)
			.SetFormat(PF_R8G8B8A8)
			.SetFlags(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource)
			.SetInitialState(ERHIAccess::SRVMask);

		RenderTargetTextureRHI = RHICreateTexture(Desc);
	}

	virtual FString GetFriendlyName() const override { return FString(TEXT("FFICRendererViewport"));}
	// End FRenderResource Interface

	// Begin FFICRenderTarget
	virtual FTexture2DRHIRef GetRenderTarget() override { return this->GetRenderTargetTexture(); }
	// End FFICRenderTarget
	
private:
	FCanvas* DebugCanvas;
};

UCLASS()
class UFICRuntimeProcessRenderScene : public UFICRuntimeProcessPlayScene {
	GENERATED_BODY()
public:
	UPROPERTY()
	AFICCaptureCamera* CaptureCamera = nullptr;
	TSharedPtr<FFICRendererViewport> DummyViewport = nullptr;
	TSharedPtr<FSequenceExporter> Exporter;
	FString Path;
	TSharedPtr<SWidget> Overlay;

	TArray<float> ETAStatistics;

	FICFrame FrameProgress = 0;

	float PrevMinUndilatedFrameTime = 0;
	float PrevMaxUndilatedFrameTime = 0;

	// Begin UFICRuntimeProcess
	virtual void Start(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) override;
	virtual void Stop(AFICRuntimeProcessorCharacter* InCharacter) override;
	// End UFICRuntimeProcess

	void Frame();

	UFUNCTION(BlueprintCallable)
	static UFICRuntimeProcessRenderScene* StartRenderScene(AFICScene* InScene);
};