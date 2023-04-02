#include "Editor/UI/SelectionManager.h"

const TSet<TPair<FFICAttribute*, int64>>& FSelectionManager::GetSelection() const {
	return SelectedKeyframes;
}

void FSelectionManager::SetSelection(const TSet<TPair<FFICAttribute*, FICFrame>>& InSelection) {
	SelectedKeyframes = InSelection;
	SelectedWithBox = SelectedKeyframes;
	auto _ = OnSelectionChanged.ExecuteIfBound();
}

void FSelectionManager::AddKeyframeToSelection(FFICAttribute& InAttribute, FICFrame InFrame) {
	SelectedKeyframes.Add(TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame));
	auto _ = OnSelectionChanged.ExecuteIfBound();
}

void FSelectionManager::RemoveKeyframeFromSelection(FFICAttribute& InAttribute, FICFrame InFrame) {
	SelectedKeyframes.Remove(TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame));
	auto _ = OnSelectionChanged.ExecuteIfBound();
}

bool FSelectionManager::IsKeyframeSelected(FFICAttribute& InAttribute, FICFrame InFrame) const {
	return GetSelection().Contains(TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame));
}

void FSelectionManager::ToggleKeyframeSelection(FFICAttribute& InAttribute, FICFrame InFrame, const FModifierKeysState* InModifiers) {
	bool bIsSelected = IsKeyframeSelected(InAttribute, InFrame);
	if (!InModifiers || InModifiers->IsShiftDown()) {
		if (bIsSelected) {
			RemoveKeyframeFromSelection(InAttribute, InFrame);
		} else {
			AddKeyframeToSelection(InAttribute, InFrame);
		}
	} else if (InModifiers->IsControlDown()) {
		if (!bIsSelected) AddKeyframeToSelection(InAttribute, InFrame);
	} else if (InModifiers->IsAltDown()) {
		if (bIsSelected) RemoveKeyframeFromSelection(InAttribute, InFrame);
	} else if (!BoxSelection.bIsValid) {
		SetSelection({TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame)});
	} else {
		if (!bIsSelected) AddKeyframeToSelection(InAttribute, InFrame);
	}
}

void FSelectionManager::BeginBoxSelection(const FModifierKeysState& InModifiers) {
	SelectedWithBox = SelectedKeyframes;
	if (!InModifiers.IsCommandDown() && !InModifiers.IsShiftDown() && !InModifiers.IsAltDown()) {
		SetSelection({});
	}
	BoxSelection = FBox2D(0);
	BoxSelection.bIsValid = true;
}

void FSelectionManager::EndBoxSelection(const FModifierKeysState& InModifiers) {
	SelectedWithBox = SelectedKeyframes;
	BoxSelection.bIsValid = false;
}

void FSelectionManager::SetBoxSelection(FBox2D InBox, const FModifierKeysState& InModifiers) {
	BoxSelection = InBox;
	BoxSelection.bIsValid = true;
	SelectedKeyframes = SelectedWithBox;
	
	OnHandleBoxSelection.Execute(InBox, InModifiers);
}
