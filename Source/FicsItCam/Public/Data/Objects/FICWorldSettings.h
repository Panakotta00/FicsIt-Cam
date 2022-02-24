#pragma once

#include "FICSceneObject.h"
#include "Data/Attributes/FICAttributeBool.h"
#include "Data/Attributes/FICAttributeFloat.h"
#include "Data/Attributes/FICAttributeGroup.h"
#include "FICWorldSettings.generated.h"

UCLASS()
class UFICWorldSettings : public UObject, public IFICSceneObject {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FString SceneObjectName = TEXT("World Settings");
	UPROPERTY(SaveGame)
	FFICAttributeBool Active;
	UPROPERTY(SaveGame)
	FFICFloatAttribute Daytime;

	UPROPERTY()
	FFICGroupAttribute RootAttribute;

	UPROPERTY()
	int32 OldTimeOfDay = 0;

	UFICWorldSettings();

	// Begin IFICSceneObject
	virtual FString GetSceneObjectName() override { return SceneObjectName; }
	virtual void SetSceneObjectName(FString Name) override { SceneObjectName = Name; }
	virtual FFICAttribute& GetRootAttribute() override { return RootAttribute; }
	virtual void InitEditor(UFICEditorContext* Context) override;
	virtual void UnloadEditor(UFICEditorContext* Context) override;
	virtual void EditorUpdate(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) override;
	// End IFICSceneObject

	int32 IntFromClock(int32 Hours, int32 Minute);
	void ClockFromInt(int32 Int, int32& Hours, int32& Minute);
};