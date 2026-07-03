#include "Editor/UI/FICUIUtil.h"

#include "Framework/Application/SlateApplication.h"

FMenuBuilder FICCreateKeyframeTypeChangeMenu(UFICEditorContext* Context, TFunction<TSet<TPair<FFICAttribute*, FICFrame>>()> GetKeyframes) {
	TFunction<void(EFICKeyframeType)> SetKeyframeType;
	SetKeyframeType = [GetKeyframes, Context](EFICKeyframeType Type) {
		TSharedRef<FFICChange_Group> Group = MakeShared<FFICChange_Group>();
		TMap<FFICAttribute*, TSharedRef<FFICAttribute>> Snapshots;
		TSet<TTuple<FFICAttribute*, long long>> Keyframes = GetKeyframes();
		for (const TPair<FFICAttribute*, FICFrame>& KF : Keyframes) {
			TSharedRef<FFICAttribute>* Snapshot = Snapshots.Find(KF.Key);
			if (!Snapshot) Snapshots.Add(KF.Key, KF.Key->CreateCopy());
			TMap<FICFrame, TSharedRef<FFICKeyframe>> KFS = KF.Key->GetKeyframes();
			TSharedRef<FFICKeyframe>* NKF = KFS.Find(KF.Value);
			if (NKF) (*NKF)->SetType(Type);
			KF.Key->LockUpdateEvent();
			KF.Key->RecalculateAllKeyframes();
			KF.Key->UnlockUpdateEvent(false);
		}
		for (const TPair<FFICAttribute*, TSharedRef<FFICAttribute>>& Snapshot : Snapshots) {
			Group->PushChange(MakeShared<FFICChange_Attribute>(Snapshot.Key, Snapshot.Value));
		}
		Context->ChangeList.PushChange(Group);
		for (const TPair<FFICAttribute*, FICFrame>& KF : Keyframes) KF.Key->OnUpdate.Broadcast();
	};

	TSharedPtr<IMenu> MenuHandle;
	FMenuBuilder MenuBuilder(true, NULL);
	MenuBuilder.AddMenuEntry(
        FText::FromString("Ease"),
        FText(),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([SetKeyframeType]() {
            SetKeyframeType(FIC_KF_EASE);
        }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
	MenuBuilder.AddMenuEntry(
        FText::FromString("Ease-In/Out"),
        FText(),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([SetKeyframeType]() {
            SetKeyframeType(FIC_KF_EASEINOUT);
        }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
	MenuBuilder.AddMenuEntry(
        FText::FromString("Linear"),
        FText(),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([SetKeyframeType]() {
            SetKeyframeType(FIC_KF_LINEAR);
        }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));
	MenuBuilder.AddMenuEntry(
        FText::FromString("Step"),
        FText(),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([SetKeyframeType]() {
            SetKeyframeType(FIC_KF_STEP);
        }), FCanExecuteAction::CreateRaw(&FSlateApplication::Get(), &FSlateApplication::IsNormalExecution)));

	return MenuBuilder;
}

#if PLATFORM_WINDOWS
#include "Microsoft/MinWindows.h"
#include "Microsoft/COMPointer.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <LM.h>
#include "Windows/HideWindowsPlatformTypes.h"

#pragma comment( lib, "version.lib" )

TOptional<FString> FileDialog(bool bSave, const FString& Title, const FString& DefaultFile, const FString& DefaultPath) {
	TComPtr<IFileDialog> FileDialog;
	if (SUCCEEDED(::CoCreateInstance(
		bSave ? CLSID_FileSaveDialog : CLSID_FileOpenDialog,
		nullptr,
		CLSCTX_INPROC_SERVER,
		bSave ? IID_IFileSaveDialog : IID_IFileOpenDialog,
		IID_PPV_ARGS_Helper(&FileDialog)
	))) {
		FileDialog->SetTitle(*Title);
		if (!DefaultFile.IsEmpty()) {
			FileDialog->SetFileName(*DefaultFile);
		}

		if (!DefaultPath.IsEmpty()) {
			FString DefaultWindowsPath = FPaths::ConvertRelativePathToFull(DefaultPath);
			DefaultWindowsPath.ReplaceInline(TEXT("/"), TEXT("\\"), ESearchCase::CaseSensitive);
			TComPtr<IShellItem> DefaultPathItem;
			if (SUCCEEDED(::SHCreateItemFromParsingName(*DefaultWindowsPath, nullptr, IID_PPV_ARGS(&DefaultPathItem)))) {
				FileDialog->SetFolder(DefaultPathItem);
			}
		}

		COMDLG_FILTERSPEC rgSpec[] = {
			{ TEXT("FicsIt-Cam Scene"), L"*.fic_scene" },
		};
		FileDialog->SetFileTypes(1, rgSpec);
		FileDialog->SetDefaultExtension(TEXT("fic_scene"));
		if (SUCCEEDED(FileDialog->Show(nullptr))) {
			TComPtr<IShellItem> Result;
			if (SUCCEEDED(FileDialog->GetResult(&Result))) {
				PWSTR pFilePath = nullptr;
				if (SUCCEEDED(Result->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath))) {
					FString filePath = pFilePath;
					::CoTaskMemFree(pFilePath);
					FPaths::NormalizeFilename(filePath);
					return filePath;
				}
			}
		}
	}
	return {};
}
#else
TOptional<FString> FileDialog(bool bSave, const FString& Title, const FString& DefaultFile, const FString& DefaultPath) {
	return {};
}
#endif

TOptional<FString> FICSaveSceneFileDialog(const FString& SceneName) {
	FString DefaultFile = FString::Printf(TEXT("%ls.fic_scene"), *SceneName);
	FString DefaultPath = FPlatformProcess::UserDir();

	return FileDialog(true, TEXT("Export FicsIt-Cam Scene..."), DefaultFile, DefaultPath);
}

TOptional<FString> FICOpenSceneFileDialog() {
	FString DefaultPath = FPlatformProcess::UserDir();
	return FileDialog(false, TEXT("Import FicsIt-Cam Scene..."), FString(), DefaultPath);
}