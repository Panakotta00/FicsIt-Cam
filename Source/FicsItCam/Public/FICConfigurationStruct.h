#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "FICConfigurationStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/FicsItCam/FICConfiguration' */
USTRUCT(BlueprintType)
struct FFICConfigurationStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float DPIScaling{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FFICConfigurationStruct GetActiveConfig(UObject* WorldContext) {
        FFICConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"FicsItCam", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FFICConfigurationStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

