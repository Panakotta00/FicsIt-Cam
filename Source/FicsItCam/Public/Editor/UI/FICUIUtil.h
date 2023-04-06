#pragma once

#include "Editor/FICEditorContext.h"

FMenuBuilder FICCreateKeyframeTypeChangeMenu(UFICEditorContext* Context, TFunction<TSet<TPair<FFICAttribute*, FICFrame>>()> Keyframes);