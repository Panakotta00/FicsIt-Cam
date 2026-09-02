#include "Data/Objects/FICCamera.h"

#include "Editor/Data/FICEditorAttributeGroup.h"
#include "FICUtils.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Components/LineBatchComponent.h"
#include "Editor/Data/FICEditorCameraActor.h"
#include "Editor/FICEditorContext.h"
#include "Editor/Data/FICEditorAttributeBase.h"
#include "Editor/Data/FICEditorAttributeBool.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"

UFICCamera::UFICCamera() {
	Active.SetDefaultValue(true);
	Aperture.SetDefaultValue(10);
	FocusDistance.SetDefaultValue(10000);

	LensSettings.AddChildAttribute(TEXT("FOV"), &FOV);
	LensSettings.AddChildAttribute(TEXT("Aperture"), &Aperture);
	LensSettings.AddChildAttribute(TEXT("Focus Distance"), &FocusDistance);

	RootAttribute.AddChildAttribute(TEXT("Active"), &Active);
	RootAttribute.AddChildAttribute(TEXT("Position"), &Position);
	RootAttribute.AddChildAttribute(TEXT("Rotation"), &Rotation);
	RootAttribute.AddChildAttribute(TEXT("Lens Settings"), &LensSettings);
	RootAttribute.AddChildAttribute(TEXT("Post Processing"), &PostProcessingSettings);
}

template<typename T>
FArchive& operator<<(FArchive& Ar, TInstancedStruct<T>& InstancedStruct) {
	InstancedStruct.Serialize(Ar);
	return Ar;
}

void UFICCamera::Serialize(FStructuredArchive::FRecord Record) {
	Super::Serialize(Record);

	if (!Record.GetUnderlyingArchive().IsSaveGame()) return;

	auto PostProcessingAttributes = Record.EnterField(TEXT("PostProcessingAttributes"));

	for (auto& [name, overrideAttrib] : PPOverrideAttribMap) {
		auto valueAttrib = PPValueAttribMap.Find(name);
		if (!valueAttrib) continue;

		auto entry = PostProcessingAttributes.TryEnterAttribute(*name, true);
		if (!entry.IsSet()) continue;
		auto entryRecord = entry->EnterRecord();

		if (entryRecord.GetUnderlyingArchive().IsSaving()) {
			PostProcessingSettings.RemoveChildAttribute(TEXT("Override ") + name);
		}
		if (auto field = entryRecord.TryEnterField(TEXT("OverrideAttrib"), true)) {
			overrideAttrib.Serialize(field->GetUnderlyingArchive());
		}
		if (entryRecord.GetUnderlyingArchive().IsSaving()) {
			PostProcessingSettings.AddChildAttribute(TEXT("Override ") + name, overrideAttrib.GetMutablePtr());
		}

		if (entryRecord.GetUnderlyingArchive().IsSaving()) {
			entryRecord << SA_VALUE(TEXT("ValueAttrib"), *valueAttrib);
		} else {
			PostProcessingSettings.RemoveChildAttribute(name);
			TInstancedStruct<FFICAttribute> value;
			entryRecord << SA_VALUE(TEXT("ValueAttrib"), value);
			if (value.GetScriptStruct() == valueAttrib->GetScriptStruct()) {
				*valueAttrib = value;
			}
			PostProcessingSettings.AddChildAttribute(name, valueAttrib->GetMutablePtr());
		}
	}
}

