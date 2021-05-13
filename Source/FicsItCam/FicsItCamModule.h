#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItCam, Log, Log);

#define FIC_ModRef "FicsItCam"

class FFicsItCamModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }
};