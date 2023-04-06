#include "Editor/UI/SelectionManager.h"

#include "Data/Attributes/FICAttributeGroup.h"

const TSet<TPair<FFICAttribute*, int64>>& FSelectionManager::GetSelection() const {
	return SelectedKeyframes;
}

void FSelectionManager::SetSelection(const TSet<TPair<FFICAttribute*, FICFrame>>& InSelection) {
	SelectedKeyframes.Empty();
	for (const TPair<FFICAttribute*, FICFrame>& Keyframe : InSelection) {
		AddKeyframeToSelection(*Keyframe.Key, Keyframe.Value, false);
	}
	SelectedWithBox = SelectedKeyframes;
	auto _ = OnSelectionChanged.ExecuteIfBound();
}

void FSelectionManager::AddKeyframeToSelection(FFICAttribute& InAttribute, FICFrame InFrame, bool TriggerUpdate) {
	const TMap<FString, FFICAttribute*>& Children = InAttribute.GetChildAttributes();
	if (Children.Num() > 0) {
		for (const TTuple<FString, FFICAttribute*>& Attr : Children) {
			AddKeyframeToSelection(*Attr.Value, InFrame);
		}
	} else {
		SelectedKeyframes.Add(TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame));
	}
	if (TriggerUpdate) auto _ = OnSelectionChanged.ExecuteIfBound();
}

void FSelectionManager::RemoveKeyframeFromSelection(FFICAttribute& InAttribute, FICFrame InFrame) {
	const TMap<FString, FFICAttribute*>& Children = InAttribute.GetChildAttributes();
	if (Children.Num() > 0) {
		for (const TTuple<FString, FFICAttribute*>& Attr : Children) {
			RemoveKeyframeFromSelection(*Attr.Value, InFrame);
		}
	} else {
		SelectedKeyframes.Remove(TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame));
	}
	auto _ = OnSelectionChanged.ExecuteIfBound();
}

bool FSelectionManager::IsKeyframeSelected(FFICAttribute& InAttribute, FICFrame InFrame) const {
	const TMap<FString, FFICAttribute*>& Children = InAttribute.GetChildAttributes();
	if (Children.Num() > 0) {
		bool IsGroupSelected = true;
		for (const TTuple<FString, FFICAttribute*>& Attr : Children) {
			if (Attr.Value->HasKeyframe(InFrame)) {
				IsGroupSelected = IsGroupSelected & IsKeyframeSelected(*Attr.Value, InFrame);
			}
		}
		return IsGroupSelected;
	} else {
		return GetSelection().Contains(TPair<FFICAttribute*, FICFrame>(&InAttribute, InFrame));
	}
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
