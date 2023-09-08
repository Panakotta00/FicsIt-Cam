#include "Util/FICProceduralTexture.h"

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
	check(Data.Num() == RawSize);

	Texture->GetPlatformData()->SizeX = Size.X;
	Texture->GetPlatformData()->SizeY = Size.Y;
	
	FTexture2DMipMap& PrimaryMipMap = Texture->GetPlatformData()->Mips[0];
	void* TextureDataPtr = PrimaryMipMap.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureDataPtr, Data.GetData(), Data.Num());
	PrimaryMipMap.BulkData.Unlock();

	FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
		Texture->UpdateResource();
		OnTextureUpdate.Broadcast();
	}, TStatId(), nullptr, ENamedThreads::GameThread);
}

void UFICProceduralTexture::SetData(const TArray<uint8>& InData, const FIntPoint& InSize) {
	uint64 RawSize = Size.X * Size.Y * 4;
	check(Data.Num() == RawSize);
	
	Data = InData;
	Size = InSize;
	
	ReloadData();
}

FSequenceExporterProceduralTexture::FSequenceExporterProceduralTexture(UFICProceduralTexture* Texture) : Texture(Texture) {}

bool FSequenceExporterProceduralTexture::Init() {
	return true;
}

void FSequenceExporterProceduralTexture::AddFrame(void* ptr, FIntPoint ReadSize, FIntPoint Size) {
	TArray<uint8> Data((uint8*)ptr, ReadSize.X * ReadSize.Y * 4);
	Texture->SetData(Data, ReadSize);
}

void FSequenceExporterProceduralTexture::Finish() {
	FSequenceExporter::Finish();
}
