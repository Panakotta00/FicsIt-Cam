#include "Data/Objects/FICDismantleZone.h"

#include "AbstractInstanceManager.h"
#include "FGLightweightBuildableSubsystem.h"
#include "FGMaterialEffect_Build.h"
#include "FICUtils.h"
#include "NiagaraComponent.h"
#include "Data/Objects/FICWorldSettings.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICParticleSystemSelection.h"
#include "Components/SphereComponent.h"
#include "NiagaraSystem.h"
#include "Buildables/FGBuildable.h"
#include "Components/LineBatchComponent.h"
#include "Editor/Data/FICEditorAttributeBase.h"
#include "Editor/Data/FICEditorAttributeBool.h"
#include "Engine/OverlapResult.h"
#include "Widgets/Input/SRotatorInputBox.h"
#include "Widgets/Input/SVectorInputBox.h"

AFICDismantleZoneActor::AFICDismantleZoneActor() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->InitBoxExtent(FVector(1000.0));
	Collision->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Collision->SetMobility(EComponentMobility::Movable);
	Collision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel10, ECollisionResponse::ECR_Block);
	Collision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFICDismantleZoneActor::Tick(float DeltaTime) {
	if (!IsValid(GetWorld())) return;
	auto LineBatcher = GetWorld()->GetLineBatcher(UWorld::ELineBatcherType::World);
	auto box = FBox(-DismantleZone->Size/2, DismantleZone->Size/2);
	auto transform = FTransform(DismantleZone->Rotation, DismantleZone->Position);
	LineBatcher->DrawBox(box, transform.ToMatrixWithScale(), FColor::Cyan, SDPG_World);
}

UObject* UFICDismantleZone::CreateNewObject(UObject* InOuter, AFICScene* InScene) {
	UFICDismantleZone* Object = NewObject<UFICDismantleZone>(InOuter);
	Object->SceneObjectName = UFICUtils::AdjustSceneObjectName(InScene, Object->SceneObjectName);
	Object->Active.SetDefaultValue(false);
	APlayerController* Player = InScene->GetWorld()->GetFirstPlayerController();
	FVector Pos = Player->PlayerCameraManager->GetCameraLocation();
	Object->Position = Pos;
	return Object;
}

TSharedRef<SWidget> UFICDismantleZone::CreateDetailsWidget(UFICEditorContext* InContext) {
	return SNew(SVerticalBox)
	+SVerticalBox::Slot().AutoHeight()[
		InContext->GetEditorAttributes()[this]->CreateDetailsWidget(InContext)
	]
	+SVerticalBox::Slot().AutoHeight()[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Position: ")))
		]
		+SHorizontalBox::Slot().FillWidth(1)[
			SNew( SNumericVectorInputBox<double>)
			.AllowSpin(true)
			.Vector_Lambda([this]() {
				return Position;
			})
			.OnXChanged_Lambda([this](double value) {
				Position.X = value;
			})
			.OnYChanged_Lambda([this](double value) {
				Position.Y = value;
			})
			.OnZChanged_Lambda([this](double value) {
				Position.Z = value;
			})
		]
	]
	+SVerticalBox::Slot().AutoHeight()[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Rotation: ")))
		]
		+SHorizontalBox::Slot().FillWidth(1)[
			SNew( SRotatorInputBox)
			.AllowSpin(true)
			.Pitch_Lambda([this]() { return Rotation.Pitch; })
			.Yaw_Lambda([this]() { return Rotation.Yaw; })
			.Roll_Lambda([this]() { return Rotation.Roll; })
			.OnPitchChanged_Lambda([this](double value) {
				Rotation.Pitch = value;
			})
			.OnYawChanged_Lambda([this](double value) {
				Rotation.Yaw = value;
			})
			.OnRollChanged_Lambda([this](double value) {
				Rotation.Roll = value;
			})
		]
	]
	+SVerticalBox::Slot().AutoHeight()[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Size: ")))
		]
		+SHorizontalBox::Slot().FillWidth(1)[
			SNew( SNumericVectorInputBox<double>)
			.AllowSpin(true)
			.Vector_Lambda([this]() {
				return Size;
			})
			.OnXChanged_Lambda([this](double value) {
				Size.X = value;
			})
			.OnYChanged_Lambda([this](double value) {
				Size.Y = value;
			})
			.OnZChanged_Lambda([this](double value) {
				Size.Z = value;
			})
		]
	]
	+SVerticalBox::Slot().AutoHeight()[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Duration in Frames: ")))
		]
		+SHorizontalBox::Slot().FillWidth(1)[
			SNew(SNumericEntryBox<int64>)
			.AllowSpin(true)
			.MinValue(0)
			.Value_Lambda([this]() {
				return DurationFrames;
			})
			.OnValueChanged_Lambda([this](int64 value) {
				DurationFrames = value;
			})
		]
	]
	+SVerticalBox::Slot().AutoHeight()[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Dismantle Effect: ")))
		]
		+SHorizontalBox::Slot().FillWidth(1)[
			SNew(SCheckBox)
			.IsChecked_Lambda([this]() {
				return bDismantleEffect ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([this](ECheckBoxState value) {
				bDismantleEffect = value == ECheckBoxState::Checked;
			})
		]
	];;
}

