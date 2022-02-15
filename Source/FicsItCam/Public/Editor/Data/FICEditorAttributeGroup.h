#pragma once

#include "FICEditorAttributeBase.h"

struct FFICGroupAttribute;

class FFICEditorAttributeGroupBase : public FFICEditorAttributeBase {
protected:
	TMap<FString, TSharedRef<FFICEditorAttributeBase>> Attributes;
	
public:
	DECLARE_DELEGATE_RetVal_OneParam(TSharedRef<SWidget>, FFICCreateAttributeDetailsWidget, UFICEditorContext*)

	FFICCreateAttributeDetailsWidget OnCreateAttributeDetailsWidget;
	
	FFICEditorAttributeGroupBase(FFICAttributeValueChanged OnValueChanged = FFICAttributeValueChanged(), FLinearColor GraphColor = FColor::White);

	// Begin FFICEditorAttributeBase
	virtual void SetKeyframe(FICFrame Time) override;
	virtual void RemoveKeyframe(FICFrame Time) override;
	virtual bool HasChanged(FICFrame Time) const override;
	virtual void UpdateValue(FICFrame Time) override;
	virtual float GetValue(FICFrame InFrame) const override;
	virtual void SetKeyframe(FFICValueTime InValueFrame, EFICKeyframeType InType = FIC_KF_EASE, bool bCreate = true) override;
	virtual TMap<FString, TSharedRef<FFICEditorAttributeBase>> GetChildAttributes() override;
	virtual TSharedRef<SWidget> CreateDetailsWidget(UFICEditorContext* Context) override;
	// End FFICEditorAttributeBase
};

class FFICEditorAttributeGroup : public FFICEditorAttributeGroupBase {
private:
	FFICGroupAttribute& GroupAttribute;

public:
	FFICEditorAttributeGroup(FFICGroupAttribute& InGroupAttribute, FFICAttributeValueChanged OnValueChanged = FFICAttributeValueChanged(), FLinearColor GraphColor = FColor::White);

	// Begin FFICEditorAttributeBase
	virtual const FFICAttribute& GetAttributeConst() const override;
	// End FFICEditorAttributeBases
};