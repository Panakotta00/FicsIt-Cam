#pragma once

#include "FICAnimation.h"
#include "FICEditor.h"
#include "FICEditorContext.generated.h"

UCLASS()
class UFICEditorContext : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY()
	UFICAnimation* Animation = nullptr;

	UPROPERTY()
	int64 CurrentFrame = 0;

	TSharedPtr<SFICEditor> EditorWidget;
	TSharedPtr<SViewport> GameViewport;
	TSharedPtr<SHorizontalBox> GameViewportContainer;

	void ShowEditor();
	void HideEditor();
};
