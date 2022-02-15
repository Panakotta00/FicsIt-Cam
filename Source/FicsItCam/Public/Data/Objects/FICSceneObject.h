#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FICSceneObject.generated.h"

UINTERFACE()
class UFICSceneObject : public UInterface {
	GENERATED_BODY()
};

class FICSITCAM_API IFICSceneObject {
	GENERATED_BODY()

public:
	virtual FText GetSceneObjectName() = 0;
	
	virtual FFICAttribute& GetRootAttribute() = 0;
};
