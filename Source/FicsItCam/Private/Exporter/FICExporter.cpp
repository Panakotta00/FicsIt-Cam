#include "Exporter/FICExporter.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Data/FICScene.h"

void UFICExporterSpecificSettingsWidget::SetSettings(const FFICExportSettings& InExportSettings) {
	check(InExportSettings.Exporter->IsChildOf(Exporter))
	SpecificsFromExporterSettings(InExportSettings);
}

FFICExportSettings UFICExporterSpecificSettingsWidget::ApplySpecificSettings(FFICExportSettings ExportSettings, bool& bOutSuccess) {
	ExportSettings.Exporter = Exporter;
	FFICExportSettings NewExportSettings = ApplySpecificsToExporterSettings(ExportSettings, bOutSuccess);
	return NewExportSettings;
}

FFICExportSettings UFICExporterDescriptor::CreateExportSettings(TSubclassOf<UFICExporterDescriptor> Exporter, UObject* Context) {
	FFICExportSettings Settings;

	if (AFICScene* Scene = Cast<AFICScene>(Context)) {
		Settings.Resolution.X = Scene->ResolutionWidth;
		Settings.Resolution.Y = Scene->ResolutionHeight;
	}
		
	SetExporterOfExportSettings(Exporter, Settings, Context);
	return Settings;
}

TArray<TSubclassOf<UFICExporterDescriptor>> UFICExporterDescriptor::GetAvailableExporters(UObject* WorldContext) {
	TArray<TSubclassOf<UFICExporterDescriptor>> Exporters;

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByClass(UBlueprintGeneratedClass::StaticClass()->GetClassPathName(), Assets, true);
	for (const FAssetData& Asset : Assets) {
		UObject* Object = Asset.GetAsset();
		UBlueprintGeneratedClass* Exporter = Cast<UBlueprintGeneratedClass>(Object);
		if (!Exporter || !Exporter->IsChildOf<UFICExporterDescriptor>()) continue;
		if (!Cast<UFICExporterDescriptor>(Exporter->ClassDefaultObject)->bFinal) continue;
		Exporters.Add(Exporter);
	}
	return Exporters;
}
