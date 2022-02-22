#include "Editor/FICEditorContext.h"

#include "FGInputLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Data/Objects/FICCamera.h"
#include "Data/Objects/FICSceneObject.h"
#include "Editor/Data/FICEditorAttributeBool.h"

void UFICEditorContext::SetAnimPlayer(EFICAnimPlayerState InAnimPlayerState, float InAnimPlayerFactor) {
	AnimPlayerFactor = InAnimPlayerFactor;
	if (AnimPlayerState != InAnimPlayerState) {
		AnimPlayerState = InAnimPlayerState;
		AnimPlayerDelta = 0;
	}
}

void UFICEditorContext::Load(AFICEditorCameraCharacter* InEditorPlayerCharacter, AFICScene* InScene) {
	EditorPlayerCharacter = InEditorPlayerCharacter;
	Scene = InScene;

	AllAttributes = MakeShared<FFICEditorAttributeGroupDynamic>();

	for (UObject* SceneObject : Scene->GetSceneObjects()) {
		LoadSceneObject(SceneObject);
	}
	
	SetCurrentFrame(Scene->AnimationRange.Begin);

	FFICFrameRange Range = InScene->AnimationRange;
	FICFrame Len = Range.Length();
	Range.Begin -= Len/5;
	Range.End += Len/5;
	SetActiveRange(Range);
}

void UFICEditorContext::Unload() {
	for (UObject* SceneObject : Scene->GetSceneObjects()) {
		UnloadSceneObject(SceneObject);
	}
}

UFICEditorContext::UFICEditorContext() {}

void UFICEditorContext::Tick(float DeltaTime) {
	if (Scene) {
		AnimPlayerDelta += DeltaTime * AnimPlayerFactor;
		int32 FrameDelta = FMath::Floor(AnimPlayerDelta * Scene->FPS);
		AnimPlayerDelta -= (float)FrameDelta / (float)Scene->FPS;
		switch (AnimPlayerState) {
		case FIC_PLAY_FORWARDS:
			SetCurrentFrame(CurrentFrame + FrameDelta);
			break;
		case FIC_PLAY_BACKWARDS:
			SetCurrentFrame(CurrentFrame - FrameDelta);
			break;
		default: ;
		}
	}
}

bool UFICEditorContext::IsTickable() const {
	return !WITH_EDITOR;
}

void UFICEditorContext::LoadSceneObject(UObject* SceneObject) {
	TSharedRef<FFICEditorAttributeBase> Attribute = Cast<IFICSceneObject>(SceneObject)->GetRootAttribute().CreateEditorAttribute();
	Attribute->UpdateValue(GetCurrentFrame());
	AllAttributes->AddAttribute(FString::FromInt(SceneObject->GetUniqueID()), Attribute);
	EditorAttributes.Add(SceneObject, Attribute);
	Attribute->OnValueChanged.AddLambda([this, Attribute, SceneObject]() {
		Cast<IFICSceneObject>(SceneObject)->EditorUpdate(this, Attribute);
		if (bAutoKeyframe && !bInAutoKeyframeSet && AutoKeyframeChangeRef) {
			bInAutoKeyframeSet = true;
			bBlockValueUpdate = true;
			FFICChange::ChangeStack.Push(FChangeStackEntry(&Attribute->GetAttribute(), Attribute->GetAttribute().Get()));

			Attribute->SetKeyframe(GetCurrentFrame());

			FChangeStackEntry StackEntry = FFICChange::ChangeStack.Pop();
			ChangeList.PushChange(MakeShared<FFICChange_Attribute>(StackEntry.Key, StackEntry.Value, FFICChangeSource(AutoKeyframeChangeRef, FString::FromInt(GetCurrentFrame()))));
			bBlockValueUpdate = false;
			bInAutoKeyframeSet = false;
		}
	});
	DataAttributeOnUpdateDelegateHandles.Add(SceneObject, Attribute->GetAttribute().OnUpdate.AddLambda([this, Attribute]() {
		if (bBlockValueUpdate) return;
		Attribute->UpdateValue(GetCurrentFrame());
	}));

	Cast<IFICSceneObject>(SceneObject)->InitEditor(this);
}

