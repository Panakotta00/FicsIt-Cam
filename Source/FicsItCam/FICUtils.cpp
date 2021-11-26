#include "FICUtils.h"

FRotator NormalizeRotator(FRotator Rot) {
	while (Rot.Pitch > 180.0) Rot.Pitch -= 360.0f;
	while (Rot.Pitch < -180.0) Rot.Pitch += 360.0f;
	while (Rot.Yaw > 180.0) Rot.Yaw -= 360.0f;
	while (Rot.Yaw < -180.0) Rot.Yaw += 360.0f;
	while (Rot.Roll > 180.0) Rot.Roll -= 360.0f;
	while (Rot.Roll < -180.0) Rot.Roll += 360.0f;
	return Rot;
}

bool FIC_SaveRenderTargetAsJPG(const FString& FilePath, UTextureRenderTarget2D* RenderTarget) {
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	if (!RenderTargetResource || !ImageWrapper.IsValid()) return false;
	
	TArray<FColor> RawData;
	bool bReadPixels = RenderTargetResource->ReadPixels(RawData);
	if (!bReadPixels) return false;

	bool bRaw = ImageWrapper->SetRaw(RawData.GetData(), RawData.GetTypeSize() * RawData.Num(), RenderTarget->SizeX, RenderTarget->SizeY, ERGBFormat::BGRA, 8);
	if (!bRaw) return false;

	TArray64<uint8> CompressedData = ImageWrapper->GetCompressed();
	FFileHelper::SaveArrayToFile(CompressedData, *FilePath);
	return true;
}
