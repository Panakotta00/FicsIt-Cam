#pragma once

#include "Data/Attributes/FICAttribute.h"

class FFICChangeList;

typedef TPair<FFICAttribute*, TSharedRef<FFICAttribute>> FChangeStackEntry;

#define BEGIN_ATTRIB_CHANGE(Attribute) \
	FFICChange::ChangeStack.Push(FChangeStackEntry(Attribute, Attribute->Get()));

#define END_ATTRIB_CHANGE(ChangeList) { \
	FChangeStackEntry StackEntry = FFICChange::ChangeStack.Pop(); \
	ChangeList->PushChange(MakeShared<FFICChange_Attribute>(StackEntry.Key, StackEntry.Value)); \
	}

#define BEGIN_QUICK_ATTRIB_CHANGE(Context, Attribute, FromFrame, ToFrame) \
	auto _Change = MakeShared<FFICChange_Group>(); \
	_Change->PushChange(MakeShared<FFICChange_ActiveFrame>(Context, FromFrame, ToFrame)); \
	BEGIN_ATTRIB_CHANGE(Attribute)

#define END_QUICK_ATTRIB_CHANGE(ChangeList) \
	END_ATTRIB_CHANGE(_Change) \
	ChangeList.PushChange(_Change);

struct FFICChange {
	static TArray<FChangeStackEntry> ChangeStack;
	
	virtual ~FFICChange() = default;
	
	virtual void RedoChange() = 0;
	virtual void UndoChange() = 0;
	virtual FName ChangeType() = 0;
};

struct FFICChange_ActiveFrame : public FFICChange {
	class UFICEditorContext* EditorContext;
	int64 FromFrame;
	int64 ToFrame;
	
	FFICChange_ActiveFrame(class UFICEditorContext* InEditorContext, int64 FromFrame = TNumericLimits<int64>::Min(), int64 ToFrame = TNumericLimits<int64>::Min());

	virtual void RedoChange() override;
	virtual void UndoChange() override;
	virtual FName ChangeType() override { return FName(TEXT("ActiveFrame")); }
};

struct FFICChange_Attribute : public FFICChange {
	FFICAttribute* Attribute;
	TSharedRef<FFICAttribute> FromAttribute;
	TSharedRef<FFICAttribute> ToAttribute;

	FFICChange_Attribute(FFICAttribute* InAttribute, TSharedRef<FFICAttribute> InFromAttribute) : Attribute(InAttribute), FromAttribute(InFromAttribute), ToAttribute(Attribute->Get()) {}

	virtual void RedoChange() override {
		Attribute->Set(ToAttribute);
	}

	virtual void UndoChange() override {
		Attribute->Set(FromAttribute);
	}

	virtual FName ChangeType() override {
		return FName(TEXT("Attribute"));
	}
};

struct FFICChange_Group : public FFICChange {
	TSet<TSharedPtr<FFICChange>> Changes;
	
	virtual void RedoChange() override {
		for (TSharedPtr<FFICChange> Change : Changes) Change->RedoChange();
	}

	virtual void UndoChange() override {
		for (TSharedPtr<FFICChange> Change : Changes) Change->UndoChange();
	}

	virtual FName ChangeType() override {
		return FName(TEXT("Group"));
	}

	void PushChange(TSharedPtr<FFICChange> InChange) {
		Changes.Add(InChange);
	}
};

class FFICChangeList {
private:
	TArray<TSharedPtr<FFICChange>> Changes;
	int ChangeIndex = -1;

	int MaxChanges = 50;
	
public:
	void PushChange(TSharedPtr<FFICChange> InChange);
	TSharedPtr<FFICChange> PushChange();
	TSharedPtr<FFICChange> PopChange();
	TSharedPtr<FFICChange> PeakChange();
};
