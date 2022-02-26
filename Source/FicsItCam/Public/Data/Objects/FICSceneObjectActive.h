#pragma once

#include "InteractiveGizmo.h"
#include "Data/Attributes/FICAttributeBool.h"
#include "FICSceneObjectActive.generated.h"

UINTERFACE()
class UFICSceneObjectActive : public UInterface {
	GENERATED_BODY()
};

class FICSITCAM_API IFICSceneObjectActive {
	GENERATED_BODY()
public:
	virtual FString GetActiveType() = 0;
	virtual FFICAttributeBool& GetActiveAttribute() = 0;
	virtual void Activate() {}
	virtual void Deactivate() {}
};
