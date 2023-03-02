#pragma once

extern "C" {
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
}

#include "CoreMinimal.h"

class FSequenceExporter {
public:
	virtual ~FSequenceExporter() = default;

	virtual bool Init() = 0;
	virtual void AddFrame(void* ptr, size_t len) = 0;
	virtual void Finish() = 0;
};

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
	bool bFinished = false;
	
public:
	FSequenceMP4Exporter(FIntPoint ImageSize, int FPS, FString InPath);
	~FSequenceMP4Exporter();
	
	virtual bool Init() override;
	virtual void AddFrame(void* ptr, size_t len) override;
	virtual void Finish() override;
	
	void ReadBuffer();
};

class FSequenceImageExporter : public FSequenceExporter {
private:
	FString Path;
	FIntPoint ImageSize;
	FDateTime StartTime;
	uint64 Increment = 0;

public:
	FSequenceImageExporter(FString InPath, FIntPoint InImageSize);

	virtual bool Init() override;
	virtual void AddFrame(void* ptr, size_t len) override;
	virtual void Finish() override;
};