#pragma once

#include "Data/Attributes/FICAttribute.h"
#include "Data/Objects/FICSceneObject.h"

class FFICChangeList;

typedef TPair<FFICAttribute*, TSharedRef<FFICAttribute>> FChangeStackEntry;

#define BEGIN_ATTRIB_CHANGE(Attribute) \
	FFICChange::ChangeStack.Push(FChangeStackEntry(&Attribute, Attribute.CreateCopy()));

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
	virtual bool IsStackable(TSharedRef<FFICChange> InChange) { return false; }
	virtual void Stack(TSharedRef<FFICChange> InChange) {}
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

struct FFICChangeSource {
	void* SourcePtr = nullptr;
	FString SourceKey;

	FFICChangeSource(void* InSourcePtr = nullptr, FString InSourceKey = TEXT("")) : SourcePtr(InSourcePtr), SourceKey(InSourceKey) {}

	bool IsValid() { return !!SourcePtr; }

	bool operator==(const FFICChangeSource& Other) const {
		return SourcePtr == Other.SourcePtr && SourceKey == Other.SourceKey;
	}
};

struct FFICChange_Attribute : public FFICChange {
	FFICAttribute* Attribute;
	TSharedRef<FFICAttribute> FromAttribute;
	TSharedRef<FFICAttribute> ToAttribute;
	FFICChangeSource ChangeSource;

	FFICChange_Attribute(FFICAttribute* InAttribute, TSharedRef<FFICAttribute> InFromAttribute, FFICChangeSource ChangeSource = FFICChangeSource()) : Attribute(InAttribute), FromAttribute(InFromAttribute), ToAttribute(Attribute->CreateCopy()), ChangeSource(ChangeSource) {}

	virtual void RedoChange() override {
		Attribute->CopyFrom(ToAttribute);
	}

	virtual void UndoChange() override {
		Attribute->CopyFrom(FromAttribute);
	}

	virtual FName ChangeType() override {
		return FName(TEXT("Attribute"));
	}

	virtual bool IsStackable(TSharedRef<FFICChange> InChange) override {
		if (InChange->ChangeType() == ChangeType() && ChangeSource.IsValid()) {
			return ChangeSource == StaticCastSharedRef<FFICChange_Attribute>(InChange)->ChangeSource;
		}
		return false;
	}

	virtual void Stack(TSharedRef<FFICChange> InChange) override {
		ToAttribute = StaticCastSharedRef<FFICChange_Attribute>(InChange)->ToAttribute;
	}
};

struct FFICChange_AddSceneObject : public FFICChange {
	UFICEditorContext* Context;
	UClass* SceneObjectClass;
	FString SceneObjectName;
	TSharedPtr<FFICAttribute> Snapshot;

	FFICChange_AddSceneObject(UFICEditorContext* InContext, UObject* InSceneObject) : Context(InContext), SceneObjectClass(InSceneObject->GetClass()), SceneObjectName(Cast<IFICSceneObject>(InSceneObject)->GetSceneObjectName()) {}

	virtual void RedoChange() override;
	virtual void UndoChange() override;

	virtual FName ChangeType() override {
		return FName(TEXT("AddSceneObject"));
	}
};

struct FFICChange_RemoveSceneObject : public FFICChange {
	UFICEditorContext* Context;
	FString SceneObjectName = "";
	UClass* SceneObjectClass;
	TSharedPtr<FFICAttribute> Snapshot;

	FFICChange_RemoveSceneObject(UFICEditorContext* InContext, UClass* InSceneObjectClass, TSharedRef<FFICAttribute> InSnapshot) : Context(InContext), SceneObjectClass(InSceneObjectClass), Snapshot(InSnapshot) {}
	FFICChange_RemoveSceneObject(UFICEditorContext* InContext, UObject* InSceneObject);

	virtual void RedoChange() override;
	virtual void UndoChange() override;

	virtual FName ChangeType() override {
		return FName(TEXT("RemoveSceneObject"));
	}
};

struct FFICChange_Group : public FFICChange {
	TSet<TSharedRef<FFICChange>> Changes;
		
	virtual void RedoChange() override {
		for (TSharedRef<FFICChange> Change : Changes) Change->RedoChange();
	}

	virtual void UndoChange() override {
		for (TSharedRef<FFICChange> Change : Changes) Change->UndoChange();
	}

	virtual FName ChangeType() override {
		return FName(TEXT("Group"));
	}

	void PushChange(TSharedRef<FFICChange> InChange) {
		Changes.Add(InChange);
	}
};

class FFICChangeList {
private:
	TArray<TSharedRef<FFICChange>> Changes;
	int ChangeIndex = -1;

	int MaxChanges = 50;
	
public:
	void PushChange(TSharedRef<FFICChange> InChange);
	TSharedPtr<FFICChange> PushChange();
	TSharedPtr<FFICChange> PopChange();
	TSharedPtr<FFICChange> PeakChange();
};
