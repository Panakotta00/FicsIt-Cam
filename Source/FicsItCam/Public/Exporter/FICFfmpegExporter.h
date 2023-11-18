#pragma once

#include "CoreMinimal.h"
#include "FICExporter.h"
#include "FICFfmpegExporter.generated.h"

USTRUCT(BlueprintType)
struct FFICFfmpegSettings : public FFICExporterSpecificSettings {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Path = "unkown.mp4";

	UPROPERTY(BlueprintReadWrite)
	int64 ExportFPS = 60;

	virtual bool IsValid(const struct FFICExportSettings& ExportSettings) const override {
		return !Path.IsEmpty()
		&& ExportFPS > 0;
	}
};

UCLASS()
class UFICFfmpegExporter : public UFICExporterDescriptor {
	GENERATED_BODY()
public:
	UFICFfmpegExporter();

	// Begin UFICExporterDescription
	virtual TFICDynamicStruct<FFICExporterSpecificSettings> CreateSpecificSettings(UObject* Context) override;
	virtual TSharedRef<FFICSequenceExporter> CreateExporter(const FFICExportSettings& InSettings) override;
	// End UFICExporterDescription

	UFUNCTION(BlueprintCallable)
	static void SetSpecificFfmpegSettings(UPARAM(ref) FFICExportSettings& ExportSettings, const FFICFfmpegSettings& SpecificSettings);
	UFUNCTION(BlueprintCallable)
	static FFICFfmpegSettings GetSpecificFfmpegSettings(const FFICExportSettings& ExportSettings);
	UFUNCTION(BlueprintCallable)
	static FString OpenFileDialogFfmpeg(UWidget* ParentWidget, const FString& Path);
};

class FFICFfmpegSequenceExporter : public FFICSequenceExporter {
private:
	FIntPoint ImageSize;
	int FPS;
	FString Path;
	struct AVFormatContext* FormatContext = nullptr;
	struct AVStream* VideoStream = nullptr;
	const struct AVCodec* VideoCodec = nullptr;
	struct AVCodecContext* CodecContext = nullptr;
	struct AVFrame* Frame = nullptr;
	struct AVPacket* Packet = nullptr;
	struct SwsContext* SwsContext = nullptr;
	IFileHandle* File = nullptr;
	int64 FrameNr = 0;
	
public:
	FFICFfmpegSequenceExporter(FIntPoint ImageSize, int FPS, FString InPath);
	~FFICFfmpegSequenceExporter();
	
	virtual bool Init() override;
	virtual void AddFrame(EPixelFormat Format, void* ptr, FIntPoint ReadSize, FIntPoint Size) override;
	virtual void Finish() override;
	
	void ReadBuffer();
};
