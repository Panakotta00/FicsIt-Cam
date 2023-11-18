#pragma once

#include "CoreMinimal.h"
#include "Util/FICDynamicStruct.h"
#include "Util/FICStructOnScope.h"
#include "FICREnderSettings.generated.h"

class UFICExporterDescriptor;

UENUM()
enum EFICExporterType {
	FIC_Exporter_ImageSequence,
	FIC_Exporter_MP4,
};

USTRUCT(BlueprintType)
struct FFICRenderSettings {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FIntPoint Resolution = FIntPoint(1920, 1080);
};

USTRUCT()
struct FFICExporterSpecificSettings {
	GENERATED_BODY()

	virtual ~FFICExporterSpecificSettings() = default;

	virtual bool IsValid(const struct FFICExportSettings& ExportSettings) const {
		return false;
	}
};

USTRUCT(BlueprintType)
struct FFICExportSettings : public FFICRenderSettings {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, SaveGame)
	TSubclassOf<UFICExporterDescriptor> Exporter;
	
	TFICDynamicStruct<FFICExporterSpecificSettings> ExporterSpecificSettings;

	bool IsValid() const;
	
	bool Serialize(FStructuredArchive::FSlot Slot);
	void AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) const;
};

template<>
struct TStructOpsTypeTraits<FFICExportSettings> : TStructOpsTypeTraitsBase2<FFICExportSettings> {
	enum {
		WithStructuredSerializer = true,
		WithAddStructReferencedObjects = true,
	};
};
