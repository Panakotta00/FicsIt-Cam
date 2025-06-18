#include "Util/FICProceduralTexture.h"

#include "ImageUtils.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"

bool UFICProceduralTexture::ShouldSave_Implementation() const {
	return true;
}

UTexture2D* UFICProceduralTexture::GetTexture() {
	if (Data.Num() == 0) return nullptr;
	
	if (!Texture) {
		Texture = UTexture2D::CreateTransient(Size.X, Size.Y, EPixelFormat::PF_R8G8B8A8);

		ReloadData();
	}
	
	return Texture;
}

void UFICProceduralTexture::ReloadData() {
	if (Texture == nullptr) return;

	if (Data.Num() == 0) return;

	uint64 RawSize = Size.X * Size.Y * 4;
	check(Data.Num() >= RawSize);

	Texture->GetPlatformData()->SizeX = Size.X;
	Texture->GetPlatformData()->SizeY = Size.Y;
	
	FTexture2DMipMap& PrimaryMipMap = Texture->GetPlatformData()->Mips[0];
	PrimaryMipMap.BulkData.Lock(LOCK_READ_WRITE);
	void* TextureDataPtr = PrimaryMipMap.BulkData.Realloc(Data.Num());
	FMemory::Memcpy(TextureDataPtr, Data.GetData(), Data.Num());
	PrimaryMipMap.BulkData.Unlock();

	FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		Texture->UpdateResource();
		OnTextureUpdate.Broadcast();
	}, TStatId(), nullptr, ENamedThreads::GameThread);
}

void UFICProceduralTexture::SetData(const TArray<uint8>& InData, const FIntPoint& InSize) {
	SetData((const TArrayView<uint8>&)InData, InSize);
}

void UFICProceduralTexture::SetData(const TArrayView<uint8>& InData, const FIntPoint& InSize) {
	uint64 RawSize = Size.X * Size.Y * 4;
	check(Data.Num() >= RawSize);
	
	Data = InData;
	Size = InSize;
	
	ReloadData();
}

FSequenceExporterProceduralTexture::FSequenceExporterProceduralTexture(UFICProceduralTexture* Texture) : Texture(Texture) {}

bool FSequenceExporterProceduralTexture::Init() {
	return true;
}

void FSequenceExporterProceduralTexture::AddFrame(EPixelFormat Format, void* ptr, FIntPoint ReadSize, FIntPoint Size) {
	TArray<FColor> SrcData;
	SrcData.AddUninitialized(Size.X * Size.Y * sizeof(FColor));
	for (int Y = 0; Y < Size.Y; ++Y) {
		FMemory::Memcpy( SrcData.GetData() + (Size.X * Y), ((FColor*)ptr) + (ReadSize.X * Y), Size.X * sizeof(FColor));
	}
	//FIntPoint DstSize((int)((double)Size.X * (360.0 / (double)Size.Y)), 360);
	FIntPoint DstSize(640, 360);
	TArray<FColor> DstData;
	DstData.SetNumUninitialized(DstSize.X * DstSize.Y);
	FImageUtils::ImageResize(Size.X, Size.Y, SrcData, DstSize.X, DstSize.Y, DstData, false);
	Texture->SetData(TArrayView<uint8>((uint8*)DstData.GetData(), DstData.Num() * sizeof(FColor)), DstSize);
	Texture->GetTexture()->SRGB = true;
}

void FSequenceExporterProceduralTexture::Finish() {
	FSequenceExporter::Finish();
}
