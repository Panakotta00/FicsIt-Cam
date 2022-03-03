#pragma once
#include "Widgets/Input/SComboBox.h"

class UFICEditorContext;

class SFICParticleSystemSelection : public SCompoundWidget {
public:
	DECLARE_DELEGATE_OneParam(FOnSelectionChanged, UParticleSystem*)
	
	SLATE_BEGIN_ARGS(SFICParticleSystemSelection) {}
	SLATE_EVENT(FOnSelectionChanged, OnSelectionChanged)
	SLATE_ARGUMENT_DEFAULT(UParticleSystem*, System) = nullptr;
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UFICEditorContext* InContext);

private:
	UFICEditorContext* Context = nullptr;
	TArray<UParticleSystem*> ParticleSystems;
	FOnSelectionChanged OnSelectionChanged;
	TSharedPtr<SComboBox<UParticleSystem*>> ComboBox;
	TSharedPtr<STextBlock> Content;

public:
	void LoadParticleSystems();
	void UpdateContent();
};
