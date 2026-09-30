#include "Common.h"
#include "ShellCommand.h"
#include "SelectionHelper.h"
#include "PathUtils.h"
#include "FileOperations.h"
#include "UI.h"
#include "Resource.h"
#include <iostream>

using namespace QuickFolder;

namespace {

bool g_hasConsole = false;
bool g_isSilent = false;

void OutputMessage(bool isError, const std::wstring& msg) {
    if (g_hasConsole) {
        HANDLE hOut = GetStdHandle(isError ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
        if (hOut && hOut != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteConsoleW(hOut, msg.c_str(), static_cast<DWORD>(msg.length()), &written, NULL);
            WriteConsoleW(hOut, L"\n", 1, &written, NULL);
            return;
        }
    }

    if (!g_isSilent) {
        if (isError) {
            UI::ShowErrorMessage(NULL, msg);
        } else {
            UI::ShowInfoMessage(NULL, msg);
        }
    }
}

void RunTestDialog() {
    UI::DialogParams params;
    params.hwndOwner = NULL;
    params.itemCount = 20;
    params.suggestedName = L"Birthday 2026";
    params.commonParent = L"D:\\Photos";

    bool res = UI::ShowQuickFolderDialog(params);
    if (res) {
        std::wstring msg = L"Dialog confirmed!\nChosen folder name: " + params.chosenFolderName;
        OutputMessage(false, msg);
    } else {
        OutputMessage(false, L"Dialog cancelled.");
    }
}

void ProcessDirectPaths(const std::vector<std::wstring>& paths, const std::wstring& explicitFolderName = L"") {
    auto analysis = QuickFolder::Selection::AnalyzePaths(paths);
    if (analysis.items.empty()) {
        OutputMessage(true, L"None of the specified items could be accessed.");
        return;
    }

    if (analysis.hasDifferentParents) {
        OutputMessage(true, UI::GetString(IDS_ERR_DIFFERENT_PARENTS));
        return;
    }

    std::wstring suggested = explicitFolderName;
    if (suggested.empty() && analysis.items.size() == 1) {
        suggested = QuickFolder::PathUtils::SuggestFolderNameForSingleItem(
            analysis.items[0].path, analysis.items[0].isDirectory
        );
    }

    std::vector<std::wstring> sourcePaths;
    for (const auto& item : analysis.items) {
        sourcePaths.push_back(item.path);
    }

    // If explicit folder name provided, move directly without modal UI
    if (!explicitFolderName.empty()) {
        std::wstring validationErr;
        auto valRes = QuickFolder::PathUtils::ValidateFolderName(explicitFolderName, &validationErr);
        if (valRes != QuickFolder::PathUtils::ValidationResult::Valid) {
            OutputMessage(true, QuickFolder::PathUtils::GetValidationErrorMessage(valRes));
            return;
        }

        std::wstring destPath = QuickFolder::PathUtils::CombinePath(analysis.commonParent, explicitFolderName);
        for (const auto& item : analysis.items) {
            if (QuickFolder::PathUtils::IsSubdirectoryOf(destPath, item.path)) {
                OutputMessage(true, UI::GetString(IDS_ERR_DEST_INSIDE_SELECTION));
                return;
            }
        }

        bool exists = QuickFolder::PathUtils::PathExists(destPath);
        auto moveRes = QuickFolder::FileOps::ExecuteMove(NULL, sourcePaths, destPath, exists);
        if (moveRes.success) {
            QuickFolder::UI::AddRecentFolder(explicitFolderName);
        } else if (!moveRes.cancelled) {
            std::wstring err = UI::GetString(IDS_ERR_MOVE_FAILED);
            if (!moveRes.errorMessage.empty()) {
                err += L"\n" + moveRes.errorMessage;
            }
            OutputMessage(true, err);
        }
        return;
    }

    while (true) {
        UI::DialogParams params;
        params.hwndOwner = NULL;
        params.itemCount = analysis.items.size();
        params.suggestedName = suggested;
        params.commonParent = analysis.commonParent;

        bool confirmed = UI::ShowQuickFolderDialog(params);
        if (!confirmed || params.chosenFolderName.empty()) {
            break;
        }

        std::wstring destPath = QuickFolder::PathUtils::CombinePath(analysis.commonParent, params.chosenFolderName);

        bool recursive = false;
        for (const auto& item : analysis.items) {
            if (QuickFolder::PathUtils::IsSubdirectoryOf(destPath, item.path)) {
                recursive = true;
                break;
            }
        }
        if (recursive) {
            OutputMessage(true, UI::GetString(IDS_ERR_DEST_INSIDE_SELECTION));
            suggested = params.chosenFolderName;
            continue;
        }

        bool exists = QuickFolder::PathUtils::PathExists(destPath);
        auto moveRes = QuickFolder::FileOps::ExecuteMove(NULL, sourcePaths, destPath, exists);
        if (moveRes.success) {
            QuickFolder::UI::AddRecentFolder(params.chosenFolderName);
        } else if (!moveRes.cancelled) {
            std::wstring err = UI::GetString(IDS_ERR_MOVE_FAILED);
            if (!moveRes.errorMessage.empty()) {
                err += L"\n" + moveRes.errorMessage;
            }
            OutputMessage(true, err);
        }
        break;
    }
}

} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        g_hasConsole = true;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        return 1;
    }

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    bool isEmbedding = false;
    bool isRegister = false;
    bool isUnregister = false;
    bool isVersion = false;
    bool isTestDialog = false;
    std::wstring testPath;
    std::wstring explicitFolderName;
    std::vector<std::wstring> directPaths;

    for (int i = 1; i < argc; ++i) {
        std::wstring arg = argv[i];
        if (_wcsicmp(arg.c_str(), L"-Embedding") == 0 || _wcsicmp(arg.c_str(), L"/Embedding") == 0) {
            isEmbedding = true;
        } else if (_wcsicmp(arg.c_str(), L"--silent") == 0 || _wcsicmp(arg.c_str(), L"-s") == 0 || _wcsicmp(arg.c_str(), L"/s") == 0) {
            g_isSilent = true;
        } else if (_wcsicmp(arg.c_str(), L"-su") == 0 || _wcsicmp(arg.c_str(), L"/su") == 0) {
            g_isSilent = true;
            isUnregister = true;
        } else if (_wcsicmp(arg.c_str(), L"--register") == 0 || _wcsicmp(arg.c_str(), L"-register") == 0 || _wcsicmp(arg.c_str(), L"/register") == 0) {
            isRegister = true;
        } else if (_wcsicmp(arg.c_str(), L"--unregister") == 0 || _wcsicmp(arg.c_str(), L"-unregister") == 0 || _wcsicmp(arg.c_str(), L"/unregister") == 0) {
            isUnregister = true;
        } else if (_wcsicmp(arg.c_str(), L"--version") == 0 || _wcsicmp(arg.c_str(), L"-v") == 0 || _wcsicmp(arg.c_str(), L"/version") == 0) {
            isVersion = true;
        } else if (_wcsicmp(arg.c_str(), L"--test-dialog") == 0) {
            isTestDialog = true;
        } else if (_wcsicmp(arg.c_str(), L"--test-path") == 0 && i + 1 < argc) {
            testPath = argv[++i];
        } else if ((_wcsicmp(arg.c_str(), L"--folder-name") == 0 || _wcsicmp(arg.c_str(), L"-f") == 0) && i + 1 < argc) {
            explicitFolderName = argv[++i];
        } else if (arg.rfind(L"@", 0) == 0 || (_wcsicmp(arg.c_str(), L"--file-list") == 0 && i + 1 < argc)) {
            std::wstring listPath = (arg.rfind(L"@", 0) == 0) ? arg.substr(1) : argv[++i];
            FILE* fp = _wfopen(listPath.c_str(), L"r, ccs=UTF-8");
            if (fp) {
                wchar_t lineBuf[1024];
                while (fgetws(lineBuf, 1024, fp)) {
                    std::wstring line = lineBuf;
                    while (!line.empty() && (line.back() == L'\r' || line.back() == L'\n' || line.back() == L' ')) {
                        line.pop_back();
                    }
                    if (!line.empty()) {
                        directPaths.push_back(line);
                    }
                }
                fclose(fp);
            }
        } else if (arg.rfind(L"-", 0) != 0 && arg.rfind(L"/", 0) != 0) {
            directPaths.push_back(arg);
        }
    }

    if (argv) {
        LocalFree(argv);
    }

    // Version flag
    if (isVersion) {
        std::wstring verMsg = std::wstring(QUICKFOLDER_APP_NAME) + L" v" + QUICKFOLDER_VERSION +
            L"\nHigh-performance native Windows utility.\nhttps://github.com/QuickFolder/QuickFolder";
        OutputMessage(false, verMsg);
        CoUninitialize();
        return 0;
    }

    // Register flag
    if (isRegister) {
        HRESULT regHr = QuickFolder::Shell::RegisterServer(true);
        if (SUCCEEDED(regHr)) {
            OutputMessage(false, L"QuickFolder context menu registered successfully.");
        } else {
            OutputMessage(true, L"Failed to register QuickFolder context menu.");
        }
        CoUninitialize();
        return SUCCEEDED(regHr) ? 0 : 1;
    }

    // Unregister flag
    if (isUnregister) {
        HRESULT unregHr = QuickFolder::Shell::UnregisterServer(true);
        if (SUCCEEDED(unregHr)) {
            OutputMessage(false, L"QuickFolder context menu unregistered successfully.");
        } else {
            OutputMessage(true, L"Failed to unregister QuickFolder context menu.");
        }
        CoUninitialize();
        return SUCCEEDED(unregHr) ? 0 : 1;
    }

    // Test Dialog flag
    if (isTestDialog) {
        RunTestDialog();
        CoUninitialize();
        return 0;
    }

    // Test path flag
    if (!testPath.empty()) {
        ProcessDirectPaths({ testPath }, explicitFolderName);
        CoUninitialize();
        return 0;
    }

    // Direct paths passed
    if (!directPaths.empty()) {
        ProcessDirectPaths(directPaths, explicitFolderName);
        CoUninitialize();
        return 0;
    }

    // COM Local Server mode
    if (isEmbedding) {
        auto* factory = new (std::nothrow) QuickFolder::Shell::CQuickFolderClassFactory();
        if (!factory) {
            CoUninitialize();
            return 1;
        }

        DWORD dwCookie = 0;
        HRESULT regHr = CoRegisterClassObject(
            CLSID_QuickFolderCommand,
            factory,
            CLSCTX_LOCAL_SERVER,
            REGCLS_MULTIPLEUSE,
            &dwCookie
        );

        if (SUCCEEDED(regHr)) {
            // Standard COM message loop
            MSG msg = {};
            while (GetMessageW(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            CoRevokeClassObject(dwCookie);
        }

        factory->Release();
        CoUninitialize();
        return 0;
    }

    // If launched with no arguments, show informational usage dialog
    std::wstring infoMsg = std::wstring(QUICKFOLDER_APP_NAME) + L" v" + QUICKFOLDER_VERSION +
        L"\n\nQuickFolder is integrated directly into Windows Explorer.\n\n"
        L"To use it:\n"
        L"1. Select files or folders in Windows Explorer.\n"
        L"2. Right-click and select 'Move to new folder...'.\n"
        L"3. Type the folder name and press Enter.\n\n"
        L"Commands:\n"
        L"  --register       Register context menu for current user\n"
        L"  --unregister     Unregister context menu\n"
        L"  --silent         Silent operation (used with --register or --unregister)\n"
        L"  --test-dialog    Preview the input dialog\n"
        L"  --version        Show version information";

    OutputMessage(false, infoMsg);

    CoUninitialize();
    return 0;
}
