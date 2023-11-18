#include "Exporter/FICFfmpegExporter.h"

#include <string>

#include "FicsItCamModule.h"
#include "FICSubsystem.h"
#include "Data/FICScene.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Runtime/Process/FICRuntimeProcess.h"
#include "SlateFileDialogs/Public/ISlateFileDialogModule.h"
#include "Util/FICWindowsUtils.h"

extern "C" {
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
}

UFICFfmpegExporter::UFICFfmpegExporter() {
	ExporterName = FText::FromString(TEXT("FFmpeg Exporter"));
}

TFICDynamicStruct<FFICExporterSpecificSettings> UFICFfmpegExporter::CreateSpecificSettings(UObject* Context) {
	FFICFfmpegSettings Settings;
	
	FString Name;
	if (AFICScene* Scene = Cast<AFICScene>(Context)) {
		Name = Scene->SceneName;
		Settings.ExportFPS = Scene->FPS;
	} else if (UFICRuntimeProcess* Process = Cast<UFICRuntimeProcess>(Context)) {
		Name = AFICSubsystem::GetFICSubsystem(Context)->FindRuntimeProcessKey(Process);
	}
	
	FString FolderPath;
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	if (FolderPath.IsEmpty()) {
		FolderPath = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/FicsItCam/"));
	}
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*FolderPath)) PlatformFile.CreateDirectoryTree(*FolderPath);
	FString FileName = FDateTime::Now().ToString() + TEXT(".mp4");
	if (!Name.IsEmpty()) {
		FileName = Name + TEXT("-") + FileName;
	}
	
	Settings.Path = FPaths::Combine(FolderPath, FileName);
	
	return FFICDynamicStruct(Settings);
}

TSharedRef<FFICSequenceExporter> UFICFfmpegExporter::CreateExporter(const FFICExportSettings& InExportSettings) {
	FFICFfmpegSettings& FfmpegSettings = InExportSettings.ExporterSpecificSettings.Get<FFICFfmpegSettings>();
	return MakeShared<FFICFfmpegSequenceExporter>(InExportSettings.Resolution, FfmpegSettings.ExportFPS, FfmpegSettings.Path);
}

void UFICFfmpegExporter::SetSpecificFfmpegSettings(FFICExportSettings& ExportSettings, const FFICFfmpegSettings& SpecificSettings) {
	check(ExportSettings.Exporter && ExportSettings.Exporter->IsChildOf<UFICFfmpegExporter>());
	ExportSettings.ExporterSpecificSettings = FFICDynamicStruct(SpecificSettings);
}

FFICFfmpegSettings UFICFfmpegExporter::GetSpecificFfmpegSettings(const FFICExportSettings& ExportSettings) {
	return ExportSettings.ExporterSpecificSettings.Get<FFICFfmpegSettings>();
}

FString UFICFfmpegExporter::OpenFileDialogFfmpeg(UWidget* ParentWidget, const FString& Path) {
	TArray<FString> FileNames;
	TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(ParentWidget->GetCachedWidget().ToSharedRef());
	void* NativeWindow = Window->GetNativeWindow()->GetOSWindowHandle();
	FFICWindowsUtils::SaveFileDialog(NativeWindow, TEXT("FFmpeg Export File"), Path, TEXT("MP4|*.mp4"), FileNames);
	if (FileNames.Num() < 1) return Path;
	return FileNames[0];
}

FFICFfmpegSequenceExporter::FFICFfmpegSequenceExporter(FIntPoint InImageSize, int FPS, FString InPath) : ImageSize(InImageSize), FPS(FPS), Path(InPath) {}

FFICFfmpegSequenceExporter::~FFICFfmpegSequenceExporter() {
	if (SwsContext) sws_freeContext(SwsContext);
	if (FormatContext->pb) avio_closep(&FormatContext->pb);
	if (Frame) av_frame_free(&Frame);
	if (CodecContext) avcodec_free_context(&CodecContext);
	if (Packet) av_packet_free(&Packet);
	if (FormatContext) avformat_free_context(FormatContext);
}

FString ffmpeg_err2str_func(int ret) {
	char data[AV_ERROR_MAX_STRING_SIZE];
	av_make_error_string(data, AV_ERROR_MAX_STRING_SIZE, ret);
	return FString(UTF8_TO_TCHAR(data));
}

#define ffmpeg_err2str(ret) *ffmpeg_err2str_func(ret)

