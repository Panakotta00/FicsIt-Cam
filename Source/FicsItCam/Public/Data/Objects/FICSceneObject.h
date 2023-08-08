#pragma once

#include "CoreMinimal.h"
#include "Data/FICTypes.h"
#include "Data/Attributes/FICAttributeGroup.h"
#include "UObject/Interface.h"
#include "FICSceneObject.generated.h"

class AFICScene;
class UFICEditorContext;

UINTERFACE()
class UFICSceneObject : public UInterface {
	GENERATED_BODY()
};

class FICSITCAM_API IFICSceneObject {
	GENERATED_BODY()
protected:
	FFICGroupAttribute RootAttribute;

public:
	virtual FString GetSceneObjectName() = 0;
	virtual void SetSceneObjectName(FString Name) = 0;
	
	virtual FFICAttribute& GetRootAttribute() { return RootAttribute; }
	virtual UObject* CreateNewObject(UObject* InOuter, AFICScene* InScene) = 0;
	virtual TSharedRef<SWidget> CreateDetailsWidget(UFICEditorContext* InContext) = 0;

	virtual void InitEditor(UFICEditorContext* Context) {}
	virtual void ShutdownEditor(UFICEditorContext* Context) {}
	virtual void TickEditor(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) {}
	virtual void EditorUpdate(UFICEditorContext* Context, TSharedRef<FFICEditorAttributeBase> Attribute) {}
	virtual void Select(UFICEditorContext* Context) {}
	virtual void Unselect(UFICEditorContext* Context) {}
	
	virtual void InitAnimation() {}
	virtual void ShutdownAnimation() {}
	virtual void TickAnimation(FICFrameFloat Frame) {}
};
