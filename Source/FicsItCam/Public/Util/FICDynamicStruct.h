#pragma once

#include "CoreMinimal.h"
#include "FICDynamicStruct.generated.h"

#define MakeDynamicStruct(Type, ...) MakeShared<FFICDynamicStruct>(TBaseStructure<Type>::Get(), new Type{__VA_ARGS__})

template<typename T>
class TFICDynamicStruct;

/**
 * This structure allows you to store any kind of UStruct
 */
USTRUCT(BlueprintType)
struct FICSITCAM_API FFICDynamicStruct {
	GENERATED_BODY()
	
protected:
	void* Data = nullptr;
	UScriptStruct* Struct = nullptr;

public:
	FFICDynamicStruct();
	FFICDynamicStruct(UScriptStruct* Struct);
	FFICDynamicStruct(UScriptStruct* Struct, void* Data);
	FFICDynamicStruct(const FFICDynamicStruct& Other);
	~FFICDynamicStruct();
	FFICDynamicStruct& operator=(const FFICDynamicStruct& Other);

	template<typename T>
	explicit FFICDynamicStruct(const T& Struct) : FFICDynamicStruct(Copy(TBaseStructure<T>::Get(), &Struct)) {}

	static FFICDynamicStruct Copy(UScriptStruct* Struct, const void* Data);
	
	bool Serialize(FArchive& Ar);
	bool NetSerialize( FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	void AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) const;

	/**
	 * Returns the struct type stored in this holder.
	 *
	 * @return the stored structs type
	 */
	UScriptStruct* GetStruct() const;
	
	void* GetData() const;

	template<typename T>
    T& Get() const {
		return *static_cast<T*>(GetData());
	}

	template<typename T>
	T* GetPtr() const {
		if (!Struct->IsChildOf(TBaseStructure<T>::Get())) return nullptr;
		return &Get<T>();
	}

	template<typename T>
	TSharedPtr<T> SharedCopy() const {
		if (Struct->IsChildOf(TBaseStructure<T>::Get())) {
			void* Data = FMemory::Malloc(Struct->GetStructureSize());
			Struct->InitializeStruct(Data);
			Struct->CopyScriptStruct(Data, this->Data);
			return MakeShareable(reinterpret_cast<T*>(Data));
		}
		return nullptr;
	}

	template<typename T>
	operator TFICDynamicStruct<T>() const {
		return TFICDynamicStruct<T>(*this);
	}
};

template<>
struct TStructOpsTypeTraits<FFICDynamicStruct> : public TStructOpsTypeTraitsBase2<FFICDynamicStruct>
{
	enum
	{
		WithSerializer = true,
		WithNetSerializer = true,
		WithAddStructReferencedObjects = true,
        WithCopy = true,
    };
};

inline void operator<<(FArchive& Ar, FFICDynamicStruct& Struct) {
	Struct.Serialize(Ar);
}

template<typename T>
class TFICDynamicStruct : public FFICDynamicStruct {
public:
	TFICDynamicStruct() : FFICDynamicStruct(TBaseStructure<T>::Get()) {}
	TFICDynamicStruct(UScriptStruct* Struct) : FFICDynamicStruct(Struct) { check(Struct->IsChildOf(TBaseStructure<T>::Get())) }
	TFICDynamicStruct(UScriptStruct* Struct, void* Data) : FFICDynamicStruct(Struct, Data) { check(Struct->IsChildOf(TBaseStructure<T>::Get())) }
	template<typename K>
	TFICDynamicStruct(const TFICDynamicStruct<K>& Other) : FFICDynamicStruct(FFICDynamicStruct::Copy(Other.GetStruct(), Other.GetData())) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
	}
	TFICDynamicStruct(const FFICDynamicStruct& Other) : FFICDynamicStruct(FFICDynamicStruct::Copy(Other.GetStruct(), Other.GetData())) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
	}
	template<typename K>
    explicit TFICDynamicStruct(const K& Other) : FFICDynamicStruct(FFICDynamicStruct::Copy(TBaseStructure<K>::Get(), &Other)) {
		check(TBaseStructure<K>::Get()->IsChildOf(TBaseStructure<T>::Get()));
	}

	template<typename K>
	TFICDynamicStruct<T>& operator=(const TFICDynamicStruct<K>& Other) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
		FFICDynamicStruct::operator=(Other);
		return *this;
	}
	
	TFICDynamicStruct<T>& operator=(const FFICDynamicStruct& Other) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
		FFICDynamicStruct::operator=(Other);
		return *this;
	}
	
	T* operator->() const {
		return &Get<T>();
	}

	T* operator*() const {
		return &Get<T>();
	}

    TSharedPtr<T> SharedCopy() {
		return FFICDynamicStruct::SharedCopy<T>();
	}

	operator FFICDynamicStruct() const {
		return FFICDynamicStruct::Copy(Struct, Data);
	}
};