bool FFICFfmpegSequenceExporter::Init() {
	FTCHARToUTF8 FilePath(*Path, Path.Len());
	std::string FilePathStr = std::string(FilePath.Get(), FilePath.Length());
	
	int ret = avformat_alloc_output_context2(&FormatContext, NULL, NULL, FilePathStr.c_str());
	if (!FormatContext) {
		ret = avformat_alloc_output_context2(&FormatContext, NULL, "mpeg", FilePathStr.c_str());
	}
	if (ret < 0 || !FormatContext) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create format context: %s"), ffmpeg_err2str((ret)));
		return false;
	}
	
	VideoCodec = avcodec_find_encoder(FormatContext->oformat->video_codec);
	if (!VideoCodec) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to get video codec from format context."));
		return false;
	}

	Packet = av_packet_alloc();
	if (!Packet) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create codec output packet."));
		return false;
	}

	VideoStream = avformat_new_stream(FormatContext, NULL);
	if (!VideoStream) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create video stream for format context."));
		return false;
	}
	VideoStream->id = FormatContext->nb_streams-1;

	CodecContext = avcodec_alloc_context3(VideoCodec);
	if (!CodecContext) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create video codec context for video codec."));
		return false;
	}
	// CodecContext->codec_id = FormatContext->oformat->video_codec; /// Nötig???
	//CodecContext->bit_rate = 40000000;
	CodecContext->width = ImageSize.X;
	CodecContext->height = ImageSize.Y;
	VideoStream->time_base = AVRational{1, FPS};
	CodecContext->time_base = VideoStream->time_base;
	//CodecContext->framerate = AVRational{FPS, 1}; // Not needed with mux?
	//CodecContext->gop_size = 12;
	CodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	/*if (CodecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
		CodecContext->max_b_frames = 2;
	}
	if (CodecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
		CodecContext->mb_decision = 2;
	}
	if (FormatContext->oformat->flags & AVFMT_GLOBALHEADER)
		CodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;*/

	ret = avcodec_open2(CodecContext, VideoCodec, NULL);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to open video codec for video codec context: %s"), ffmpeg_err2str(ret));
		return false;
	}

	Frame = av_frame_alloc();
	if (!Frame) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create sws2codec frame."));
		return false;
	}
	Frame->format = CodecContext->pix_fmt;
	Frame->width = CodecContext->width;
	Frame->height = CodecContext->height;

	ret = av_frame_get_buffer(Frame, 0);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to allocate sws2codec frame buffers: %s"), ffmpeg_err2str(ret));
		return false;
	}
	
	ret = avcodec_parameters_from_context(VideoStream->codecpar, CodecContext);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to set video stream settings to video codec context settings: %s"), ffmpeg_err2str(ret));
		return false;
	}
	
	SwsContext = sws_getContext(CodecContext->width, CodecContext->height, AV_PIX_FMT_RGBA, CodecContext->width, CodecContext->height, CodecContext->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
	if (!SwsContext) {
		UE_LOG(LogFicsItCam, Error, TEXT("FFmpeg-Export: Failed to create sws context for color space conversion."));
		return false;
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

void FFICFfmpegSequenceExporter::Finish() {
	FFICSequenceExporter::Finish();
	
	int ret = av_write_trailer(FormatContext);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to write format file trailer (may corrupt file): %s"), ffmpeg_err2str(ret));
	}
}

void FFICFfmpegSequenceExporter::AddFrame(EPixelFormat Format, void* ptr, FIntPoint ReadSize, FIntPoint Size) {
	if (bFinished) return;

	int64 FramePts = FrameNr++;
	
	int ret = av_frame_make_writable(Frame);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to mark video frame as writeable for sws (skip frame %lld): %hs"), FramePts, ffmpeg_err2str(ret));
		return;
	}

	const uint8* data[] = {(uint8*)ptr, NULL};
	int stride[] = {ReadSize.X*4, 0};
		
	ret = sws_scale(SwsContext, data, stride, 0, CodecContext->height, Frame->data, Frame->linesize);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to convert video color spaces using sws context (skip frame %lld): %hs"), FramePts, ffmpeg_err2str(ret));
		return;
	}
	
	ret = avcodec_send_frame(CodecContext,  Frame);
	if (ret < 0) {
		UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to send video frame to encoding (skip frame %lld): %hs"), FramePts, ffmpeg_err2str(ret));
		return;
	}
	
	Frame->pts = FramePts;
	
	ReadBuffer();
}

void FFICFfmpegSequenceExporter::ReadBuffer() {
	int ret = 0;
	while (ret >= 0) {
		ret = avcodec_receive_packet(CodecContext, Packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
		else if (ret < 0) {
			UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to receive encoded video data packet (skip packet, may corrupt output file): %hs"), ffmpeg_err2str(ret));
			return;
		}
		av_packet_rescale_ts(Packet, CodecContext->time_base, VideoStream->time_base);
		Packet->stream_index = VideoStream->index;
		ret = av_interleaved_write_frame(FormatContext, Packet);
		if (ret < 0) {
			UE_LOG(LogFicsItCam, Warning, TEXT("FFmpeg-Export: Failed to write encoded video data packet to output format and file (skip packet, may corrupt output file): %hs"), ffmpeg_err2str(ret));
			return;
		}
	}
}