void UFICEditorContext::UnloadSceneObject(UObject* SceneObject) {
	Cast<IFICSceneObject>(SceneObject)->UnloadEditor(this);
	
	if (GetSelectedSceneObject() == SceneObject) SetSelectedSceneObject(nullptr);
	
	AllAttributes->RemoveAttribute(FString::FromInt(SceneObject->GetUniqueID()));
	EditorAttributes[SceneObject]->GetAttribute().OnUpdate.Remove(DataAttributeOnUpdateDelegateHandles[SceneObject]);
	EditorAttributes.Remove(SceneObject);
	DataAttributeOnUpdateDelegateHandles.Remove(SceneObject);
}

void UFICEditorContext::AddSceneObject(UObject* SceneObject) {
	Scene->AddSceneObject(SceneObject);
	LoadSceneObject(SceneObject);
	OnSceneObjectsChanged.Broadcast();
}

void UFICEditorContext::RemoveSceneObject(UObject* SceneObject) {
	UnloadSceneObject(SceneObject);
	Scene->RemoveSceneObject(SceneObject);
	OnSceneObjectsChanged.Broadcast();
}

AFICScene* UFICEditorContext::GetScene() const {
	return Scene;
}

UFICCamera* UFICEditorContext::GetCamera() {
	for (UObject* Object : Scene->GetSceneObjects()) {
		UFICCamera* CameraObject = Cast<UFICCamera>(Object);
		if (CameraObject) {
			if (GetEditorAttributes()[CameraObject]->Get<FFICEditorAttributeBool>("Active").GetActiveValue()) return Cast<UFICCamera>(Object);
		}
	}
	return nullptr;
}

TSharedPtr<FFICEditorAttributeBase> UFICEditorContext::GetCameraEditor() {
	UFICCamera* Camera = GetCamera();
	if (!Camera) return nullptr;
	return GetEditorAttributes()[GetCamera()];
}

void UFICEditorContext::SetCurrentFrame(FICFrame inFrame) {
	CurrentFrame = inFrame;

	bInAutoKeyframeSet = true;
	AllAttributes->UpdateValue(inFrame);
	UpdateCharacterValues();
	OnCurrentFrameChanged.Broadcast();
	bInAutoKeyframeSet = false;
}

int64 UFICEditorContext::GetCurrentFrame() const {
	return CurrentFrame;
}

void UFICEditorContext::SetActiveRange(const FFICFrameRange& InActiveRange) {
	ActiveRange = InActiveRange;
}

FFICFrameRange UFICEditorContext::GetActiveRange() {
	return ActiveRange;
}

void UFICEditorContext::SetSelectedSceneObject(UObject* SceneObject) {
	if (SelectedSceneObject) Cast<IFICSceneObject>(SelectedSceneObject)->Unselect(this);
	SelectedSceneObject = SceneObject;
	if (SelectedSceneObject) Cast<IFICSceneObject>(SelectedSceneObject)->Select(this);
	OnSceneObjectSelectionChanged.Broadcast();
}

UObject* UFICEditorContext::GetSelectedSceneObject() {
	return SelectedSceneObject;
}

void UFICEditorContext::SetAutoKeyframe(bool bInAutoKeyframe) {
	bAutoKeyframe = bInAutoKeyframe;
}

void UFICEditorContext::SetLockCameraToView(bool bInLockCameraToView) {
	bMoveCamera = bInLockCameraToView;
	OnSceneObjectsChanged.Broadcast();
}

void UFICEditorContext::UpdateCharacterValues() {
	if (bMoveCamera) EditorPlayerCharacter->UpdateValues();
}

void UFICEditorContext::ToggleCurrentKeyframes() {
	if (!GetSelectedSceneObject()) return;
	TSharedRef<FFICEditorAttributeBase> Attribute = GetEditorAttributes()[GetSelectedSceneObject()];
	bBlockValueUpdate = true;
	if (Attribute->HasChanged(GetCurrentFrame()) || !Attribute->AllKeyframesSet(GetCurrentFrame())) Attribute->SetKeyframe(GetCurrentFrame());
	else Attribute->RemoveKeyframe(GetCurrentFrame());
	bBlockValueUpdate = false;
	SetCurrentFrame(GetCurrentFrame());
}