void UFICCamera::PostInitProperties() {
	Super::PostInitProperties();

	for (TFieldIterator<FProperty> iter(FPostProcessSettings::StaticStruct()); iter; ++iter) {
		FProperty* prop = *iter;
		FString name = prop->GetNameCPP();
		if (name.RemoveFromStart(TEXT("bOverride_"))) {
			if (auto boolProp = CastField<FBoolProperty>(prop)) {
				auto entry = PPPropertyMap.Find(name);
				if (entry) {
					entry->Key = boolProp;
				} else {
					PPPropertyMap.Add(name, {boolProp, nullptr});
				}
			}
		} else {
			auto entry = PPPropertyMap.Find(name);
			if (entry) {
				entry->Value = prop;
			} else {
				PPPropertyMap.Add(name, {nullptr, prop});
			}
		}
	}
	TArray<FString> toClean;
	for (auto& [name, props] : PPPropertyMap) {
		auto& [overrideProp, valueProp] = props;
		if (!overrideProp || !valueProp) {
			toClean.Add(name);
			continue;
		}

		if (!PPOverrideAttribMap.Contains(name)) {
			PPOverrideAttribMap.Add(name, TInstancedStruct<FFICAttributeBool>::Make());
		}

		if (!PPValueAttribMap.Contains(name)) {
			TInstancedStruct<FFICAttribute> valueAttrib;
			if (auto boolProp = CastField<FBoolProperty>(valueProp)) {
				valueAttrib = TInstancedStruct<FFICAttribute>::Make<FFICAttributeBool>();
			} else if (auto floatProp = CastField<FFloatProperty>(valueProp)) {
				valueAttrib = TInstancedStruct<FFICAttribute>::Make<FFICFloatAttribute>();
			} else if (auto intProp = CastField<FIntProperty>(valueProp)) {
				valueAttrib = TInstancedStruct<FFICAttribute>::Make<FFICFloatAttribute>();
			} else {
				toClean.Add(name);
				continue;
			}
			PPValueAttribMap.Add(name, MoveTemp(valueAttrib));
		}

		PostProcessingSettings.AddChildAttribute(TEXT("Override ") + name, PPOverrideAttribMap.Find(name)->GetMutablePtr());
		PostProcessingSettings.AddChildAttribute(name, PPValueAttribMap.Find(name)->GetMutablePtr());
	}
	for (FString name : toClean) PPPropertyMap.Remove(name);
}

void UFICCamera::Tick(float DeltaTime) {
	// Draw Path
	/*if (EditorContext && EditorContext->bShowPath) {
		FVector PrevLoc = FVector::ZeroVector;
		FRotator PrevRot = FRotator::ZeroRotator;
		for (int64 Time : EditorContext->GetScene()->AnimationRange) {
			bool bIsKeyframe = EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeBase>("Position").GetKeyframe(Time).IsValid();
			FVector Loc = FFICAttributePosition::FromEditorAttribute(EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Position"), Time);
			if (bIsKeyframe || Loc != PrevLoc) EditorContext->GetScene()->GetWorld()->LineBatcher->DrawLine(Loc, Loc, bIsKeyframe ? FColor::Yellow : FColor::Blue, SDPG_World, 20);
			if (PrevLoc != FVector::ZeroVector) {
				EditorContext->GetScene()->GetWorld()->LineBatcher->DrawLine(PrevLoc, Loc, FColor::Red, SDPG_World, 5);
			}
			PrevLoc = Loc;
		}
	}*/

	if (EditorContext && EditorContext->bShowPath && EditorCameraActor) {
		TArray<FVector>& FramePoints = EditorCameraActor->CameraPathComponent->FramePoints;
		TSet<int64>& KeyframePoints = EditorCameraActor->CameraPathComponent->KeyframePoints;
		int64& Hovered = EditorCameraActor->CameraPathComponent->Hovered;
		if (FramePoints.Num() > 0) {
			FVector* PrevPoint = nullptr;
			for (int32 i = 0; i < FramePoints.Num(); ++i) {
				FVector* Point = &FramePoints[i];

				FColor PointColor = FColor::Blue;
				bool bIsKeyframe = KeyframePoints.Contains(i);
				if (Hovered == i) PointColor = FColor::Green; 
				else if (bIsKeyframe) PointColor = FColor::Yellow;
				if (bIsKeyframe || Hovered == i || !PrevPoint || *PrevPoint != *Point) EditorContext->GetScene()->GetWorld()->GetLineBatcher(UWorld::ELineBatcherType::World)->DrawLine(*Point, *Point, PointColor, SDPG_World, 20);

				if (PrevPoint) EditorContext->GetScene()->GetWorld()->GetLineBatcher(UWorld::ELineBatcherType::World)->DrawLine(*PrevPoint, *Point,  FColor::Red, SDPG_World, 5);

				PrevPoint = Point;
			}
		}
	}
}

