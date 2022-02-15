#include "Editor/FICChangeList.h"

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

void FFICChangeList::PushChange(TSharedPtr<FFICChange> InChange) {
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
