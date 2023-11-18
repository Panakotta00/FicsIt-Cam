#pragma once

#include "CoreMinimal.h"

template<typename T>
T& GetStructFromScope(const FStructOnScope& Scope) {
	check(Scope.GetStruct()->IsChildOf<T>())
	return *static_cast<T*>(Scope.GetStructMemory());
}
