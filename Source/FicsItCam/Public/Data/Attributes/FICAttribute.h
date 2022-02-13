#pragma once

#include "FICKeyframe.h"
#include "FICAttribute.generated.h"

USTRUCT(BlueprintType)
struct FFICAttribute {
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE(FOnUpdate)
	FOnUpdate OnUpdate;

	virtual ~FFICAttribute() = default;
	
	virtual FName GetAttributeType() { checkf(false, TEXT("Not Implemented!")); return FName(); }
	
	virtual EFICKeyframeType GetAllowedKeyframeTypes() const { return FIC_KF_NONE; }
	virtual TMap<FICFrame, TSharedRef<FFICKeyframe>> GetKeyframes() { checkf(false, TEXT("Not Implemented!")); return TMap<FICFrame, TSharedRef<FFICKeyframe>>(); }
	virtual TSharedRef<FFICKeyframe> AddKeyframe(FICFrame Time) { checkf(false, TEXT("Not Implemented!")); return MakeShareable<FFICKeyframe>(nullptr); }
	virtual void RemoveKeyframe(FICFrame Time) { checkf(false, TEXT("Not Implemented!")); }
	virtual void MoveKeyframe(FICFrame From, FICFrame To) { checkf(false, TEXT("Not Implemented!")); }
	virtual void RecalculateKeyframe(FICFrame Time) { checkf(false, TEXT("Not Implemented!")); }
	
	virtual void Set(TSharedRef<FFICAttribute> InAttrib) { checkf(false, TEXT("Not Implemented!")); }
	virtual TSharedRef<FFICAttribute> Get() { checkf(false, TEXT("Not Implemented!")); return MakeShareable<FFICAttribute>(nullptr); }

	void RecalculateAllKeyframes();

	// TODO: Use Binary-Search
	TSharedPtr<FFICKeyframe> GetNextKeyframe(FICFrame Time, FICFrame& OutTime);
	TSharedPtr<FFICKeyframe> GetPrevKeyframe(FICFrame Time, FICFrame& OutTime);
};

