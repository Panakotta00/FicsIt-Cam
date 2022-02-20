#include "Data/Attributes/FICAttributeRotation.h"

#include "Editor/FICEditorContext.h"
#include "Editor/Data/FICEditorAttributeGroup.h"
#include "Editor/UI/FICKeyframeControl.h"
#include "Editor/UI/FICVectorEditor.h"

TSharedRef<FFICEditorAttributeBase> FFICAttributeRotation::CreateEditorAttribute() {
	TSharedRef<FFICEditorAttributeBase> Base = Super::CreateEditorAttribute();
	Base->Get<TFICEditorAttribute<FFICFloatAttribute>>("Pitch").GraphColor = FColor::Red;
    Base->Get<TFICEditorAttribute<FFICFloatAttribute>>("Yaw").GraphColor = FColor::Green;
    Base->Get<TFICEditorAttribute<FFICFloatAttribute>>("Roll").GraphColor = FColor::Blue;
	StaticCastSharedRef<FFICEditorAttributeGroup>(Base)->OnCreateAttributeDetailsWidget.BindLambda([Base](UFICEditorContext* Context) {
		return SNew(SHorizontalBox)
		+SHorizontalBox::Slot().Padding(5).AutoWidth()[
			SNew(SFICKeyframeControl, Context, Base)
			.Frame_Lambda([Context]() {
				return Context->GetCurrentFrame();
			})
		]
		+SHorizontalBox::Slot().Padding(5).AutoWidth()[
			SNew(STextBlock)
			.Text(FText::FromString(":"))
		]
		+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
			SNew(SFICVectorEditor, Context, Base->GetRef<TFICEditorAttribute<FFICFloatAttribute>>("Pitch"), Base->GetRef<TFICEditorAttribute<FFICFloatAttribute>>("Yaw"), Base->GetRef<TFICEditorAttribute<FFICFloatAttribute>>("Roll"))
			.ShowKeyframeControls(true)
			.Frame_Lambda([Context]() {
				return Context->GetCurrentFrame();
			})
		];
	});
	return Base;
}

FRotator FFICAttributeRotation::FromEditorAttribute(FFICEditorAttributeGroup& Attribute) {
	return FRotator(
		Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Pitch").GetValue(),
		Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Yaw").GetValue(),
		Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Roll").GetValue()
	);
}

void FFICAttributeRotation::ToEditorAttribute(const FRotator& Rotator, FFICEditorAttributeGroup& Attribute) {
	Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Pitch").SetValue(Rotator.Pitch);
	Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Yaw").SetValue(Rotator.Yaw);
	Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Roll").SetValue(Rotator.Roll);
}
