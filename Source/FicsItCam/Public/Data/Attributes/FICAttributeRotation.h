#pragma once

#include "FICAttribute.h"
#include "FICAttributeFloat.h"
#include "FICAttributeGroup.h"
#include "FICAttributeRotation.generated.h"

class FFICEditorAttributeGroup;

USTRUCT(BlueprintType)
struct FFICAttributeRotation : public FFICGroupAttribute {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FFICFloatAttribute Pitch;
	
	UPROPERTY(SaveGame)
	FFICFloatAttribute Yaw;

	UPROPERTY(SaveGame)
	FFICFloatAttribute Roll;

	FFICAttributeRotation() {
		AddChildAttribute("Pitch", &Pitch);
		AddChildAttribute("Yaw", &Yaw);
		AddChildAttribute("Roll", &Roll);
	}

	// Begin FFICAttribute
	virtual TSharedRef<FFICEditorAttributeBase> CreateEditorAttribute() override;
	// End FFICAttribute

	FRotator Get(FICFrameFloat Frame) {
		return FRotator(
			Pitch.GetValue(Frame),
			Yaw.GetValue(Frame),
			Roll.GetValue(Frame)
		);
	}

	void SetDefaultValue(const FRotator& Rot) {
		Pitch.SetDefaultValue(Rot.Pitch);
		Yaw.SetDefaultValue(Rot.Yaw);
		Roll.SetDefaultValue(Rot.Roll);
	}

	static FRotator FromEditorAttribute(FFICEditorAttributeGroup& Attribute);
	static void ToEditorAttribute(const FRotator& Rotator, FFICEditorAttributeGroup& Attribute);
};