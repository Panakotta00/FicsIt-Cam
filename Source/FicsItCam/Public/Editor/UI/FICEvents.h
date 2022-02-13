#pragma once

#include "CoreMinimal.h"
#include "Data/FICTypes.h"

DECLARE_DELEGATE_OneParam(FFICFrameChanged, FICFrame)
DECLARE_DELEGATE_OneParam(FFICValueChanged, FICValue)
DECLARE_DELEGATE_OneParam(FFICFrameRangeChanged, FFICFrameRange)
DECLARE_DELEGATE_OneParam(FFICValueRangeChanged, FFICValueRange)
