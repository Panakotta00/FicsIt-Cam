#include "Editor/Data/FICEditorAttributeGroupDynamic.h"

FFICEditorAttributeGroupDynamic::FFICEditorAttributeGroupDynamic(TMap<FString, TSharedRef<FFICEditorAttributeBase>> InAttributes, FFICAttributeValueChanged OnValueChanged,	FLinearColor GraphColor) : FFICEditorAttributeGroupBase(OnValueChanged, GraphColor) {
	Attributes = InAttributes;

	for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Attribute : Attributes) {
		GroupAttribute.AddChildAttribute(Attribute.Key, &Attribute.Value->GetAttribute());
	}
	
	GroupAttribute.OnUpdate.AddLambda([this]() {
		for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Attribute : Attributes) {
			Attribute.Value->GetAttribute().OnUpdate.Broadcast();
		}
	});
}

const FFICAttribute& FFICEditorAttributeGroupDynamic::GetAttributeConst() const {
	return GroupAttribute;
}

void FFICEditorAttributeGroupDynamic::AddAttribute(FString InName, TSharedRef<FFICEditorAttributeBase> InAttribute) {
	Attributes.Add(InName, InAttribute);
	GroupAttribute.AddChildAttribute(InName, &InAttribute->GetAttribute());
}

void FFICEditorAttributeGroupDynamic::RemoveAttribute(FString InName) {
	Attributes.Remove(InName);
	GroupAttribute.RemoveChildAttribute(InName);
}