UObject* UFICCamera::CreateNewObject(UObject* InOuter, AFICScene* InScene) {
	UFICCamera* Camera = NewObject<UFICCamera>(InOuter);
	Camera->SceneObjectName = UFICUtils::AdjustSceneObjectName(InScene, Camera->SceneObjectName);
	FFICCameraSettingsSnapshot Snapshot = UFICUtils::CreateCameraSettingsSnapshotFromView(InOuter);
	Camera->Position.SetDefaultValue(Snapshot.Location);
	Camera->Rotation.SetDefaultValue(Snapshot.Rotation);
	Camera->FOV.SetDefaultValue(Snapshot.FOV);
	return Camera;
}
UE_DISABLE_OPTIMIZATION_SHIP
TSharedRef<SWidget> UFICCamera::CreateDetailsWidget(UFICEditorContext* InContext) {
	return SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight()[
			InContext->GetEditorAttributes()[this]->CreateDetailsWidget(InContext)
		]
		+SVerticalBox::Slot().AutoHeight()[
			SNew(SButton)
			.Text(FText::FromString("Adjust Keyframes for Constant Speed"))
			.OnClicked_Lambda([this, InContext]() {
				auto editorAttribute = InContext->GetEditorAttributes()[this];
				auto& attribute = editorAttribute->GetAttribute();
				auto& posAttrib = *(FFICAttributePosition*)&editorAttribute->Get<FFICEditorAttributeGroup>("Position").GetAttribute();

				bool collision = true;
				for (int collisionTries = 0; collision && collisionTries < 10; ++collisionTries) {
					collision = false;

					auto keyframes = attribute.GetKeyframes().Array();
					Algo::Sort(keyframes, [](const auto& a, const auto& b) { return a.Key < b.Key; });
					if (keyframes.Num() < 3) return FReply::Handled();

					FICFrame startFrame = keyframes[0].Key;
					FICFrame endFrame = keyframes.Last().Key;

					TArray<double> distances = {0};
					TOptional<FVector> lastPos;
					for (FICFrame frame = startFrame; frame <= endFrame; frame += 1) {
						float posX = posAttrib.X.GetValue(frame);
						float posY = posAttrib.Y.GetValue(frame);
						float posZ = posAttrib.Z.GetValue(frame);
						FVector pos(posX, posY, posZ);
						if (lastPos.IsSet()) distances.Last() += FVector::Dist(pos, lastPos.GetValue());
						if (attribute.HasKeyframe(frame) && lastPos.IsSet()) {
							double distance = distances.Last();
							distances.Add(distance);
						}
						lastPos = pos;
					}
					double fullDistance = distances.Last();

					for (int keyframeIndex = 1; keyframeIndex < keyframes.Num()-1; ++keyframeIndex) {
						FICFrame curTime = keyframes[keyframeIndex].Key;
						float factor = distances[keyframeIndex-1] / fullDistance;
						FICFrame newTime = startFrame + (float)(endFrame - startFrame) * factor;
						if (newTime == curTime) continue;
						int dir = (newTime < curTime) ? 1 : -1;
						while (attribute.HasKeyframe(newTime)) {
							newTime += dir;
							collision = true;
						}
						attribute.MoveKeyframe(curTime, newTime);
					}
				}
				attribute.RecalculateAllKeyframes();
				return FReply::Handled();
			})
		];
}
UE_ENABLE_OPTIMIZATION_SHIP

void UFICCamera::InitEditor(UFICEditorContext* Context) {
	EditorContext = Context;

	EditorCameraActor = Context->GetScene()->GetWorld()->SpawnActor<AFICEditorCameraActor>();
	EditorCameraActor->Initialize(EditorContext, this);
	EditorCameraActor->UpdateValues(Context->GetEditorAttributes()[this]);
}

void UFICCamera::ShutdownEditor(UFICEditorContext* Context) {
	if (EditorCameraActor) {
		EditorCameraActor->Destroy();
		EditorCameraActor = nullptr;
	}
	EditorContext = nullptr;
}

void UFICCamera::EditorUpdate(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) {
	EditorCameraActor->UpdateValues(Attribute);
	if (EditorContext->GetActiveCamera() == this) EditorContext->UpdateCharacterValues();
}

