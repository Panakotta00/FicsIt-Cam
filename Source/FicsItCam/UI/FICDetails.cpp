#include "FICDetails.h"

#include "util/Logging.h"

void SFICDetails::Construct(const FArguments& InArgs) {
	Context = InArgs._Context;

	AddSlot(0,0)[
		SNew(STextBlock)
		.Text(FText::FromString("Hey Ho!"))
	];
	AddSlot(0,1)[
		SNew(SButton)
		.Text(FText::FromString("What the actual fuck???"))
		.DesiredSizeScale(FVector2D(2,2))
		.OnClicked_Lambda([]() {
			SML::Logging::error("Clicked!!!");
			return FReply::Handled();
		})
	];
}
