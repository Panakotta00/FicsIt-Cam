#pragma once

#if PLATFORM_WINDOWS
extern "C" {
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
}
#endif

#include "CoreMinimal.h"
#include "PixelFormat.h"

class FSequenceExporter {
protected:
	bool bFinished = false;
	
public:
	virtual ~FSequenceExporter() {
		if (!bFinished) Finish();
	}

	virtual bool Init() = 0;
	virtual void AddFrame(EPixelFormat Format, void* ptr, FIntPoint ReadSize, FIntPoint Size) = 0;
	virtual void Finish() {
		bFinished = true;
	};
};

#if PLATFORM_WINDOWS
class FSequenceMP4Exporter : public FSequenceExporter {
private:
	FIntPoint ImageSize;
	int FPS;
	FString Path;
	AVFormatContext* FormatContext = nullptr;
	AVStream* VideoStream = nullptr;
	const AVCodec* VideoCodec = nullptr;
	AVCodecContext* CodecContext = nullptr;
	AVFrame* Frame = nullptr;
	AVPacket* Packet = nullptr;
	SwsContext* SwsContext = nullptr;
	IFileHandle* File = nullptr;
	int64 FrameNr = 0;
	
public:
	FSequenceMP4Exporter(FIntPoint ImageSize, int FPS, FString InPath);
	~FSequenceMP4Exporter();
	
	virtual bool Init() override;
	virtual void AddFrame(EPixelFormat Format, void* ptr, FIntPoint ReadSize, FIntPoint Size) override;
	virtual void Finish() override;
	
	void ReadBuffer();
};
#endif

class FSequenceImageExporter : public FSequenceExporter {
private:
	FString Path;
	FIntPoint ImageSize;
	FDateTime StartTime;
	uint64 Increment = 0;

public:
	FSequenceImageExporter(FString InPath, FIntPoint InImageSize);

	virtual bool Init() override;
	virtual void AddFrame(EPixelFormat Format, void* ptr, FIntPoint ReadSize, FIntPoint Size) override;
	virtual void Finish() override;
};