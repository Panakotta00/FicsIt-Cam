#pragma once
#include "Data/Attributes/FICAttribute.h"


class FSelectionManager {
private:
	TSet<TPair<FFICAttribute*, FICFrame>> SelectedKeyframes;
	TSet<TPair<FFICAttribute*, FICFrame>> SelectedWithBox;
	FBox2D BoxSelection;

public:
	DECLARE_DELEGATE(FOnSelectionChangedEvent);
	DECLARE_DELEGATE_TwoParams(FHandleBoxSelectionEvent, const FBox2D& InBox, const FModifierKeysState& InModifiers);

	FOnSelectionChangedEvent OnSelectionChanged;
	FHandleBoxSelectionEvent OnHandleBoxSelection;
	
	const TSet<TPair<FFICAttribute*, int64>>& GetSelection() const;
	void SetSelection(const TSet<TPair<FFICAttribute*, FICFrame>>& InSelection);
	void AddKeyframeToSelection(FFICAttribute& InAttribute, FICFrame InFrame, bool TriggerUpdate = true);
	void RemoveKeyframeFromSelection(FFICAttribute& InAttribute, FICFrame InFrame);
	bool IsKeyframeSelected(FFICAttribute& InAttribute, FICFrame InFrame) const;
	void ToggleKeyframeSelection(FFICAttribute& InAttribute, FICFrame InFrame, const FModifierKeysState* InModifiers = nullptr);
	void BeginBoxSelection(const FModifierKeysState& InModifiers);
	void EndBoxSelection(const FModifierKeysState& InModifiers);
	void SetBoxSelection(FBox2D InBox, const FModifierKeysState& InModifiers);
	FBox2D GetSelectionBox() const { return BoxSelection; }
};
