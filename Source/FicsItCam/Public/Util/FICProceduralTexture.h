#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "SequenceExporter.h"
#include "FICProceduralTexture.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFICTextureUpdateDelegate);

UCLASS()
class UFICProceduralTexture : public UObject, public IFGSaveInterface {
	GENERATED_BODY()
private:
	UPROPERTY()
	UTexture2D* Texture = nullptr;

	UPROPERTY(SaveGame)
	TArray<uint8> Data;

	UPROPERTY(SaveGame)
	FIntPoint Size;
	
public:
	UPROPERTY(BlueprintAssignable)
	FFICTextureUpdateDelegate OnTextureUpdate;
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
	
	UFUNCTION(BlueprintCallable)
	UTexture2D* GetTexture();

	UFUNCTION(BlueprintCallable)
	void ReloadData();

	UFUNCTION(BlueprintCallable)
	void SetData(const TArray<uint8>& InData, const FIntPoint& InSize);
	void SetData(const TArrayView<uint8>& InData, const FIntPoint& InSize);
};

class FSequenceExporterProceduralTexture : public FSequenceExporter {
private:
	UFICProceduralTexture* Texture;

public:
	FSequenceExporterProceduralTexture(UFICProceduralTexture* Texture);
	virtual bool Init() override;
	virtual void AddFrame(void* ptr, FIntPoint ReadSize, FIntPoint Size) override;
	virtual void Finish() override;
};