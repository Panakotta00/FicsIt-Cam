#pragma once

#include "CoreMinimal.h"
#include "FICAttribute.h"
#include "FICAttributeFloat.h"
#include "FICAttributeGroup.h"
#include "FICAttributePosition.generated.h"

class FFICEditorAttributeGroup;

USTRUCT(BlueprintType)
struct FFICAttributePosition : public FFICGroupAttribute {
	GENERATED_BODY()
public:
	inline static const FName TypeName = FName(TEXT("PositionAttribute"));
	
	UPROPERTY(SaveGame)
	FFICFloatAttribute X;
	
	UPROPERTY(SaveGame)
	FFICFloatAttribute Y;

	UPROPERTY(SaveGame)
	FFICFloatAttribute Z;

	FFICAttributePosition() {
		AddChildAttribute("X", &X);
		AddChildAttribute("Y", &Y);
		AddChildAttribute("Z", &Z);
	}

	// Begin FFICAttribute
	virtual FName GetAttributeType() const override { return TypeName; }
	virtual TSharedRef<FFICEditorAttributeBase> CreateEditorAttribute() override;
	// End FFICAttribute

	FVector Get(FICFrameFloat Frame) {
		return FVector(
			X.GetValue(Frame),
			Y.GetValue(Frame),
			Z.GetValue(Frame)
		);
	}

	void SetDefaultValue(const FVector& Pos) {
		X.SetDefaultValue(Pos.X);
		Y.SetDefaultValue(Pos.Y);
		Z.SetDefaultValue(Pos.Z);
	}

	static FVector FromEditorAttribute(FFICEditorAttributeGroup& Attribute);
	static FVector FromEditorAttribute(FFICEditorAttributeGroup& Attribute, FICFrameFloat Time);
	static void ToEditorAttribute(const FVector& Vector, FFICEditorAttributeGroup& Attribute);
};
