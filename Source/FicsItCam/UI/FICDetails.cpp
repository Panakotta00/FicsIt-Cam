#include "FICDetails.h"

void SFICDetails::Construct(const FArguments& InArgs) {
	Context = InArgs._Context;

	AddSlot(0,0)[
		SNew(STextBlock)
		.Text(FText::FromString("Hey Ho!"))
	];
}
