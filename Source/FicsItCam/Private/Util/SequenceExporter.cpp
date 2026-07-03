#include "Util/SequenceExporter.h"

#include <string>

#include "Misc/DefaultValueHelper.h"
#include "FicsItCamModule.h"
#include "Misc/FileHelper.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/Paths.h"
#include "Engine/EngineBaseTypes.h"

#if PLATFORM_WINDOWS
FSequenceMP4Exporter::FSequenceMP4Exporter(FIntPoint InImageSize, int FPS, FString InPath, int AudioSampleRate) : ImageSize(InImageSize), FPS(FPS), Path(InPath), AudioSampleRate(AudioSampleRate) {}

FSequenceMP4Exporter::~FSequenceMP4Exporter() {
	if (VideoCodecContext) avcodec_free_context(&VideoCodecContext);
	if (AudioCodecContext) avcodec_free_context(&AudioCodecContext);
	if (VideoFrame) av_frame_free(&VideoFrame);
	if (AudioFrame) av_frame_free(&AudioFrame);
	if (VideoPacket) av_packet_free(&VideoPacket);
	if (AudioPacket) av_packet_free(&AudioPacket);
	if (SwsContext) sws_freeContext(SwsContext);
	if (SwrContext) swr_free(&SwrContext);
	if (FormatContext->pb) avio_closep(&FormatContext->pb);
	if (FormatContext) avformat_free_context(FormatContext);
}

FString ffmpeg_err2str_func(int ret) {
	char data[AV_ERROR_MAX_STRING_SIZE];
	av_make_error_string(data, AV_ERROR_MAX_STRING_SIZE, ret);
	return FString(UTF8_TO_TCHAR(data));
}

#define ffmpeg_err2str(ret) *ffmpeg_err2str_func(ret)

