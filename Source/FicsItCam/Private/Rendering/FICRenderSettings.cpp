#include "Rendering/FICRenderSettings.h"

#include "Exporter/FICExporter.h"

bool FFICExportSettings::IsValid() const {
	return Exporter != nullptr
	&& ExporterSpecificSettings->IsValid(*this)
	&& UFICExporterDescriptor::CreateExportSettings(Exporter, nullptr).ExporterSpecificSettings.GetStruct() == ExporterSpecificSettings.GetStruct()
	&& Resolution.X > 0 && Resolution.X % 2 == 0
	&& Resolution.Y > 0 && Resolution.Y % 2 == 0;
}

bool FFICExportSettings::Serialize(FStructuredArchive::FSlot Slot) {
	FStructuredArchive::FRecord Record = Slot.EnterRecord();
	StaticStruct()->SerializeBin(Record.EnterField(TEXT("Struct")), this);

	Record << SA_VALUE(TEXT("ExporterSpecificSettings"), ExporterSpecificSettings);

	return true;
}

void FFICExportSettings::AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) const {
	ReferenceCollector.AddPropertyReferences(StaticStruct(), (void*)this);
	ExporterSpecificSettings.AddStructReferencedObjects(ReferenceCollector);
}
