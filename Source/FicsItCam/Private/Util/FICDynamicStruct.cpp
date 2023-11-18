#include "Util/FICDynamicStruct.h"

FFICDynamicStruct::FFICDynamicStruct() {}

FFICDynamicStruct::FFICDynamicStruct(UScriptStruct* Struct) : Struct(Struct) {
	Data = FMemory::Malloc(Struct->GetStructureSize());
	Struct->InitializeStruct(Data);
}

FFICDynamicStruct::FFICDynamicStruct(UScriptStruct* Struct, void* Data) : Data(Data), Struct(Struct) {}

FFICDynamicStruct::FFICDynamicStruct(const FFICDynamicStruct& Other) {
	*this = Other;
}

FFICDynamicStruct::~FFICDynamicStruct() {
	if (Data) {
		Struct->DestroyStruct(Data);
		FMemory::Free(Data);
		Data = nullptr;
	}
}

FFICDynamicStruct& FFICDynamicStruct::operator=(const FFICDynamicStruct& Other) {
	if (Data) {
		Struct->DestroyStruct(Data);
		if (Other.Data) {
			Data = FMemory::Realloc(Data, Other.Struct->GetStructureSize());
		} else {
			FMemory::Free(Data);
			Data = nullptr;
		}
	} else {
		if (Other.Data) {
			Data = FMemory::Malloc(Other.Struct->GetStructureSize());
		}
	}
	Struct = Other.Struct;
	if (Data) {
		Struct->InitializeStruct(Data);
		Struct->CopyScriptStruct(Data, Other.Data);
	
	}
	return *this;
}

FFICDynamicStruct FFICDynamicStruct::Copy(UScriptStruct* Struct, const void* Data) {
	FFICDynamicStruct holder(Struct);
	if (Data) Struct->CopyScriptStruct(holder.Data, Data);
	return holder;
}

bool FFICDynamicStruct::Serialize(FArchive& Ar) {
	UScriptStruct* OldStruct = Struct;
	// TODO: TObjectPtr<UObject> ObjectPtr; ????
	Ar << Struct;
	
	if (Ar.IsLoading()) {
		if (Data) {
			if (OldStruct) OldStruct->DestroyStruct(Data);
			if (Struct) {
				Data = FMemory::Realloc(Data, Struct->GetStructureSize());
			} else {
				FMemory::Free(Data);
				Data = nullptr;
			}
		} else if (Struct) {
			Data = FMemory::Malloc(Struct->GetStructureSize());
		}
		if (Struct) Struct->InitializeStruct(Data);
	}
	if (Struct) {
		Struct->SerializeBin(Ar, Data);
	}
	return true;
}

bool FFICDynamicStruct::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) {
	bOutSuccess = Serialize(Ar);
	return bOutSuccess;
}

void FFICDynamicStruct::AddStructReferencedObjects(FReferenceCollector& Collector) const {
	UScriptStruct* ThisStruct = Struct;
	if (Struct) Collector.AddReferencedObject(ThisStruct);
	if (Struct && Data) {
		if (Struct->GetCppStructOps()->HasAddStructReferencedObjects()) Struct->GetCppStructOps()->AddStructReferencedObjects()(Data, Collector);
	}
}

UScriptStruct* FFICDynamicStruct::GetStruct() const {
	return Struct;
}

void* FFICDynamicStruct::GetData() const {
	return Data;
}