bool FSequenceMP4Exporter::Init() {
	int ret;

	FTCHARToUTF8 FilePath(*Path, Path.Len());
	std::string FilePathStr = std::string(FilePath.Get(), FilePath.Length());
	const char* format = NULL;
	TOptional<int64> video_bitrate;
	if (Path.StartsWith(TEXT("srt://"))) {
		format = "mpegts";
		if (auto option = FGenericPlatformHttp::GetUrlParameter(Path, TEXT("bitrate"))) {
			int64 bitrate;
			if (FDefaultValueHelper::ParseInt64(*option, bitrate)) {
				video_bitrate = bitrate;
			}
		}
	}

	// Find Output Format Context by file-ending otherwise use mpeg
	{
		ret = avformat_alloc_output_context2(&FormatContext, NULL, format, FilePathStr.c_str());
		if (!FormatContext && format == NULL) {
			format = "mpeg";
			ret = avformat_alloc_output_context2(&FormatContext, NULL, format, FilePathStr.c_str());
		}
		if (ret < 0 || !FormatContext) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create format context: %s"), ffmpeg_err2str((ret)));
			return false;
		}
	}

	// Create Video Stream & associated Settings (VideoCodecContext)
	{
		VideoCodec = avcodec_find_encoder(FormatContext->oformat->video_codec);
		if (!VideoCodec) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to get video codec from format context."));
			return false;
		}

		VideoPacket = av_packet_alloc();
		if (!VideoPacket) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create video codec output packet."));
			return false;
		}

		VideoStream = avformat_new_stream(FormatContext, VideoCodec);
		if (!VideoStream) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create video stream for format context."));
			return false;
		}
		VideoStream->id = FormatContext->nb_streams-1;

		VideoCodecContext = avcodec_alloc_context3(VideoCodec);
		if (!VideoCodecContext) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create video codec context for video codec."));
			return false;
		}
		// CodecContext->codec_id = FormatContext->oformat->video_codec; /// Nötig???
		if (video_bitrate) VideoCodecContext->bit_rate = *video_bitrate;
		VideoCodecContext->width = ImageSize.X;
		VideoCodecContext->height = ImageSize.Y;
		VideoStream->time_base = AVRational{1, FPS};
		VideoCodecContext->time_base = VideoStream->time_base;
		//CodecContext->framerate = AVRational{FPS, 1}; // Not needed with mux?
		//CodecContext->gop_size = 12;
		VideoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
		/*if (CodecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
			CodecContext->max_b_frames = 2;
		}
		if (CodecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
			CodecContext->mb_decision = 2;
		}
		if (FormatContext->oformat->flags & AVFMT_GLOBALHEADER)
			CodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;*/
		if (FormatContext->oformat->flags & AVFMT_GLOBALHEADER)
			VideoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	// Create Audio Stream & associated Settings/Objects
	if (AudioSampleRate > 0) {
		AudioCodec = avcodec_find_encoder(FormatContext->oformat->audio_codec);
		if (!AudioCodec) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to get audio codec from format context."));
			return false;
		}

		AudioPacket = av_packet_alloc();
		if (!AudioPacket) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create audio codec output packet."));
			return false;
		}

		AudioStream = avformat_new_stream(FormatContext, AudioCodec);
		if (!AudioStream) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create audio stream for format context."));
			return false;
		}
		AudioStream->id = FormatContext->nb_streams-1;

		AudioCodecContext = avcodec_alloc_context3(AudioCodec);
		if (!AudioCodecContext) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create audio codec context for audio codec."));
			return false;
		}
		AudioCodecContext->sample_fmt = AudioCodec->sample_fmts ? AudioCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
		AudioCodecContext->bit_rate = 64000;
		AudioCodecContext->sample_rate = AudioSampleRate;
		if (AudioCodec->supported_samplerates) {
			AudioCodecContext->sample_rate = AudioCodec->supported_samplerates[0];
			for (int i = 0; AudioCodec->supported_samplerates[i]; ++i) {
				if (AudioCodec->supported_samplerates[i] == AudioSampleRate) {
					AudioCodecContext->sample_rate = AudioSampleRate;
				}
			}
		}
		AVChannelLayout layout AV_CHANNEL_LAYOUT_STEREO;
		av_channel_layout_copy(&AudioCodecContext->ch_layout, &layout);
		AudioStream->time_base = AVRational{1, AudioCodecContext->sample_rate};
		if (FormatContext->oformat->flags & AVFMT_GLOBALHEADER)
			AudioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	// Open and Init Video Stream
	{
		ret = avcodec_open2(VideoCodecContext, VideoCodec, NULL);
		if (ret < 0) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to open video codec for video codec context: %s"), ffmpeg_err2str(ret));
			return false;
		}

		VideoFrame = av_frame_alloc();
		if (!VideoFrame) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create sws2codec video frame."));
			return false;
		}
		VideoFrame->format = VideoCodecContext->pix_fmt;
		VideoFrame->width = VideoCodecContext->width;
		VideoFrame->height = VideoCodecContext->height;

		ret = av_frame_get_buffer(VideoFrame, 0);
		if (ret < 0) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to allocate sws2codec video frame buffers: %s"), ffmpeg_err2str(ret));
			return false;
		}

		ret = avcodec_parameters_from_context(VideoStream->codecpar, VideoCodecContext);
		if (ret < 0) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to set video stream settings to video codec context settings: %s"), ffmpeg_err2str(ret));
			exit(1);
		}
	}

	// Open and Init Audio Stream
	if (AudioSampleRate > 0) {
		ret = avcodec_open2(AudioCodecContext, AudioCodec, NULL);
		if (ret < 0) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to open audio codec for audio codec context: %s"), ffmpeg_err2str(ret));
			return false;
		}

		AudioFrame = av_frame_alloc();
		if (!AudioFrame) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create sws2codec audio frame."));
			return false;
		}
		AudioFrame->format = AudioCodecContext->sample_fmt;
		av_channel_layout_copy(&AudioFrame->ch_layout, &AudioCodecContext->ch_layout);
		AudioFrame->sample_rate = AudioCodecContext->sample_rate;
		AudioFrame->nb_samples = AudioCodecContext->frame_size;

		if (AudioFrame->nb_samples) {
			ret = av_frame_get_buffer(AudioFrame, 0);
			if (ret < 0) {
				UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to allocate sws2codec audio frame buffers: %s"), ffmpeg_err2str(ret));
				return false;
			}
		}

		ret = avcodec_parameters_from_context(AudioStream->codecpar, AudioCodecContext);
		if (ret < 0) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to set audio stream settings to audio codec context settings: %s"), ffmpeg_err2str(ret));
			exit(1);
		}
	}
	
	SwsContext = sws_getContext(VideoCodecContext->width, VideoCodecContext->height, AV_PIX_FMT_RGBA, VideoCodecContext->width, VideoCodecContext->height, VideoCodecContext->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
	if (!SwsContext) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create sws context for color space conversion."));
		return false;
	}

	if (AudioSampleRate > 0) {
		SwrContext = swr_alloc();
		if (!SwrContext) {
			UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create swr context for audio sample rate conversion."));
			return false;
		}

		av_opt_set_chlayout  (SwrContext, "in_chlayout",       &AudioCodecContext->ch_layout,      0);
		av_opt_set_int       (SwrContext, "in_sample_rate",     AudioSampleRate,    0);
		av_opt_set_sample_fmt(SwrContext, "in_sample_fmt",      AV_SAMPLE_FMT_FLT, 0);
		av_opt_set_chlayout  (SwrContext, "out_chlayout",      &AudioCodecContext->ch_layout,      0);
		av_opt_set_int       (SwrContext, "out_sample_rate",    AudioCodecContext->sample_rate,    0);
		av_opt_set_sample_fmt(SwrContext, "out_sample_fmt",     AudioCodecContext->sample_fmt,     0);

		/* initialize the resampling context */
		if ((ret = swr_init(SwrContext)) < 0) {
			fprintf(stderr, "Failed to initialize the resampling context\n");
			exit(1);
		}
	}

	ret = avio_open(&FormatContext->pb, FilePathStr.c_str(), AVIO_FLAG_WRITE);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to open output file: %s"), ffmpeg_err2str(ret));
		return false;
	}
	
	ret = avformat_write_header(FormatContext, NULL);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to write format file header: %s"), ffmpeg_err2str(ret));
		return false;
	}
	
	return true;
}

