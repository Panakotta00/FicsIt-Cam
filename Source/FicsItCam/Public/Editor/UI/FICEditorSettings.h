#pragma once

#include "SCompoundWidget.h"

class UFICEditorContext;

class SFICEditorSettings : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICEditorSettings) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);

private:
	UFICEditorContext* Context = nullptr;
};
