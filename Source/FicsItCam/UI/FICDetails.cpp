#include "FICDetails.h"

#include "FICEditorContext.h"
#include "FICNumericType.h"
#include "FICVectorEditor.h"
#include "SNumericEntryBox.h"
#include "FICKeyframeControl.h"

FSlateColorBrush SFICDetails::DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex("030303"));

void SFICDetails::Construct(const FArguments& InArgs) {
	Context = InArgs._Context;
	BackgroundBrush = InArgs._Background;

	ChildSlot[
		SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(BackgroundBrush)
		]
		+SOverlay::Slot()[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(STextBlock)
				.Text(FText::FromString("Details:"))
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("Position:"))
				]
				+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
					SNew(SFICVectorEditor)
					.ShowKeyframeControls(true)
					.XAttr_Lambda([this]() {
						return &Context->PosX;
					})
				    .YAttr_Lambda([this]() {
				        return &Context->PosY;
					})
                    .ZAttr_Lambda([this]() {
                        return &Context->PosZ;
                    })
                    .Frame_Lambda([this]() {
	                    return Context->GetCurrentFrame();
                    })
				]
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
                SNew(SHorizontalBox)
                +SHorizontalBox::Slot().Padding(5).AutoWidth()[
                    SNew(STextBlock)
                    .Text(FText::FromString("Rotation:"))
                ]
                +SHorizontalBox::Slot().Padding(5).FillWidth(1)[
                    SNew(SFICVectorEditor)
                    .ShowKeyframeControls(true)
                    .XAttr_Lambda([this]() {
	                    return &Context->RotPitch;
                    })
                    .YAttr_Lambda([this]() {
                        return &Context->RotYaw;
                    })
                    .ZAttr_Lambda([this]() {
                        return &Context->RotRoll;
                    })
                ]
            ]
            +SVerticalBox::Slot().Padding(5).AutoHeight()[
                SNew(SHorizontalBox)
                +SHorizontalBox::Slot().Padding(5).AutoWidth()[
                    SNew(STextBlock)
                    .Text(FText::FromString("FOV:"))
                ]
                +SHorizontalBox::Slot().Padding(5).FillWidth(1)[
                    SNew(SNumericEntryBox<float>)
                    .Value_Lambda([this]() {
	                    return Context->FOV.GetValue();
                    })
                    .OnValueCommitted_Lambda([this](float Val, auto) {
	                    Context->FOV.SetValue(Val);
                    })
                    .TypeInterface(MakeShared<TFICNumericTypeInterface<float>>())
                ]
                +SHorizontalBox::Slot().Padding(5).AutoWidth()[
	                SNew(SFICKeyframeControl)
		            .Attribute_Lambda([this]() {
			            return &Context->FOV;
		            })
		            .Frame_Lambda([this]() {
			            return Context->GetCurrentFrame();
		            })
				]
            ]
			+SVerticalBox::Slot().FillHeight(1)[
				SNew(SSpacer)
			]
		]
	];
}
