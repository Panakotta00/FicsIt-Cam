#include "Editor/UI/FICUIUtil.h"

FMenuBuilder FICCreateKeyframeTypeChangeMenu(UFICEditorContext* Context, TFunction<TSet<TPair<FFICAttribute*, FICFrame>>()> GetKeyframes) {
	TFunction<void(EFICKeyframeType)> SetKeyframeType;
	SetKeyframeType = [GetKeyframes, Context](EFICKeyframeType Type) {
		TSharedRef<FFICChange_Group> Group = MakeShared<FFICChange_Group>();
		TMap<FFICAttribute*, TSharedRef<FFICAttribute>> Snapshots;
		TSet<TTuple<FFICAttribute*, long long>> Keyframes = GetKeyframes();
		for (const TPair<FFICAttribute*, FICFrame>& KF : Keyframes) {
			TSharedRef<FFICAttribute>* Snapshot = Snapshots.Find(KF.Key);
			if (!Snapshot) Snapshots.Add(KF.Key, KF.Key->Get());
			TMap<FICFrame, TSharedRef<FFICKeyframe>> KFS = KF.Key->GetKeyframes();
			TSharedRef<FFICKeyframe>* NKF = KFS.Find(KF.Value);
			if (NKF) (*NKF)->SetType(Type);
			KF.Key->LockUpdateEvent();
			KF.Key->RecalculateAllKeyframes();
			KF.Key->UnlockUpdateEvent(false);
		}
		for (const TPair<FFICAttribute*, TSharedRef<FFICAttribute>>& Snapshot : Snapshots) {
			Group->PushChange(MakeShared<FFICChange_Attribute>(Snapshot.Key, Snapshot.Value));
		}
		Context->ChangeList.PushChange(Group);
		for (const TPair<FFICAttribute*, FICFrame>& KF : Keyframes) KF.Key->OnUpdate.Broadcast();
	};

	TSharedPtr<IMenu> MenuHandle;
	FMenuBuilder MenuBuilder(true, NULL);
	MenuBuilder.AddMenuEntry(
        FText::FromString("Ease"),
        FText(),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([SetKeyframeType]() {
            SetKeyframeType(FIC_KF_EASE);
        }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
	MenuBuilder.AddMenuEntry(
        FText::FromString("Ease-In/Out"),
        FText(),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([SetKeyframeType]() {
            SetKeyframeType(FIC_KF_EASEINOUT);
        }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
	MenuBuilder.AddMenuEntry(
        FText::FromString("Linear"),
        FText(),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([SetKeyframeType]() {
            SetKeyframeType(FIC_KF_LINEAR);
        }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
	MenuBuilder.AddMenuEntry(
        FText::FromString("Step"),
        FText(),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([SetKeyframeType]() {
            SetKeyframeType(FIC_KF_STEP);
        }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));

	return MenuBuilder;
}
