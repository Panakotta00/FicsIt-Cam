#pragma once

#include "CoreMinimal.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "CanvasTypes.h"
#include "Rendering/FICDummyViewport.h"
#include "FICRuntimeProcessPlayScene.h"
#include "FICSubsystem.h"
#include "Components/Viewport.h"
#include "Util/SequenceExporter.h"
#include "FICRuntimeProcessRenderScene.generated.h"

inline FName NAME_FICRendererViewport = TEXT("FICRendererViewport");

class FFICRendererViewport : public FViewport, public FFICRenderTarget {
public:
	FFICRendererViewport(FViewportClient* InViewportClient, int SizeX, int SizeY) : FViewport(InViewportClient), DebugCanvas(nullptr) {
		this->SizeX = SizeX;
		this->SizeY = SizeY;
		ViewportType = NAME_FICRendererViewport;
		UWorld* CurWorld = (InViewportClient != nullptr ? InViewportClient->GetWorld() : nullptr);
		DebugCanvas = new FCanvas(this, nullptr, CurWorld, (CurWorld != nullptr ? CurWorld->GetFeatureLevel() : GMaxRHIFeatureLevel));
		
		DebugCanvas->SetAllowedModes(0);

		BeginInitResource(this);
	}

	~FFICRendererViewport() {
		BeginReleaseResource(this);
		FlushRenderingCommands();
		
		if (DebugCanvas != nullptr) {
			delete DebugCanvas;
			DebugCanvas = nullptr;
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
			FRHITextureCreateDesc::Create2D(TEXT("BufferedRT"))
			.SetExtent(SizeX, SizeY)
			.SetFormat(PF_R8G8B8A8)
			.SetFlags(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource)
			.SetInitialState(ERHIAccess::SRVMask);

		RenderTargetTextureRHI = RHICreateTexture(Desc);
	}

	virtual FString GetFriendlyName() const override { return FString(TEXT("FFICRendererViewport"));}
	// End FRenderResource Interface

	// Begin FFICRenderTarget
	virtual FTextureRHIRef GetRenderTarget() override { return this->GetRenderTargetTexture(); }
	// End FFICRenderTarget
	
private:
	FCanvas* DebugCanvas = nullptr;
};

UCLASS()
class UFICRuntimeProcessRenderScene : public UFICRuntimeProcessPlayScene {
	GENERATED_BODY()
public:
	TSharedPtr<FFICRendererViewport> DummyViewport = nullptr;
	TSharedPtr<FFICDummyViewport> DummyViewportInterface;
	TSharedPtr<FSequenceExporter> Exporter;
	FString Path;
	TSharedPtr<SWidget> Overlay;
	int AudioSampleRate = 44100;
	AkOutputDeviceID m_defaultOutputDeviceId = AK_INVALID_OUTPUT_DEVICE_ID;
	IWwiseSoundEngineAPI* WwiseSoundEngineAPI = nullptr;
	bool bStarted = false;
	bool bSkipAudio = false;

	TArray<float> ETAStatistics;

	FICFrame FrameProgress = 0;

	float PrevMinUndilatedFrameTime = 0;
	float PrevMaxUndilatedFrameTime = 0;

	~UFICRuntimeProcessRenderScene();

	// Begin UFICRuntimeProcess
	virtual void Start(AFICRuntimeProcessorCharacter* InCharacter) override;
	virtual void Tick(AFICRuntimeProcessorCharacter* InCharacter, float DeltaSeconds) override;
	virtual void Stop(AFICRuntimeProcessorCharacter* InCharacter) override;
	// End UFICRuntimeProcess

	void Frame();

	UFUNCTION(BlueprintCallable)
	static UFICRuntimeProcessRenderScene* StartRenderScene(AFICScene* InScene);
};