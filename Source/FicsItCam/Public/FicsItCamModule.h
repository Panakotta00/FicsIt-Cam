#pragma once

#include "Modules/ModuleManager.h"
#include "Patching/NativeHookManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItCam, Log, Log);

#define FIC_ModRef "FicsItCam"

static FIntPoint FIC_LastViewSize;

class FFicsItCamModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	static void AddPickup_Override(CallScope<void(*)(class AFGItemRegrowSubsystem*, class AFGItemPickup*)>& Scope, class AFGItemRegrowSubsystem* self, class AFGItemPickup* Item);
};