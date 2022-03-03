#include "Editor/UI/FICParticleSystemSelection.h"

#include "Particles/ParticleSystem.h"
#include "Widgets/Input/SComboBox.h"

void SFICParticleSystemSelection::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;
	OnSelectionChanged = InArgs._OnSelectionChanged;

	LoadParticleSystems();
	
	ChildSlot[
		SAssignNew(ComboBox, SComboBox<UParticleSystem*>)
		.Content()[
			SAssignNew(Content, STextBlock)
		]
		.OptionsSource(&ParticleSystems)
		.OnGenerateWidget_Lambda([this](UParticleSystem* System) {
			if (!System->IsValidLowLevelFast()) {
				LoadParticleSystems();
				return SNew(STextBlock);
			}
			return SNew(STextBlock)
			.Text(FText::FromString(System->GetName()));
		})
		.OnSelectionChanged_Lambda([this](UParticleSystem* System, ESelectInfo::Type) {
			if (!System->IsValidLowLevelFast()) return;
			OnSelectionChanged.ExecuteIfBound(System);
			UpdateContent();
		})
		.InitiallySelectedItem(InArgs._System)
	];

	UpdateContent();
}

void SFICParticleSystemSelection::LoadParticleSystems() {
	ParticleSystems.Empty();
	for (TObjectIterator<UParticleSystem> System; System; ++System) {
		ParticleSystems.Add(*System);
	}
	if (ComboBox) ComboBox->RefreshOptions();
}

void SFICParticleSystemSelection::UpdateContent() {
	if (!ComboBox->GetSelectedItem()) Content->SetText(FText::FromString(TEXT("Select Particle System")));
	else Content->SetText(FText::FromString(ComboBox->GetSelectedItem()->GetName()));
}
