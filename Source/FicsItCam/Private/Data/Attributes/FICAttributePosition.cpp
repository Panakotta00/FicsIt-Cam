#include "Data/Attributes/FICAttributePosition.h"

#include "Editor/FICEditorContext.h"
#include "Editor/Data/FICEditorAttributeGroup.h"
#include "Editor/UI/FICKeyframeControl.h"
#include "Editor/UI/FICVectorEditor.h"

TSharedRef<FFICEditorAttributeBase> FFICAttributePosition::CreateEditorAttribute() {
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
				return &*(*Base)["X"];
			})
			.YAttr_Lambda([Base]() {
				return &*(*Base)["Y"];
			})
			.ZAttr_Lambda([Base]() {
				return &*(*Base)["Z"];
			})
			.AutoKeyframe_Lambda([Context]() {
				return Context->bAutoKeyframe;
			})
		];
	});
	return Base;
}

FVector FFICAttributePosition::FromEditorAttribute(FFICEditorAttributeGroup& Attribute) {
	return FVector(
		Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("X").GetValue(),
		Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Y").GetValue(),
		Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Z").GetValue()
	);
}

FVector FFICAttributePosition::FromEditorAttribute(FFICEditorAttributeGroup& Attribute, FICFrameFloat Time) {
	return FVector(
		Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("X").GetValue(Time),
		Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Y").GetValue(Time),
		Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Z").GetValue(Time)
	);
}

void FFICAttributePosition::ToEditorAttribute(const FVector& Vector, FFICEditorAttributeGroup& Attribute) {
	Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("X").SetValue(Vector.X);
	Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Y").SetValue(Vector.Y);
	Attribute.Get<TFICEditorAttribute<FFICFloatAttribute>>("Z").SetValue(Vector.Z);
}
