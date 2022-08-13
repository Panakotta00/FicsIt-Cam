#include "FicsItCam/Public/Editor/Data/FICEditorAttributeBase.h"

#include "Data/Attributes/FICAttribute.h"
#include "Editor/UI/FICSequencer.h"

TSharedRef<SFICSequencerRow> FFICEditorAttributeBase::CreateRow(SFICSequencer* InSequencer) {
	TAttribute<FLinearColor> BackgroundColor;
	if (GraphColor != FLinearColor::White) BackgroundColor = GraphColor;
	return
		SNew(SFICSequencerRowAttribute, InSequencer, AsShared())
		.BackgroundColor(BackgroundColor);
}

TArray<TSharedPtr<FFICSequencerRowMeta>> FFICEditorAttributeBase::GetChildRows() {
	TArray<TSharedPtr<FFICSequencerRowMeta>> RowProviders;
	for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Attribute : GetChildAttributes()) {
		RowProviders.Add(MakeShared<FFICSequencerRowMeta>(Attribute.Value, FText::FromString(Attribute.Key), Attribute.Value->GraphColor));
	}
	return RowProviders;
}

FName FFICEditorAttributeBase::GetAttributeType() const {
	return GetAttributeConst().GetAttributeType();
}

void FFICEditorAttributeBase::RemoveKeyframe(int64 Time) {
	GetAttribute().RemoveKeyframe(Time);
}

TSharedPtr<FFICKeyframe> FFICEditorAttributeBase::GetKeyframe(int64 Time) {
	TMap<FICFrame, TSharedRef<FFICKeyframe>> Keyframes = GetAttribute().GetKeyframes();
	TSharedRef<FFICKeyframe>* KF = Keyframes.Find(Time);
	if (KF) return *KF;
	return nullptr;
}

bool FFICEditorAttributeBase::IsAnimated() {
	return GetAttribute().GetKeyframes().Num() > 0;
}
