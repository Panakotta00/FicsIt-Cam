#include "Editor/Data/FICEditorAttributeGroup.h"

#include "Data/Attributes/FICAttributeGroup.h"

FFICEditorAttributeGroupBase::FFICEditorAttributeGroupBase(FFICAttributeValueChanged OnValueChanged, FLinearColor GraphColor): FFICEditorAttributeBase(GraphColor) {
	OnCreateAttributeDetailsWidget.BindLambda([this](UFICEditorContext* Context) {
		TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
		for (const TPair<FString, TSharedRef<FFICEditorAttributeBase>>& Attrib : Attributes) {
			List->AddSlot().AutoHeight().HAlign(HAlign_Fill)[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(5)[
					SNew(STextBlock)
					.Text(FText::FromString(Attrib.Key))
				]
				+SHorizontalBox::Slot().FillWidth(1)[
					Attrib.Value->CreateDetailsWidget(Context)
				]
			];
		}
		return List;
	});
}

void FFICEditorAttributeGroupBase::SetKeyframe(int64 Time) {
	for (const TPair<FString, TSharedRef<FFICEditorAttributeBase>>& Attrib : Attributes) {
		Attrib.Value->SetKeyframe(Time);
	}
}

void FFICEditorAttributeGroupBase::RemoveKeyframe(int64 Time) {
	for (const TPair<FString, TSharedRef<FFICEditorAttributeBase>>& Attrib : Attributes) {
		Attrib.Value->RemoveKeyframe(Time);
	} 
}

bool FFICEditorAttributeGroupBase::HasChanged(int64 Time) const {
	for (const TPair<FString, TSharedRef<FFICEditorAttributeBase>>& Attrib : Attributes) {
		if (Attrib.Value->HasChanged(Time)) return true;
	}
	return false;
}

void FFICEditorAttributeGroupBase::UpdateValue(FICFrame Time) {
	for (const TPair<FString, TSharedRef<FFICEditorAttributeBase>>& Attrib : Attributes) {
		Attrib.Value->UpdateValue(Time);
	}
}

FICValue FFICEditorAttributeGroupBase::GetValue(FICFrame InFrame) const {
	float Sum = 0.0f;
	for (const TTuple<FString, TSharedRef<FFICEditorAttributeBase>>& Attribute : Attributes) {
		Sum += Attribute.Value->GetValue(InFrame);
	}
	return Sum / Attributes.Num();
}

void FFICEditorAttributeGroupBase::SetKeyframe(FFICValueTime InValueFrame, EFICKeyframeType InType, bool bCreate) {
	for (const TTuple<FString, TSharedRef<FFICEditorAttributeBase>>& Attribute : Attributes) {
		if (!bCreate && !Attribute.Value->GetKeyframe(InValueFrame.Frame)) continue;
		Attribute.Value->SetKeyframe(InValueFrame, InType);
	}
}

TMap<FString, TSharedRef<FFICEditorAttributeBase>> FFICEditorAttributeGroupBase::GetChildAttributes() {
	return Attributes;
}

TSharedRef<SWidget> FFICEditorAttributeGroupBase::CreateDetailsWidget(UFICEditorContext* Context) {
	return OnCreateAttributeDetailsWidget.Execute(Context);
}

FFICEditorAttributeGroup::FFICEditorAttributeGroup(FFICGroupAttribute& InGroupAttribute, FFICAttributeValueChanged InOnValueChanged, FLinearColor GraphColor): FFICEditorAttributeGroupBase(InOnValueChanged, GraphColor), GroupAttribute(InGroupAttribute) {
	for (TTuple<FString, FFICAttribute*> Attribute : GroupAttribute.Children) {
		Attributes.Add(Attribute.Key, Attribute.Value->CreateEditorAttribute());
	}

	for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Attribute : Attributes) {
		Attribute.Value->OnValueChanged.AddLambda([this]() {
			OnValueChanged.Broadcast();
		});
	}
}
const FFICAttribute& FFICEditorAttributeGroup::GetAttributeConst() const {
	return GroupAttribute;
}