void UFICDismantleZone::InitEditor(UFICEditorContext* Context) {
	check(DismantleZoneActor == nullptr);
	DismantleZoneActor = GetWorld()->SpawnActor<AFICDismantleZoneActor>(Position, Rotation);
	DismantleZoneActor->DismantleZone = this;
	EditorContext = Context;

	UpdateActivation();
}

void UFICDismantleZone::ShutdownEditor(UFICEditorContext* Context) {
	check(DismantleZoneActor != nullptr);

	UpdateActivation();

	DismantleZoneActor->Destroy();
	DismantleZoneActor = nullptr;
	EditorContext = nullptr;
}

void UFICDismantleZone::EditorUpdate(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) {
	check(DismantleZoneActor != nullptr);

	UpdateActivation();
}

void UFICDismantleZone::Tick(float DeltaTime) {
	if (!IsValid(EditorContext)) return;

	if (AnimationProgression.IsSet()) {
		double progression = (EditorContext->GetCurrentFrame() - *AnimationProgression) / DurationFrames;
		HandleProgression(progression, true);
	} else {
		HandleProgression({}, true);
	}
}

void UFICDismantleZone::Select(UFICEditorContext* Context) {
	
}

void UFICDismantleZone::Unselect(UFICEditorContext* Context) {
	
}

void UFICDismantleZone::InitAnimation() {

}

UE_DISABLE_OPTIMIZATION_SHIP
void UFICDismantleZone::TickAnimation(FICFrameFloat Frame) {
	if (Active.GetValue(Frame)) {
		if (!AnimationProgression.IsSet()) {
			AnimationProgression = Frame;
		}
	} else {
		AnimationProgression.Reset();
	}

	if (AnimationProgression.IsSet()) {
		double progression = (Frame - *AnimationProgression) / DurationFrames;
		HandleProgression(progression);
	} else {
		HandleProgression({});
	}
}

void UFICDismantleZone::ShutdownAnimation() {
	AnimationProgression.Reset();

	HandleProgression({}); // Reset all dismantled things
}

FTransform UFICDismantleZone::GetSceneObjectTransform() {
	return FTransform(Rotation, Position, Size / 100);
}

void UFICDismantleZone::SetSceneObjectTransform(FTransform InTransform) {
	if (DismantleZoneActor) {
		DismantleZoneActor->SetActorTransform(InTransform);
	}
	Position = InTransform.GetLocation();
	Rotation = InTransform.GetRotation().Rotator();
	Size = InTransform.GetScale3D() * 100;
}

AActor* UFICDismantleZone::GetActor() {
	return DismantleZoneActor;
}

