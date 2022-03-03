#include "Data/Objects/FICParticleSystem.h"

#include "Data/Objects/FICWorldSettings.h"
#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICParticleSystemSelection.h"

AFICParticleSystemActor::AFICParticleSystemActor() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(40.f);
	Collision->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Collision->SetMobility(EComponentMobility::Movable);
	Collision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel10, ECollisionResponse::ECR_Block);
	Collision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
}

void UFICParticleSystem::Tick(float DeltaTime) {
	
}

UObject* UFICParticleSystem::CreateNewObject(UObject* InOuter, AFICScene* InScene) {
	UFICParticleSystem* Object = NewObject<UFICParticleSystem>(InOuter);
	APlayerController* Player = InScene->GetWorld()->GetFirstPlayerController(); 
	FVector Pos = Player->PlayerCameraManager->GetCameraLocation();
	FRotator Rot = Player->PlayerCameraManager->GetCameraRotation();
	Object->Position.SetDefaultValue(Pos);
	Object->Rotation.SetDefaultValue(Rot);
	Object->Active.SetDefaultValue(true);
	return Object;
}

TSharedRef<SWidget> UFICParticleSystem::CreateDetailsWidget(UFICEditorContext* InContext) {
	return SNew(SVerticalBox)
	+SVerticalBox::Slot().AutoHeight()[
		InContext->GetEditorAttributes()[this]->CreateDetailsWidget(InContext)
	]
	+SVerticalBox::Slot().AutoHeight()[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Particle System Type: ")))
		]
		+SHorizontalBox::Slot().FillWidth(1)[
			SNew(SFICParticleSystemSelection, EditorContext)
			.OnSelectionChanged_Lambda([this](UParticleSystem* System) {
				SetParticleSystem(System);
			})
		]
	];
}

void UFICParticleSystem::InitEditor(UFICEditorContext* Context) {
	FVector Pos = Position.Get(Context->GetCurrentFrame());
	FRotator Rot = Rotation.Get(Context->GetCurrentFrame());
	ParticleSystemActor = GetWorld()->SpawnActor<AFICParticleSystemActor>(Pos, Rot);
	EditorContext = Context;
	ParticleSystemActor->ParticleSystemComponent->SetTemplate(ParticleSystem);
	ParticleSystemActor->ParticleSystem = this;
}

void UFICParticleSystem::ShutdownEditor(UFICEditorContext* Context) {
	ParticleSystemActor->Destroy();
	ParticleSystemActor = nullptr;
}

void UFICParticleSystem::EditorUpdate(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) {
	FVector Pos = FFICAttributePosition::FromEditorAttribute(Attribute->Get<FFICEditorAttributeGroup>("Position"));
	FRotator Rot = FFICAttributeRotation::FromEditorAttribute(Attribute->Get<FFICEditorAttributeGroup>("Rotation"));
	bool bActive = Attribute->Get<FFICEditorAttributeBool>("Active").GetActiveValue(); 
	ParticleSystemActor->SetActorLocationAndRotation(Pos, Rot);
	ParticleSystemActor->ParticleSystemComponent->SetActive(bActive);
}

void UFICParticleSystem::Select(UFICEditorContext* Context) {
	
}

void UFICParticleSystem::Unselect(UFICEditorContext* Context) {
	
}

void UFICParticleSystem::InitAnimation() {
	FVector Pos = Position.Get(0);
	FRotator Rot = Rotation.Get(0);
	bool bActive = Active.GetValue(0); 
	ParticleSystemActor = GetWorld()->SpawnActor<AFICParticleSystemActor>(Pos, Rot);
	ParticleSystemActor->ParticleSystemComponent->SetTemplate(ParticleSystem);
}

void UFICParticleSystem::TickAnimation(FICFrameFloat Frame) {
	FVector Pos = Position.Get(Frame);
	FRotator Rot = Rotation.Get(Frame);
	bool bActive = Active.GetValue(Frame); 
	ParticleSystemActor->SetActorLocationAndRotation(Pos, Rot);
	ParticleSystemActor->ParticleSystemComponent->SetActive(bActive);
}

void UFICParticleSystem::ShutdownAnimation() {
	ParticleSystemActor->Destroy();
	ParticleSystemActor = nullptr;
}

FTransform UFICParticleSystem::GetSceneObjectTransform() {
	FVector Pos = FFICAttributePosition::FromEditorAttribute( EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Position"));
	FRotator Rot = FFICAttributeRotation::FromEditorAttribute(EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Rotation"));
	return FTransform(Rot, Pos);
}

void UFICParticleSystem::SetSceneObjectTransform(FTransform InTransform) {
	if (ParticleSystemActor) {
		ParticleSystemActor->SetActorTransform(InTransform);
	}
	EditorContext->CommitAutoKeyframe(this);
	FFICAttributePosition::ToEditorAttribute(InTransform.GetLocation(), EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Position"));
	FFICAttributeRotation::ToEditorAttribute(InTransform.GetRotation().Rotator(), EditorContext->GetEditorAttributes()[this]->Get<FFICEditorAttributeGroup>("Rotation"));
	EditorContext->CommitAutoKeyframe(nullptr);
}

void UFICParticleSystem::SetParticleSystem(UParticleSystem* InParticleSystem) {
	ParticleSystem = InParticleSystem;
	if (ParticleSystemActor) {
		ParticleSystemActor->ParticleSystemComponent->SetTemplate(ParticleSystem);
		ParticleSystemActor->ParticleSystemComponent->Activate(true);
	}
}
