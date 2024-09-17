#include "Editor/FICChangeList.h"

#include "FICSubsystem.h"
#include "Editor/FICEditorContext.h"

TArray<FChangeStackEntry> FFICChange::ChangeStack = TArray<FChangeStackEntry>();

FFICChange_ActiveFrame::FFICChange_ActiveFrame(UFICEditorContext* InEditorContext, int64 InFromFrame, int64 InToFrame) : EditorContext(InEditorContext), FromFrame(InFromFrame), ToFrame(InToFrame) {
	if (ToFrame == TNumericLimits<int64>::Min()) {
		ToFrame = EditorContext->GetCurrentFrame();
	}
	if (FromFrame == TNumericLimits<int64>::Min()) {
		TFunction<bool(TSharedPtr<FFICChange>)> DoChange;
		DoChange = [this, &DoChange](TSharedPtr<FFICChange> InChange) {
			if (InChange) {
				if (InChange->ChangeType() == "Group") {
					for (TSharedPtr<FFICChange> Change : StaticCastSharedPtr<FFICChange_Group>(InChange)->Changes) {
						if (DoChange(Change)) return true;
					}
				} else if (InChange->ChangeType() == "ActiveFrame") {
					FromFrame = StaticCastSharedPtr<FFICChange_ActiveFrame>(InChange)->ToFrame;
					return true;
				}
			}
			return false;
		};
		if (!DoChange(EditorContext->ChangeList.PeakChange())) {
			FromFrame = EditorContext->GetScene()->AnimationRange.Begin;
		}
	}
}

void FFICChange_ActiveFrame::RedoChange() {
	EditorContext->SetCurrentFrame(ToFrame);
}

void FFICChange_ActiveFrame::UndoChange() {
	EditorContext->SetCurrentFrame(FromFrame);
}

void FFICChange_AddSceneObject::RedoChange() {
	UObject* SceneObject = Context->FindSceneObject(SceneObjectName);
	if (SceneObject) return;
	UObject* CDO = SceneObjectClass->GetDefaultObject();
	SceneObject = Cast<IFICSceneObject>(CDO)->CreateNewObject(AFICSubsystem::GetFICSubsystem(Context), Context->GetScene());
	Cast<IFICSceneObject>(SceneObject)->SetSceneObjectName(SceneObjectName);
	Context->AddSceneObject(SceneObject);
	Context->SetSelectedSceneObject(SceneObject);
	if (Snapshot) Cast<IFICSceneObject>(SceneObject)->GetRootAttribute().CopyFrom(Snapshot.ToSharedRef());
	Snapshot.Reset();
}

void FFICChange_AddSceneObject::UndoChange() {
	UObject* SceneObject = Context->FindSceneObject(SceneObjectName);
	if (!SceneObject) return;
	Snapshot = Cast<IFICSceneObject>(SceneObject)->GetRootAttribute().CreateCopy();
	Context->RemoveSceneObject(SceneObject);
	SceneObject = nullptr;
}

FFICChange_RemoveSceneObject::FFICChange_RemoveSceneObject(UFICEditorContext* InContext, UObject* InSceneObject) :
	Context(InContext),
	SceneObjectName(Cast<IFICSceneObject>(InSceneObject)->GetSceneObjectName()),
	SceneObjectClass(InSceneObject->GetClass()),
	Snapshot(Cast<IFICSceneObject>(InSceneObject)->GetRootAttribute().CreateCopy()) {}

void FFICChange_RemoveSceneObject::RedoChange() {
	UObject* SceneObject = Context->FindSceneObject(SceneObjectName);
	if (!SceneObject) return;
	Snapshot = Cast<IFICSceneObject>(SceneObject)->GetRootAttribute().CreateCopy();
	Context->RemoveSceneObject(SceneObject);
}

void FFICChange_RemoveSceneObject::UndoChange() {
	UObject* SceneObject = Context->FindSceneObject(SceneObjectName);
	if (SceneObject) return;
	UObject* CDO = SceneObjectClass->GetDefaultObject();
	SceneObject = Cast<IFICSceneObject>(CDO)->CreateNewObject(AFICSubsystem::GetFICSubsystem(Context), Context->GetScene());
	Cast<IFICSceneObject>(SceneObject)->SetSceneObjectName(SceneObjectName);
	Cast<IFICSceneObject>(SceneObject)->GetRootAttribute().CopyFrom(Snapshot.ToSharedRef());
	Snapshot.Reset();
	Context->AddSceneObject(SceneObject);
	Context->SetSelectedSceneObject(SceneObject);
}

void FFICChangeList::PushChange(TSharedRef<FFICChange> InChange) {
	if (Changes.Num() > 0) {
		TSharedRef<FFICChange> Change = Changes[Changes.Num()-1];
		if (Change->IsStackable(InChange)) {
			Change->Stack(InChange);
			return;
		}
	}
	if (Changes.Num() > ChangeIndex+1) Changes.RemoveAt(ChangeIndex+1, Changes.Num() - ChangeIndex - 1);
	Changes.Push(InChange);

	if (Changes.Num() > MaxChanges) {
		Changes.RemoveAt(0, Changes.Num() - MaxChanges);
	}
	
	ChangeIndex = Changes.Num() - 1;
}

TSharedPtr<FFICChange> FFICChangeList::PushChange() {
	if (ChangeIndex >= Changes.Num()-1) return nullptr;
	return Changes[++ChangeIndex];
}

TSharedPtr<FFICChange> FFICChangeList::PopChange() {
	if (ChangeIndex < 0) return nullptr;
	return Changes[ChangeIndex--];
}

TSharedPtr<FFICChange> FFICChangeList::PeakChange() {
	if (ChangeIndex < 0) return nullptr;
	return Changes[ChangeIndex];
}
