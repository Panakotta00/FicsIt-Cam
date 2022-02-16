#pragma once

#include "FICEditorAttributeBase.h"

struct FFICAttributeBool;

class FFICEditorAttributeBool : public FFICEditorAttributeBase {
private:
	FFICAttributeBool* Attribute;
	bool ActiveValue = false;
	
public:
	FFICEditorAttributeBool(FFICAttributeBool* InAttribute, FLinearColor GraphColor = FColor::White);

	// Begin FFICEditorAttributeBase
	virtual void SetKeyframe(FICFrame Time) override;
	virtual void RemoveKeyframe(FICFrame Time) override;
	virtual bool HasChanged(FICFrame Time) const override;
	virtual const FFICAttribute& GetAttributeConst() const override;
	virtual void UpdateValue(FICFrame Time) override;
	virtual FICValue GetValue(FICFrame InFrame) const override;
	virtual void SetKeyframe(FFICValueTime InValueFrame, EFICKeyframeType InType = FIC_KF_EASE, bool bCreate = true) override;
	virtual TMap<FString, TSharedRef<FFICEditorAttributeBase>> GetChildAttributes() override;
	virtual TSharedRef<SWidget> CreateDetailsWidget(UFICEditorContext* Context) override;
	// End FFICEditorAttributeBase

	void SetActiveValue(bool InValue);
	bool GetActiveValue() const;
};
