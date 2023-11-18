#pragma once

#include "CoreMinimal.h"
#include "FICRenderer.generated.h"

USTRUCT()
struct FFICRenderer {
	GENERATED_BODY()
private:
	FSceneViewStateReference ViewState;

public:
	~FFICRenderer();
	
	void AddStructReferencedObjects(FReferenceCollector& ReferenceCollector);
	
	void Render(FRenderTarget* RenderTarget, FMinimalViewInfo ViewInfo, UWorld* World);
};

template<>
struct TStructOpsTypeTraits<FFICRenderer> : TStructOpsTypeTraitsBase2<FFICRenderer> {
	enum {
		WithAddStructReferencedObjects = true,
	};
};