#pragma once

#include "FICEditorAttributeGroup.h"
#include "Data/Attributes/FICAttributeGroup.h"

class FFICEditorAttributeGroupDynamic : public FFICEditorAttributeGroupBase {
private:
	FFICGroupAttribute GroupAttribute;

public:
	FFICEditorAttributeGroupDynamic(TMap<FString, TSharedRef<FFICEditorAttributeBase>> InAttributes = {}, FFICAttributeValueChanged OnValueChanged = FFICAttributeValueChanged(), FLinearColor GraphColor = FColor::White);

	// Begin FFICEditorAttributeBase
	virtual const FFICAttribute& GetAttributeConst() const override;
	// End FFICEditorAttributeBase

	void AddAttribute(FString InName, TSharedRef<FFICEditorAttributeBase> InAttribute);
};
