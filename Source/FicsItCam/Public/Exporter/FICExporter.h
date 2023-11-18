#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Rendering/FICRenderSettings.h"
#include "FICExporter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFICExporterSpecificSettingsUpdated);

class FFICSequenceExporter {
protected:
	bool bFinished = false;
	
public:
	virtual ~FFICSequenceExporter() = default;
	virtual bool Init() = 0;
	virtual void AddFrame(EPixelFormat Format, void* ptr, FIntPoint ReadSize, FIntPoint Size) = 0;
	virtual void Finish() {
		bFinished = true;
	};
};

UCLASS(Blueprintable)
class UFICExporterSpecificSettingsWidget : public UUserWidget {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FFICExporterSpecificSettingsUpdated OnUpdate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UFICExporterDescriptor> Exporter;

	UFUNCTION(BlueprintCallable)
	void SetSettings(const FFICExportSettings& InExportSettings);

	UFUNCTION(BlueprintCallable)
	FFICExportSettings ApplySpecificSettings(FFICExportSettings ExportSettings, bool& bOutSuccess);
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void SpecificsFromExporterSettings(const FFICExportSettings& ExporterSettings);
	
	UFUNCTION(BlueprintImplementableEvent)
	FFICExportSettings ApplySpecificsToExporterSettings(const FFICExportSettings& ExportSettings, bool& bOutSuccess);
};

UCLASS(Abstract, Blueprintable)
class UFICExporterDescriptor : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bFinal = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText ExporterName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UFICExporterSpecificSettingsWidget> SpecificSettingsWidgetType = nullptr;

	UFUNCTION(BlueprintCallable)
	static FFICExportSettings CreateExportSettings(TSubclassOf<UFICExporterDescriptor> Exporter, UObject* Context);

	UFUNCTION(BlueprintCallable)
	static void SetExporterOfExportSettings(TSubclassOf<UFICExporterDescriptor> Exporter, UPARAM(ref) FFICExportSettings& InExportSettings, UObject* Context) {
		InExportSettings.Exporter = Exporter;
		if (Exporter) InExportSettings.ExporterSpecificSettings = Cast<UFICExporterDescriptor>(Exporter->ClassDefaultObject)->CreateSpecificSettings(Context);
	}
	virtual TFICDynamicStruct<FFICExporterSpecificSettings> CreateSpecificSettings(UObject* Context) {
		return TFICDynamicStruct<FFICExporterSpecificSettings>();
	}

	static TSharedRef<FFICSequenceExporter> CreateExporter(TSubclassOf<UFICExporterDescriptor> Exporter, const FFICExportSettings& InExportSettings) {
		check(InExportSettings.IsValid())
		return Cast<UFICExporterDescriptor>(Exporter->ClassDefaultObject)->CreateExporter(InExportSettings);
	}
	virtual TSharedRef<FFICSequenceExporter> CreateExporter(const FFICExportSettings& InSettings) {
		check(false);
		return TSharedRef<FFICSequenceExporter>(static_cast<FFICSequenceExporter*>(nullptr));
	}

	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContext"))
	static TArray<TSubclassOf<UFICExporterDescriptor>> GetAvailableExporters(UObject* WorldContext);

	UFUNCTION(BlueprintCallable)
	static bool AreExportSettingsValid(const FFICExportSettings& InExportSettings) { return InExportSettings.IsValid(); }
};
