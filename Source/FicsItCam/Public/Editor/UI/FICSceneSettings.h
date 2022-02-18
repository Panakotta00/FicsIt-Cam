#pragma once

class UFICEditorContext;

class SFICSceneSettings : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICSceneSettings) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);
	
private:
	UFICEditorContext* Context = nullptr;
};
