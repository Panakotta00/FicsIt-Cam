#include "Data/Objects/FICWorldSettings.h"

#include "FGCheatManager.h"
#include "FGGameState.h"
#include "FGPlayerController.h"
#include "FGTimeSubsystem.h"
#include "FICUtils.h"
#include "Editor/FICEditorContext.h"
#include "Editor/Data/FICEditorAttributeBase.h"

UFICWorldSettings::UFICWorldSettings() {
	RootAttribute.AddChildAttribute(TEXT("Active"), &Active);
	RootAttribute.AddChildAttribute(TEXT("Time of Day"), &Daytime);
}

UObject* UFICWorldSettings::CreateNewObject(UObject* InOuter, AFICScene* InScene) {
	UFICWorldSettings* Object = NewObject<UFICWorldSettings>(InOuter);
	Object->SceneObjectName = UFICUtils::AdjustSceneObjectName(InScene, Object->SceneObjectName);
	return Object;
}

TSharedRef<SWidget> UFICWorldSettings::CreateDetailsWidget(UFICEditorContext* InContext) {
	return InContext->GetEditorAttributes()[this]->CreateDetailsWidget(InContext);
}

void UFICWorldSettings::TickEditor(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) {
	UFGCheatManager* CheatManager = Cast<UFGCheatManager>(Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController())->CheatManager);
	if (bActive) {
		//int32 Time = FMath::RoundToInt(Attribute->Get<TFICEditorAttribute<FFICFloatAttribute>>(TEXT("Time of Day")).GetValue(Context->GetCurrentFrame()));
		//int32 Hour, Minute;
		//ClockFromInt(Time, Hour, Minute);
		//CheatManager->SetTimeOfDay(Hour, Minute);
		AFGTimeOfDaySubsystem::Get(this)->SetDaySeconds(60.0 * Attribute->Get<TFICEditorAttribute<FFICFloatAttribute>>(TEXT("Time of Day")).GetValue(Context->GetCurrentFrame()));
	}
}

void UFICWorldSettings::TickAnimation(FICFrameFloat Frame) {
	UFGCheatManager* CheatManager = Cast<UFGCheatManager>(Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController())->CheatManager);
	if (bActive) {
		//int32 Time = FMath::RoundToInt(Daytime.GetValue(Frame));
		//int32 Hour, Minute;
		//ClockFromInt(Time, Hour, Minute);
		//CheatManager->SetTimeOfDay(Hour, Minute);
		AFGTimeOfDaySubsystem::Get(this)->SetDaySeconds(60.0 * Daytime.GetValue(Frame));
	}
}

void UFICWorldSettings::Activate() {
	bActive = true;
	//UFGCheatManager* CheatManager = Cast<UFGCheatManager>(Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController())->CheatManager);
	//GetWorld()->GetFirstPlayerController()->EnableCheats();
	OldDaySeconds = AFGTimeOfDaySubsystem::Get(this)->GetDaySeconds();
	//OldTimeOfDay = IntFromClock(CheatManager->SetTimeOfDay_hour_Get(), CheatManager->SetTimeOfDay_minute_Get());
}

void UFICWorldSettings::Deactivate() {
	bActive = false;
	//UFGCheatManager* CheatManager = Cast<UFGCheatManager>(Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController())->CheatManager);
	//int32 Hour, Minute;
	//ClockFromInt(OldTimeOfDay, Hour, Minute);
	//CheatManager->SetTimeOfDay(Hour, Minute);
	AFGTimeOfDaySubsystem::Get(this)->SetDaySeconds(OldDaySeconds);
}

int32 UFICWorldSettings::IntFromClock(int32 Hours, int32 Minute) {
	return (Hours*60 + Minute) % (24*60);
}

void UFICWorldSettings::ClockFromInt(int32 Int, int32& Hours, int32& Minute) {
	Int = Int % (24*60);
	Minute = Int % 60;
	Hours = (Int / 60) % 24;
}