void UFICDismantleZone::HandleProgression(TOptional<double> Progression, bool bDrawProgression) {
	auto manager = AAbstractInstanceManager::GetInstanceManager(this);

	TSet<TTuple<UClass*, int64>> seenInstances;
	TSet<UObject*> seenObjects;

	if (Progression.IsSet()) {
		TArray<FOverlapResult> Overlaps;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(DismantleZoneActor);

		auto progression = FMath::Min(*Progression, 1.0);
		FVector direction = Rotation.Vector();
		FVector pos = Position - direction * (Size.X/2) * (1.0 - progression);
		FVector sizeCalc = (Size / 2.0);
		sizeCalc.X *= progression;
		FCollisionShape shape =  FCollisionShape::MakeBox(sizeCalc);
		auto query = ECollisionChannel::ECC_Visibility;
		GetWorld()->OverlapMultiByChannel(Overlaps, pos, Rotation.Quaternion(), query, shape, Params);

		if (bDrawProgression) {
			auto LineBatcher = GetWorld()->GetLineBatcher(UWorld::ELineBatcherType::World);
			auto box = FBox(-sizeCalc, sizeCalc);
			auto transform = FTransform(Rotation, pos);
			LineBatcher->DrawBox(box, transform.ToMatrixWithScale(), FColor::Red, SDPG_World);
		}

		for (const auto& result : Overlaps) {
			AFGBuildable* rootBuildable = nullptr;

			FInstanceHandle instance;
			if (manager->ResolveOverlap(result, instance)) {
				FLightweightBuildableInstanceRef buildableDescriptor;
				if (AFGLightweightBuildableSubsystem::ResolveLightweightInstance(instance, buildableDescriptor)) {
					TTuple<UClass*,int64> ID = {buildableDescriptor.GetBuildableClass(), instance.GetHandleID()};
					seenInstances.Add(ID);
					if (DismantledInstances.Contains(ID)) continue;
					DismantledInstances.Add(ID);
					if (AFGBuildable* buildable = buildableDescriptor.SpawnTemporaryBuildable()) {
						buildable->SetBlockCleanupOfTemporary(true);
						buildable->SetBuildableHiddenInGame(true);
						buildable->ToggleInstanceVisibility(false);
						if (bDismantleEffect) {
							buildable->PlayDismantleEffects();
							buildable->SetLifeSpan(0);
							if (buildable->mActiveBuildEffect) {
								buildable->mActiveBuildEffect->SetAutoDestroy(false);
								buildable->mActiveBuildEffect->mOnEnded.Unbind();
								buildable->mActiveBuildEffect->mOnEnded.BindLambda([buildable]() {
									buildable->SetActorHiddenInGame(true);
								});
							}
						}
						seenObjects.Add(buildable);
						DismantledObjects.Add(buildable, ID);
					}
				} else {
					rootBuildable = Cast<AFGBuildable>(instance.GetOwner());
				}
			} else {
				AActor* actor = result.GetActor();
				rootBuildable = Cast<AFGBuildable>(actor);
			}

			if (IsValid(rootBuildable)) {
				auto buildable = rootBuildable;

				seenObjects.Add(buildable);
				if (buildable->GetIsLightweightTemporary()) continue;
				if (DismantledObjects.Contains(buildable)) continue;

				buildable->SetActorHiddenInGame(true);
				buildable->ToggleInstanceVisibility(false);
				if (bDismantleEffect) {
					buildable->PlayDismantleEffects();
					buildable->SetLifeSpan(0);
					if (buildable->mActiveBuildEffect) {
						buildable->mActiveBuildEffect->SetAutoDestroy(false);
						buildable->mActiveBuildEffect->mOnEnded.Unbind();
						buildable->mActiveBuildEffect->mOnEnded.BindLambda([buildable]() {
							buildable->SetActorHiddenInGame(true);
						});
					}
				}
				buildable->SetActorEnableCollision(true);
				TInlineComponentArray<UPrimitiveComponent*> comps( buildable );
				for(UPrimitiveComponent* comp : comps) {
					comp->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
					if (comp->IsSimulatingPhysics()) {
						comp->WakeAllRigidBodies();
					}
				}
				DismantledObjects.Add(buildable, {nullptr,-1});
			}
		}
	}

	auto buildings = DismantledObjects;
	for (const auto& [obj, index] : buildings) {
		if (seenInstances.Contains(index)) continue;
		if (seenObjects.Contains(obj)) continue;

		AFGBuildable* buildable = Cast<AFGBuildable>(obj);
		if (!IsValid(buildable)) continue;

		if (IsValid(index.Key) && index.Value > INDEX_NONE) {
			/*bool bRestored = false;
			for (const FInstanceOwnerHandlePtr& handle : buildable->mInstanceHandles) {
				if (handle.IsValid() && handle->GetHandleID() == (uint32)index) {
					handle->UnHideInstance();
					bRestored = true;
					break;
				}
			}

			if (!bRestored) {
				if (AFGLightweightBuildableSubsystem* subsystem = AFGLightweightBuildableSubsystem::Get(this)) {
					if (FRuntimeBuildableInstanceData* runtimeData = subsystem->GetRuntimeDataForBuildableClassAndIndex(buildable->GetClass(), index)) {
						for (const FInstanceOwnerHandlePtr& handle : runtimeData->Handles) {
							if (handle.IsValid()) {
								handle->UnHideInstance();
							}
						}
					}
				}
			}*/

			UFGMaterialEffect_Build* effect = buildable->mActiveBuildEffect;
			if (effect) {
				buildable->mActiveBuildEffect = nullptr;
				effect->mOnEnded.Unbind();
				effect->Stop();
				effect->DestroyComponent();
			}
			//buildable->SetActorHiddenInGame(false);
			buildable->ToggleInstanceVisibility(true);
			//buildable->SetBuildableHiddenInGame(false);
			buildable->SetBlockCleanupOfTemporary(false);
		} else {
			UFGMaterialEffect_Build* effect = buildable->mActiveBuildEffect;
			if (effect) {
				buildable->mActiveBuildEffect = nullptr;
				effect->mOnEnded.Unbind();
				effect->Stop();
				effect->DestroyComponent();
			}
			buildable->SetActorHiddenInGame(false);
			buildable->ToggleInstanceVisibility(true);
		}

		DismantledInstances.Remove(index);
		DismantledObjects.Remove(obj);
	}
}

void UFICDismantleZone::UpdateActivation() {
	FICFrame activationFrame = 0;
	auto active = EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeBool>("Active").GetActiveValue();
	if (active) {
		auto keyframe = Active.GetPrevKeyframe(EditorContext->GetCurrentFrame(), activationFrame);
		if (keyframe.IsValid()) active = keyframe->GetValue() > 0.5;
	}
	if (active) {
		AnimationProgression = activationFrame;
	} else {
		AnimationProgression.Reset();
	}
}
UE_ENABLE_OPTIMIZATION_SHIP