void UFICCamera::Select(UFICEditorContext* Context) {
	if (Context->GetCameraPreview()) {
		CameraPreviewWidget = SNew(SConstraintCanvas)
		+SConstraintCanvas::Slot()
		.Anchors(FAnchors(0.6, 0.6, 1,1))
		.Offset(FMargin(0))[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			.Content()[
				EditorCameraActor->GetCameraPreview()
			]
		];
		Context->AddOverlayWidget(CameraPreviewWidget.ToSharedRef());
	}
}

void UFICCamera::Unselect(UFICEditorContext* Context) {
	if (CameraPreviewWidget) {
		Context->RemoveOverlayWidget(CameraPreviewWidget.ToSharedRef());
		CameraPreviewWidget.Reset();
	}
}

FTransform UFICCamera::GetSceneObjectTransform() {
	FVector Pos = FFICAttributePosition::FromEditorAttribute( EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Position"));
	FRotator Rot = FFICAttributeRotation::FromEditorAttribute(EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Rotation"));
	return FTransform(Rot, Pos);
}

void UFICCamera::SetSceneObjectTransform(FTransform InTransform) {
	if (EditorCameraActor) {
		EditorCameraActor->SetActorTransform(InTransform);
		EditorContext->CommitAutoKeyframe(this);
		FRotator LastRotation = FFICAttributeRotation::FromEditorAttribute(EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Rotation"));
		FRotator NewRotation = UFICUtils::AdditiveRotation(LastRotation, InTransform.Rotator());
		FFICAttributePosition::ToEditorAttribute(InTransform.GetLocation(), EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Position"));
		FFICAttributeRotation::ToEditorAttribute(NewRotation, EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Rotation"));
		EditorContext->CommitAutoKeyframe(nullptr);
	}
}

AActor* UFICCamera::GetActor() { return EditorCameraActor; }

FPostProcessSettings UFICCamera::GetPostProcessingSettings(FICFrameFloat Frame) {
	FPostProcessSettings settings;
	for (auto [name, props] : PPPropertyMap) {
		auto& [overrideProp, valueProp] = props;

		bool overrideVal = PPOverrideAttribMap.Find(name)->GetMutable<FFICAttributeBool>().GetValue(Frame);
		overrideProp->SetPropertyValue_InContainer(&settings, overrideVal);

		if (auto boolProp = CastField<FBoolProperty>(valueProp)) {
			bool val = PPValueAttribMap.Find(name)->GetMutable<FFICAttributeBool>().GetValue(Frame);
			boolProp->SetPropertyValue_InContainer(&settings, val);
		} else if (auto floatProp = CastField<FFloatProperty>(valueProp)) {
			float val = PPValueAttribMap.Find(name)->GetMutable<FFICFloatAttribute>().GetValue(Frame);
			floatProp->SetPropertyValue_InContainer(&settings, val);
		} else if (auto intProp = CastField<FIntProperty>(valueProp)) {
			int val = PPValueAttribMap.Find(name)->GetMutable<FFICFloatAttribute>().GetValue(Frame);
			intProp->SetPropertyValue_InContainer(&settings, val);
		}
	}

	return settings;
}

FPostProcessSettings UFICCamera::GetPostProcessingSettings(TSharedRef<FFICEditorAttributeBase> ppAttrib) {
	FPostProcessSettings settings;
	for (auto [name, props] : PPPropertyMap) {
		auto& [overrideProp, valueProp] = props;

		bool overrideVal = ppAttrib->Get<FFICEditorAttributeBool>(TEXT("Override ") + name).GetActiveValue();
		overrideProp->SetPropertyValue_InContainer(&settings, overrideVal);

		if (auto boolProp = CastField<FBoolProperty>(valueProp)) {
			bool val = ppAttrib->Get<FFICEditorAttributeBool>(name).GetActiveValue();
			boolProp->SetPropertyValue_InContainer(&settings, val);
		} else if (auto floatProp = CastField<FFloatProperty>(valueProp)) {
			float val = ppAttrib->Get<TFICEditorAttribute<FFICFloatAttribute>>(name).GetValue();
			floatProp->SetPropertyValue_InContainer(&settings, val);
		} else if (auto intProp = CastField<FIntProperty>(valueProp)) {
			int val = ppAttrib->Get<TFICEditorAttribute<FFICFloatAttribute>>(name).GetValue();
			intProp->SetPropertyValue_InContainer(&settings, val);
		}
	}
	return settings;
}


