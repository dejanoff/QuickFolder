#include "SelectionHelper.h"
#include "PathUtils.h"

namespace QuickFolder::Selection {

namespace {

bool IsShortcutExtension(const std::wstring& path) {
    if (path.length() >= 4) {
        std::wstring ext = path.substr(path.length() - 4);
        if (_wcsicmp(ext.c_str(), L".lnk") == 0 || _wcsicmp(ext.c_str(), L".url") == 0) {
            return true;
        }
    }
    return false;
}

} // namespace

SelectionAnalysis AnalyzeShellItems(IShellItemArray* pItemArray) {
    SelectionAnalysis analysis;
    if (!pItemArray) {
        return analysis;
    }

    DWORD count = 0;
    HRESULT hr = pItemArray->GetCount(&count);
    if (FAILED(hr) || count == 0) {
        return analysis;
    }

    analysis.items.reserve(count);

    for (DWORD i = 0; i < count; ++i) {
        IShellItem* psi = nullptr;
        hr = pItemArray->GetItemAt(i, &psi);
        if (FAILED(hr) || !psi) {
            continue;
        }

        // Verify that this is a real filesystem item, not a virtual shell object
        SFGAOF attributes = 0;
        hr = psi->GetAttributes(SFGAO_FILESYSTEM | SFGAO_FOLDER, &attributes);
        if (FAILED(hr) || !(attributes & SFGAO_FILESYSTEM)) {
            analysis.hasVirtualObjects = true;
            psi->Release();
            continue;
        }

        PWSTR pszPath = nullptr;
        hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
        if (FAILED(hr) || !pszPath) {
            analysis.hasNonFilesystemItems = true;
            psi->Release();
            continue;
        }

        std::wstring itemPath = pszPath;
        CoTaskMemFree(pszPath);

        SelectedItem item;
        item.path = itemPath;
        item.isDirectory = (attributes & SFGAO_FOLDER) != 0;
        item.isShortcut = IsShortcutExtension(itemPath);

        // Determine parent directory of this item
        std::wstring itemParent;
        IShellItem* psiParent = nullptr;
        if (SUCCEEDED(psi->GetParent(&psiParent)) && psiParent) {
            PWSTR pszParent = nullptr;
            if (SUCCEEDED(psiParent->GetDisplayName(SIGDN_FILESYSPATH, &pszParent)) && pszParent) {
                itemParent = pszParent;
                CoTaskMemFree(pszParent);
            }
            psiParent->Release();
        }

        if (itemParent.empty()) {
            itemParent = PathUtils::GetParentDirectory(itemPath);
        }

        if (analysis.commonParent.empty()) {
            analysis.commonParent = itemParent;
        } else if (!PathUtils::ArePathsEqual(analysis.commonParent, itemParent)) {
            analysis.hasDifferentParents = true;
        }

        analysis.items.push_back(item);
        psi->Release();
    }

    return analysis;
}

SelectionAnalysis AnalyzePaths(const std::vector<std::wstring>& paths) {
    SelectionAnalysis analysis;
    if (paths.empty()) {
        return analysis;
    }

    analysis.items.reserve(paths.size());

    for (const auto& rawPath : paths) {
        std::wstring norm = PathUtils::NormalizePath(rawPath);
        if (norm.empty()) continue;

        DWORD attrib = GetFileAttributesW(PathUtils::EnsureLongPathPrefix(norm).c_str());
        if (attrib == INVALID_FILE_ATTRIBUTES) {
            // File does not exist on filesystem
            analysis.hasNonFilesystemItems = true;
            continue;
        }

        SelectedItem item;
        item.path = norm;
        item.isDirectory = (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
        item.isShortcut = IsShortcutExtension(norm);

        std::wstring parentDir = PathUtils::GetParentDirectory(norm);
        if (analysis.commonParent.empty()) {
            analysis.commonParent = parentDir;
        } else if (!PathUtils::ArePathsEqual(analysis.commonParent, parentDir)) {
            analysis.hasDifferentParents = true;
        }

        analysis.items.push_back(item);
    }

    return analysis;
}

} // namespace QuickFolder::Selection
