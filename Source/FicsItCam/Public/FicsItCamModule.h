#pragma once

#include "NativeGameplayTags.h"
#include "Modules/ModuleManager.h"
#include "Patching/NativeHookManager.h"
#include "Util/FICSceneViewExtension.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItCam, Verbose, All);

#define FIC_ModRef "FicsItCam"

static FIntPoint FIC_LastViewSize;

class FFicsItCamModule : public FDefaultGameModuleImpl {
public:
	TSharedPtr<FFICSceneViewExtension> SceneViewExtension;
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return true; }

	static void AddPickup_Override(CallScope<void(*)(class AFGItemRegrowSubsystem*, class AFGItemPickup*)>& Scope, class AFGItemRegrowSubsystem* self, class AFGItemPickup* Item);
};