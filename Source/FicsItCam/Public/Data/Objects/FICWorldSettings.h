#pragma once

#include "FGSaveInterface.h"
#include "FICSceneObject.h"
#include "FICSceneObjectActive.h"
#include "Data/Attributes/FICAttributeBool.h"
#include "Data/Attributes/FICAttributeFloat.h"
 #include "FICWorldSettings.generated.h"

UCLASS()
class UFICWorldSettings : public UObject, public IFICSceneObject, public IFICSceneObjectActive, public IFGSaveInterface {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FString SceneObjectName = TEXT("WorldSettings");
	UPROPERTY(SaveGame)
	FFICAttributeBool Active;
	UPROPERTY(SaveGame)
	FFICFloatAttribute Daytime;

	UPROPERTY()
	int32 OldTimeOfDay = 0;
	bool bActive = false;

	UFICWorldSettings();

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	// End IFGSaveInterface

	// Begin IFICSceneObject
	virtual UObject* CreateNewObject(UObject* InOuter, AFICScene* InScene) override;
	virtual TSharedRef<SWidget> CreateDetailsWidget(UFICEditorContext* InContext) override;
	virtual FString GetSceneObjectName() override { return SceneObjectName; }
	virtual void SetSceneObjectName(FString Name) override { SceneObjectName = Name; }
	virtual void TickEditor(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) override;
	virtual void TickAnimation(FICFrameFloat Frame) override;
	// End IFICSceneObject

	// Begin IFICWorldSettings
	virtual FString GetActiveType() override { return TEXT("WorldSettings"); }
	virtual FFICAttributeBool& GetActiveAttribute() override { return Active; };
	virtual void Activate() override;
	virtual void Deactivate() override;
	// End IFICWorldSettings

	int32 IntFromClock(int32 Hours, int32 Minute);
	void ClockFromInt(int32 Int, int32& Hours, int32& Minute);
};