void FSequenceMP4Exporter::Finish() {
	FSequenceExporter::Finish();

	if (!FormatContext) return;

	// Flush delayed packets from encoders (important for H.264/AAC and MP4)
	if (VideoCodecContext && VideoPacket) {
		int ret = avcodec_send_frame(VideoCodecContext, nullptr);
		if (ret < 0) {
			UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to flush video encoder: %s"), ffmpeg_err2str(ret));
		}

		ReadVideoBuffer();
	}

	if (AudioCodecContext && AudioPacket) {
		int ret = avcodec_send_frame(AudioCodecContext, nullptr); // signal EOF
		if (ret < 0) {
			UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to flush audio encoder: %s"), ffmpeg_err2str(ret));
		}

		ReadAudioBuffer();
	}
	
	int ret = av_write_trailer(FormatContext);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to write format file trailer (may corrupt file): %s"), ffmpeg_err2str(ret));
	}

	if (FormatContext->pb) {
		avio_close(FormatContext->pb);
		FormatContext->pb = nullptr;
	}
}

void FSequenceMP4Exporter::AddFrame(EPixelFormat InFormat, void* ptr, FIntPoint ReadSize, FIntPoint Size, double Time) {
	if (bFinished) return;

	int64 FramePts = VideoFrameNr++;
	
	int ret = av_frame_make_writable(VideoFrame);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to mark video frame as writeable for sws (skip frame %lld): %s"), FramePts, ffmpeg_err2str(ret));
		return;
	}

	const uint8* data[] = {(uint8*)ptr, NULL};
	int stride[] = {ReadSize.X*4, 0};
		
	ret = sws_scale(SwsContext, data, stride, 0, VideoCodecContext->height, VideoFrame->data, VideoFrame->linesize);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to convert video color spaces using sws context (skip frame %lld): %s"), FramePts, ffmpeg_err2str(ret));
		return;
	}
	
	ret = avcodec_send_frame(VideoCodecContext,  VideoFrame);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to send video frame to encoding (skip frame %lld): %s"), FramePts, ffmpeg_err2str(ret));
		return;
	}

	if (Time < 0.0) {
		VideoFrame->pts = Time*FPS;
	} else {
		VideoFrame->pts = FramePts;
	}
	
	ReadVideoBuffer();
}

void FSequenceMP4Exporter::AddAudioFrame(float* Samples, int SampleCount) {
	if (bFinished) return;
	if (AudioSampleRate < 1) return;

	uint64 nb_samples = swr_get_delay(SwrContext, AudioCodecContext->sample_rate) + AudioFrame->nb_samples;

	int ret = av_frame_make_writable(AudioFrame);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to mark audio frame as writeable for sws (skip frame): %s"), ffmpeg_err2str(ret));
		return;
	}

	const uint8_t* srcData[3] = {(const uint8_t*)Samples, NULL};
	nb_samples = swr_convert(SwrContext, AudioFrame->data, nb_samples, srcData, SampleCount);

	uint64 audioSampleCount = AudioSampleCount;
	AudioFrame->nb_samples = nb_samples;
	AudioFrame->pts = av_rescale_q(AudioSampleCount, AVRational{1, AudioCodecContext->sample_rate}, AudioCodecContext->time_base);
	AudioSampleCount += nb_samples;

	ret = avcodec_send_frame(AudioCodecContext, AudioFrame);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to send audio frame to encoding (skip samples [%lld - %lld]): %s"), audioSampleCount, AudioSampleCount, ffmpeg_err2str(ret));
	}

	ReadAudioBuffer();
}

