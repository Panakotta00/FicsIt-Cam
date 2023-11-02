#pragma once

#include "NativeGameplayTags.h"
#include "Modules/ModuleManager.h"
#include "Patching/NativeHookManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItCam, Verbose, All);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_FIC_OpenMenu)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_FIC_Editor_Movement)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_FIC_Editor_Rotation)

#define FIC_ModRef "FicsItCam"

static FIntPoint FIC_LastViewSize;

class FFicsItCamModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return true; }

	static void AddPickup_Override(CallScope<void(*)(class AFGItemRegrowSubsystem*, class AFGItemPickup*)>& Scope, class AFGItemRegrowSubsystem* self, class AFGItemPickup* Item);
};