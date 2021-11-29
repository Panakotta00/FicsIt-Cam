#include "FICUtils.h"

#include "FGGameUserSettings.h"
#include "GameFramework/InputSettings.h"

FRotator UFICUtils::NormalizeRotator(FRotator Rot) {
	while (Rot.Pitch > 180.0) Rot.Pitch -= 360.0f;
	while (Rot.Pitch < -180.0) Rot.Pitch += 360.0f;
	while (Rot.Yaw > 180.0) Rot.Yaw -= 360.0f;
	while (Rot.Yaw < -180.0) Rot.Yaw += 360.0f;
	while (Rot.Roll > 180.0) Rot.Roll -= 360.0f;
	while (Rot.Roll < -180.0) Rot.Roll += 360.0f;
	return Rot;
}

bool UFICUtils::SaveRenderTargetAsJPG(const FString& FilePath, UTextureRenderTarget2D* RenderTarget) {
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

FString UFICUtils::KeymappingToString(const FString& Keymapping) {
	TArray<FInputActionKeyMapping> Mappings;
	UInputSettings::GetInputSettings()->GetActionMappingByName(FName(Keymapping), Mappings);
	if (Mappings.Num() > 0) {
		FInputActionKeyMapping Map = Mappings[0];
		FString Str;
		if (Map.bCtrl) Str += TEXT("CTRL + ");
		if (Map.bShift) Str += TEXT("SHIFT + ");
		if (Map.bAlt) Str += TEXT("ALT + ");
		Str += Map.Key.GetDisplayName().ToString();
		return Str;
	}
	return TEXT("");
}
