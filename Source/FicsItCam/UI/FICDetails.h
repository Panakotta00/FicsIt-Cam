#pragma once

#include "SlateBasics.h"

class UFICEditorContext;

class SFICDetails : public SGridPanel {
	SLATE_BEGIN_ARGS(SFICDetails) {}
		SLATE_ARGUMENT(UFICEditorContext*, Context)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	UFICEditorContext* Context = nullptr;
};
