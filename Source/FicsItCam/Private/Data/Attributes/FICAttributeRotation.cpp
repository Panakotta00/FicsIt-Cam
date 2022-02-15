#include "Data/Attributes/FICAttributeRotation.h"

#include "Editor/FICEditorContext.h"
#include "Editor/Data/FICEditorAttributeGroup.h"
#include "Editor/UI/FICKeyframeControl.h"
#include "Editor/UI/FICVectorEditor.h"

TSharedRef<FFICEditorAttributeBase> FFICAttributeRotation::CreateEditorAttribute() {
	TSharedRef<FFICEditorAttributeBase> Base = Super::CreateEditorAttribute();
	StaticCastSharedRef<FFICEditorAttributeGroup>(Base)->OnCreateAttributeDetailsWidget.BindLambda([Base](UFICEditorContext* Context) {
		return SNew(SHorizontalBox)
		+SHorizontalBox::Slot().Padding(5).AutoWidth()[
			SNew(SFICKeyframeControl, Context)
			.Attribute_Lambda([Base]() -> FFICEditorAttributeBase* {
				return &*Base;
			})
			.Frame_Lambda([Context]() {
				return Context->GetCurrentFrame();
			})
		]
		+SHorizontalBox::Slot().Padding(5).AutoWidth()[
			SNew(STextBlock)
			.Text(FText::FromString(":"))
		]
		+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
			SNew(SFICVectorEditor, Context)
			.ShowKeyframeControls(true)
			.XAttr_Lambda([Base]() {
				return &*(*Base)["Pitch"];
			})
			.YAttr_Lambda([Base]() {
				return &*(*Base)["Yaw"];
			})
			.ZAttr_Lambda([Base]() {
				return &*(*Base)["Roll"];
			})
			.AutoKeyframe_Lambda([Context]() {
				return Context->bAutoKeyframe;
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
