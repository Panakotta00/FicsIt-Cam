#pragma once

#include "CoreMinimal.h"

struct FFICWindowsUtils {
	static bool OpenDirectoryDialog(const void* ParentWindowHandle, const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName);
	static bool SaveFileDialog(const void* ParentWindowHandle, const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FString>& OutFileName);
};