void FSequenceMP4Exporter::ReadVideoBuffer() {
	int ret = 0;
	while (ret >= 0) {
		ret = avcodec_receive_packet(VideoCodecContext, VideoPacket);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
		else if (ret < 0) {
			UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to receive encoded video data packet (skip packet, may corrupt output file): %s"), ffmpeg_err2str(ret));
			return;
		}
		av_packet_rescale_ts(VideoPacket, VideoCodecContext->time_base, VideoStream->time_base);
		VideoPacket->stream_index = VideoStream->index;
		ret = av_interleaved_write_frame(FormatContext, VideoPacket);
		av_packet_unref(VideoPacket);
		if (ret < 0) {
			UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to write encoded video data packet to output format and file (skip packet, may corrupt output file): %s"), ffmpeg_err2str(ret));
			return;
		}
	}
}

void FSequenceMP4Exporter::ReadAudioBuffer() {
	int ret = 0;
	while (ret >= 0) {
		ret = avcodec_receive_packet(AudioCodecContext, AudioPacket);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
		else if (ret < 0) {
			UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to receive encoded audio data packet (skip packet, may corrupt output file): %s"), ffmpeg_err2str(ret));
			return;
		}
		av_packet_rescale_ts(AudioPacket, AudioCodecContext->time_base, AudioStream->time_base);
		AudioPacket->stream_index = AudioStream->index;
		ret = av_interleaved_write_frame(FormatContext, AudioPacket);
		av_packet_unref(AudioPacket);
		if (ret < 0) {
			UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to write encoded audio data packet to output format and file (skip packet, may corrupt output file): %s"), ffmpeg_err2str(ret));
			return;
		}
	}
}
#endif

FSequenceImageExporter::FSequenceImageExporter(FString InPath, FIntPoint InImageSize) : Path(InPath), ImageSize(InImageSize) {}

bool FSequenceImageExporter::Init() {
	StartTime = FDateTime::Now();
	return true;
}

void FSequenceImageExporter::AddFrame(EPixelFormat Format, void* ptr, FIntPoint ReadSize, FIntPoint Size, double Time) {
	FString FilePath = FPaths::Combine(Path, FString::Printf(TEXT("%s-%05llu.jpg"), *StartTime.ToString(), Increment++));

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	
	/*TArray<uint8> Data;
	Data.AddUninitialized(ReadSize.X * ReadSize.Y * 4);
	for (int Y = 0; Y < ReadSize.Y; ++Y) {
		for (int X = 0; X < ReadSize.X; ++X) {
			uint32& pxl = *(((uint32*)ptr) + ReadSize.X * Y + X);
			*(Data.GetData() + Y * ReadSize.X + X) = (uint16)((pxl >> 0) & 0x3FF);
			*(Data.GetData() + Y * ReadSize.X + X + 1) = (uint16)((pxl >> 10) & 0x3FF);
			*(Data.GetData() + Y * ReadSize.X + X + 2) = (uint16)((pxl >> 20) & 0x3FF);
			*(Data.GetData() + Y * ReadSize.X + X + 3) = 0xFF;
		}
	}
	
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	ImageWrapper->SetRaw(Data.GetData(), ReadSize.X*ReadSize.Y*4, ReadSize.X, ReadSize.Y, ERGBFormat::RGBA, 8, ReadSize.X*4);
	TArray64<uint8> CompressedData = ImageWrapper->GetCompressed(100);*/
	


	
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	ImageWrapper->SetRaw(ptr, ReadSize.X*ReadSize.Y*4, Size.X, Size.Y, ERGBFormat::RGBA, 8, ReadSize.X*4);
	TArray64<uint8> CompressedData = ImageWrapper->GetCompressed(100);

	FFileHelper::SaveArrayToFile(CompressedData, *FilePath);
}

void FSequenceImageExporter::Finish() {
	FSequenceExporter::Finish();
}
