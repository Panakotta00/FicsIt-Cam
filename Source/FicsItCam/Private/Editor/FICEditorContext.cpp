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

	ActiveSceneObjectManager.Initialize(InScene);
	ActiveSceneObjectManager.IsSceneObjectActive.BindLambda([this](UObject* SceneObject, FICFrameFloat Frame) {
		IFICSceneObjectActive* Active = Cast<IFICSceneObjectActive>(SceneObject);
		TSharedRef<FFICEditorAttributeBase>* Attrib = EditorAttributeMap.Find(&Active->GetActiveAttribute());
		if (!Attrib) return false;
		return StaticCastSharedRef<FFICEditorAttributeBool>(*Attrib)->GetActiveValue();
	});

	if (Scene->bViewportEverSaved) {
		SetSelectedSceneObject(Scene->LastSelectedSceneObject);
		InEditorPlayerCharacter->SetActorLocation(Scene->LastCameraTransform.GetLocation());
		InEditorPlayerCharacter->SetActorRotation(Scene->LastCameraTransform.GetRotation().Rotator());
	}
}

void UFICEditorContext::Unload() {
	Scene->LastSelectedSceneObject = GetSelectedSceneObject();
	Scene->LastCameraTransform = FTransform(GetPlayerCharacter()->GetControlRotation(), GetPlayerCharacter()->GetActorLocation());
	Scene->bViewportEverSaved = true;
	ActiveSceneObjectManager.Shutdown();
	for (UObject* SceneObject : Scene->GetSceneObjects()) {
		UnloadSceneObject(SceneObject);
	}
	Scene = nullptr;
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

		for (UObject* SceneObject : GetScene()->GetSceneObjects()) {
			IFICSceneObject* ActiveSceneObject = Cast<IFICSceneObject>(SceneObject);
			if (ActiveSceneObject) {
				ActiveSceneObject->TickEditor(this, GetEditorAttributes()[SceneObject]); 
			}
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
	TFunction<void(TSharedRef<FFICEditorAttributeBase>)> AddEditAttrib;
	AddEditAttrib = [this, &AddEditAttrib](TSharedRef<FFICEditorAttributeBase> Attrib) {
		EditorAttributeMap.Add(&Attrib->GetAttribute(), Attrib);
		for (const TPair<FString, TSharedRef<FFICEditorAttributeBase>>& Child : Attrib->GetChildAttributes()) {
			AddEditAttrib(Child.Value);
		}
	};
	AddEditAttrib(Attribute);
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
		
		if (!bInAutoKeyframeSet) {
			IFICSceneObjectActive* SceneObjectActive = Cast<IFICSceneObjectActive>(SceneObject);
			if (SceneObjectActive) ActiveSceneObjectManager.UpdateActiveObjects(GetCurrentFrame());
		}
	});
	DataAttributeOnUpdateDelegateHandles.Add(SceneObject, Attribute->GetAttribute().OnUpdate.AddLambda([this, Attribute]() {
		if (bBlockValueUpdate) return;
		Attribute->UpdateValue(GetCurrentFrame());
	}));

	Cast<IFICSceneObject>(SceneObject)->InitEditor(this);

	ActiveSceneObjectManager.UpdateActiveObjects(GetCurrentFrame());
}

void UFICEditorContext::UnloadSceneObject(UObject* SceneObject) {
	Cast<IFICSceneObject>(SceneObject)->ShutdownEditor(this);
	
	if (GetSelectedSceneObject() == SceneObject) SetSelectedSceneObject(nullptr);
	
	AllAttributes->RemoveAttribute(FString::FromInt(SceneObject->GetUniqueID()));
	EditorAttributes[SceneObject]->GetAttribute().OnUpdate.Remove(DataAttributeOnUpdateDelegateHandles[SceneObject]);
	TFunction<void(TSharedRef<FFICEditorAttributeBase>)> RemoveEditAttrib;
	RemoveEditAttrib = [this, &RemoveEditAttrib](TSharedRef<FFICEditorAttributeBase> Attrib) {
		EditorAttributeMap.Remove(&Attrib->GetAttribute());
		for (const TPair<FString, TSharedRef<FFICEditorAttributeBase>>& Child : Attrib->GetChildAttributes()) {
			RemoveEditAttrib(Child.Value);
		}
	};
	RemoveEditAttrib(EditorAttributes[SceneObject]);
	EditorAttributes.Remove(SceneObject);
	DataAttributeOnUpdateDelegateHandles.Remove(SceneObject);

	ActiveSceneObjectManager.UpdateActiveObjects(GetCurrentFrame());
}

void UFICEditorContext::AddSceneObject(UObject* SceneObject) {
	if (!SceneObject) return;
	Scene->AddSceneObject(SceneObject);
	LoadSceneObject(SceneObject);
	OnSceneObjectsChanged.Broadcast();
}

void UFICEditorContext::RemoveSceneObject(UObject* SceneObject) {
	if (!SceneObject) return;
	UnloadSceneObject(SceneObject);
	Scene->RemoveSceneObject(SceneObject);
	OnSceneObjectsChanged.Broadcast();
}

void UFICEditorContext::MoveSceneObject(UObject* SceneObject, int Delta) {
	if (!SceneObject) return;
	Scene->MoveSceneObject(SceneObject, Delta);
	OnSceneObjectsChanged.Broadcast();
	SetSelectedSceneObject(SceneObject);
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

	ActiveSceneObjectManager.UpdateActiveObjects(inFrame);
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
