#pragma once

#include "FICKeyframe.h"
#include "Editor/Data/FICEditorAttributeBase.h"
#include "FICAttribute.generated.h"

USTRUCT(BlueprintType)
struct FFICAttribute {
	GENERATED_BODY()

private:
	int UpdateLocks = 0;

protected:
	void OnUpdateBroadcast() {
		if (UpdateLocks == 0) OnUpdate.Broadcast(); 
	}
	
public:
	DECLARE_MULTICAST_DELEGATE(FOnUpdate)
	FOnUpdate OnUpdate;

	void LockUpdateEvent() { ++UpdateLocks; }
	void UnlockUpdateEvent(bool bBroadcastIfFullUnlock = true) {
		--UpdateLocks;
		if (UpdateLocks < 1) {
			UpdateLocks = 0;
			if (bBroadcastIfFullUnlock) OnUpdate.Broadcast();
		}
	}

	virtual ~FFICAttribute() = default;
	
	virtual FName GetAttributeType() const { checkf(false, TEXT("Not Implemented!")); return FName(); }
	
	virtual EFICKeyframeType GetAllowedKeyframeTypes() const { return FIC_KF_NONE; }
	virtual TMap<FICFrame, TSharedRef<FFICKeyframe>> GetKeyframes() { checkf(false, TEXT("Not Implemented!")); return TMap<FICFrame, TSharedRef<FFICKeyframe>>(); }
	virtual TSharedRef<FFICKeyframe> AddKeyframe(FICFrame Time) { checkf(false, TEXT("Not Implemented!")); return MakeShareable<FFICKeyframe>(nullptr); }
	virtual void RemoveKeyframe(FICFrame Time) { checkf(false, TEXT("Not Implemented!")); }
	virtual void MoveKeyframe(FICFrame From, FICFrame To) { checkf(false, TEXT("Not Implemented!")); }
	virtual void RecalculateKeyframe(FICFrame Time) { checkf(false, TEXT("Not Implemented!")); }
	virtual FICValue GetFloatValue(FICFrameFloat Time) { checkf(false, TEXT("Not Implemented!")); return 0.0f; }
	virtual bool HasKeyframe(FICFrame Time) const { checkf(false, TEXT("Not Implemented!")); return false; }
	
	virtual void Set(TSharedRef<FFICAttribute> InAttrib) { checkf(false, TEXT("Not Implemented!")); }
	virtual TSharedRef<FFICAttribute> Get() { checkf(false, TEXT("Not Implemented!")); return MakeShareable<FFICAttribute>(nullptr); }

	virtual TSharedRef<FFICEditorAttributeBase> CreateEditorAttribute() { checkf(false, TEXT("Not Implemented!")); return MakeShareable<FFICEditorAttributeBase>(nullptr); }

	virtual const TMap<FString, FFICAttribute*>& GetChildAttributes() const {
		static TMap<FString, FFICAttribute*> Keyframes;
		return Keyframes;
	}

	void RecalculateAllKeyframes();

	// TODO: Use Binary-Search
	TSharedPtr<FFICKeyframe> GetNextKeyframe(FICFrame Time, FICFrame& OutTime);
	TSharedPtr<FFICKeyframe> GetPrevKeyframe(FICFrame Time, FICFrame& OutTime);
};

