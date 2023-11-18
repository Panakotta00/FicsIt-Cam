#include "Exporter/FICImageSequenceExporter.h"

#include "IImageWrapperModule.h"

FFICImageSequenceExporter::FFICImageSequenceExporter(FString InPath, FIntPoint InImageSize) : Path(InPath), ImageSize(InImageSize) {}

bool FFICImageSequenceExporter::Init() {
	StartTime = FDateTime::Now();
	return true;
}

void FFICImageSequenceExporter::AddFrame(EPixelFormat Format, void* ptr, FIntPoint ReadSize, FIntPoint Size) {
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

void FFICImageSequenceExporter::Finish() {
	FFICSequenceExporter::Finish();
}
