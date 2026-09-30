#include "ShellCommand.h"
#include "SelectionHelper.h"
#include "PathUtils.h"
#include "FileOperations.h"
#include "UI.h"
#include "Resource.h"

namespace QuickFolder::Shell {

namespace {

bool SetRegString(HKEY hRoot, const std::wstring& subkey, const std::wstring& valueName, const std::wstring& data) {
    HKEY hKey;
    if (RegCreateKeyExW(hRoot, subkey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExW(hKey, valueName.empty() ? NULL : valueName.c_str(), 0, REG_SZ,
            reinterpret_cast<const BYTE*>(data.c_str()), static_cast<DWORD>((data.length() + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

std::wstring GetExePath() {
    wchar_t path[MAX_PATH * 2] = {};
    GetModuleFileNameW(NULL, path, ARRAYSIZE(path));
    return path;
}

} // namespace

// ============================================================================
// CQuickFolderCommand
// ============================================================================

CQuickFolderCommand::CQuickFolderCommand() {
}

CQuickFolderCommand::~CQuickFolderCommand() {
    if (m_pSelection) {
        m_pSelection->Release();
        m_pSelection = nullptr;
    }
    if (m_pSite) {
        m_pSite->Release();
        m_pSite = nullptr;
    }
}

IFACEMETHODIMP CQuickFolderCommand::QueryInterface(REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (riid == IID_IUnknown) {
        *ppv = static_cast<IUnknown*>(static_cast<IExecuteCommand*>(this));
    } else if (riid == IID_IExecuteCommand) {
        *ppv = static_cast<IExecuteCommand*>(this);
    } else if (riid == IID_IObjectWithSelection) {
        *ppv = static_cast<IObjectWithSelection*>(this);
    } else if (riid == IID_IObjectWithSite) {
        *ppv = static_cast<IObjectWithSite*>(this);
    } else {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

IFACEMETHODIMP_(ULONG) CQuickFolderCommand::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

IFACEMETHODIMP_(ULONG) CQuickFolderCommand::Release() {
    ULONG rc = InterlockedDecrement(&m_refCount);
    if (rc == 0) {
        delete this;
    }
    return rc;
}

// IObjectWithSelection
IFACEMETHODIMP CQuickFolderCommand::SetSelection(IShellItemArray* psiItemArray) {
    if (m_pSelection) {
        m_pSelection->Release();
        m_pSelection = nullptr;
    }

    m_pSelection = psiItemArray;
    if (m_pSelection) {
        m_pSelection->AddRef();
    }
    return S_OK;
}

IFACEMETHODIMP CQuickFolderCommand::GetSelection(REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;
    if (m_pSelection) {
        return m_pSelection->QueryInterface(riid, ppv);
    }
    return E_NOINTERFACE;
}

// IExecuteCommand
IFACEMETHODIMP CQuickFolderCommand::SetKeyState(DWORD grfKeyState) {
    m_keyState = grfKeyState;
    return S_OK;
}

IFACEMETHODIMP CQuickFolderCommand::SetParameters(LPCWSTR) {
    return S_OK;
}

IFACEMETHODIMP CQuickFolderCommand::SetPosition(POINT) {
    return S_OK;
}

IFACEMETHODIMP CQuickFolderCommand::SetShowWindow(int) {
    return S_OK;
}

IFACEMETHODIMP CQuickFolderCommand::SetNoShowUI(BOOL) {
    return S_OK;
}

IFACEMETHODIMP CQuickFolderCommand::SetDirectory(LPCWSTR) {
    return S_OK;
}

IFACEMETHODIMP CQuickFolderCommand::Execute() {
    if (!m_pSelection) {
        PostQuitMessage(0);
        return S_OK;
    }

    auto analysis = Selection::AnalyzeShellItems(m_pSelection);
    if (analysis.items.empty()) {
        PostQuitMessage(0);
        return S_OK;
    }

    if (analysis.hasDifferentParents) {
        UI::ShowErrorMessage(m_hwndOwner, UI::GetString(IDS_ERR_DIFFERENT_PARENTS));
        PostQuitMessage(0);
        return S_OK;
    }

    // Determine initial suggestion
    std::wstring suggested;
    if (analysis.items.size() == 1) {
        suggested = PathUtils::SuggestFolderNameForSingleItem(
            analysis.items[0].path, analysis.items[0].isDirectory
        );
    }

    std::vector<std::wstring> sourcePaths;
    sourcePaths.reserve(analysis.items.size());
    for (const auto& it : analysis.items) {
        sourcePaths.push_back(it.path);
    }

    // Workflow loop (supports returning to dialog if "Choose another name" is selected)
    while (true) {
        UI::DialogParams params;
        params.hwndOwner = m_hwndOwner;
        params.itemCount = analysis.items.size();
        params.suggestedName = suggested;
        params.commonParent = analysis.commonParent;

        bool confirmed = UI::ShowQuickFolderDialog(params);
        if (!confirmed || params.chosenFolderName.empty()) {
            break; // User cancelled
        }

        std::wstring destPath = PathUtils::CombinePath(analysis.commonParent, params.chosenFolderName);

        // Check if destination is equal to or inside any of the selected items (prevent recursive move)
        bool recursiveMove = false;
        for (const auto& item : analysis.items) {
            if (PathUtils::IsSubdirectoryOf(destPath, item.path)) {
                recursiveMove = true;
                break;
            }
        }
        if (recursiveMove) {
            UI::ShowErrorMessage(m_hwndOwner, UI::GetString(IDS_ERR_DEST_INSIDE_SELECTION));
            suggested = params.chosenFolderName;
            continue;
        }

        // Check if destination folder already exists (merges silently without folder-level prompt)
        bool exists = PathUtils::PathExists(destPath);

        // Execute Move using IFileOperation (prompts only if files collide)
        auto moveResult = FileOps::ExecuteMove(m_hwndOwner, sourcePaths, destPath, exists);
        if (moveResult.success) {
            UI::AddRecentFolder(params.chosenFolderName);
        } else if (!moveResult.cancelled) {
            std::wstring err = UI::GetString(IDS_ERR_MOVE_FAILED);
            if (!moveResult.errorMessage.empty()) {
                err += L"\n" + moveResult.errorMessage;
            }
            UI::ShowErrorMessage(m_hwndOwner, err);
        }
        break;
    }

    PostQuitMessage(0);
    return S_OK;
}

// IObjectWithSite
IFACEMETHODIMP CQuickFolderCommand::SetSite(IUnknown* pUnkSite) {
    if (m_pSite) {
        m_pSite->Release();
        m_pSite = nullptr;
    }

    m_pSite = pUnkSite;
    if (m_pSite) {
        m_pSite->AddRef();
        IUnknown_GetWindow(m_pSite, &m_hwndOwner);
    }
    return S_OK;
}

IFACEMETHODIMP CQuickFolderCommand::GetSite(REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;
    if (m_pSite) {
        return m_pSite->QueryInterface(riid, ppv);
    }
    return E_FAIL;
}

// ============================================================================
// CQuickFolderClassFactory
// ============================================================================

CQuickFolderClassFactory::CQuickFolderClassFactory() {
}

CQuickFolderClassFactory::~CQuickFolderClassFactory() {
}

IFACEMETHODIMP CQuickFolderClassFactory::QueryInterface(REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (riid == IID_IUnknown || riid == IID_IClassFactory) {
        *ppv = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

IFACEMETHODIMP_(ULONG) CQuickFolderClassFactory::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

IFACEMETHODIMP_(ULONG) CQuickFolderClassFactory::Release() {
    ULONG rc = InterlockedDecrement(&m_refCount);
    if (rc == 0) {
        delete this;
    }
    return rc;
}

IFACEMETHODIMP CQuickFolderClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) {
    if (pUnkOuter) return CLASS_E_NOAGGREGATION;
    if (!ppvObject) return E_POINTER;
    *ppvObject = nullptr;

    auto* cmd = new (std::nothrow) CQuickFolderCommand();
    if (!cmd) return E_OUTOFMEMORY;

    HRESULT hr = cmd->QueryInterface(riid, ppvObject);
    cmd->Release();
    return hr;
}

IFACEMETHODIMP CQuickFolderClassFactory::LockServer(BOOL) {
    return S_OK;
}

// ============================================================================
// Registration Functions
// ============================================================================

HRESULT RegisterServer(bool perUser) {
    HKEY hRoot = perUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    std::wstring exePath = GetExePath();
    std::wstring quotedExe = L"\"" + exePath + L"\"";
    std::wstring localServerCmd = quotedExe + L" -Embedding";
    std::wstring iconValue = quotedExe + L",0";
    std::wstring verbText = UI::GetString(IDS_MOVE_TO_NEW_FOLDER);

    // 1. Register COM CLSID
    std::wstring clsidKey = L"Software\\Classes\\CLSID\\" + std::wstring(CLSID_QuickFolderCommand_String);
    SetRegString(hRoot, clsidKey, L"", L"QuickFolder Command Executor");
    SetRegString(hRoot, clsidKey + L"\\LocalServer32", L"", localServerCmd);

    // Helper to register context menu verb for a given shell class
    auto RegisterVerb = [&](const std::wstring& classPath) {
        std::wstring verbKey = classPath + L"\\shell\\QuickFolder";
        SetRegString(hRoot, verbKey, L"", verbText);
        SetRegString(hRoot, verbKey, L"MUIVerb", verbText);
        SetRegString(hRoot, verbKey, L"Icon", iconValue);
        SetRegString(hRoot, verbKey, L"MultiSelectModel", L"Document");
        SetRegString(hRoot, verbKey + L"\\command", L"DelegateExecute", CLSID_QuickFolderCommand_String);
    };

    RegisterVerb(L"Software\\Classes\\AllFilesystemObjects");
    RegisterVerb(L"Software\\Classes\\Directory");

    // Notify Explorer of association change
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    return S_OK;
}

HRESULT UnregisterServer(bool perUser) {
    HKEY hRoot = perUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    // Delete Context Menu verbs
    RegDeleteTreeW(hRoot, L"Software\\Classes\\AllFilesystemObjects\\shell\\QuickFolder");
    RegDeleteTreeW(hRoot, L"Software\\Classes\\Directory\\shell\\QuickFolder");

    // Delete COM CLSID
    std::wstring clsidKey = L"Software\\Classes\\CLSID\\" + std::wstring(CLSID_QuickFolderCommand_String);
    RegDeleteTreeW(hRoot, clsidKey.c_str());

    // Notify Explorer
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    return S_OK;
}

} // namespace QuickFolder::Shell
