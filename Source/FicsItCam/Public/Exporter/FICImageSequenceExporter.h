#pragma once

#include "CoreMinimal.h"
#include "FICExporter.h"
#include "FICImageSequenceExporter.generated.h"

UCLASS()
class UFICImageSequenceExporter : public UFICExporterDescriptor {
	GENERATED_BODY()
};

class FFICImageSequenceExporter : public FFICSequenceExporter {
private:
	FString Path;
	FIntPoint ImageSize;
	FDateTime StartTime;
	uint64 Increment = 0;

public:
	FFICImageSequenceExporter(FString InPath, FIntPoint InImageSize);

	virtual bool Init() override;
	virtual void AddFrame(EPixelFormat Format, void* ptr, FIntPoint ReadSize, FIntPoint Size) override;
	virtual void Finish() override;
};
