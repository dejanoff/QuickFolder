#pragma once

#include "Common.h"
#include <vector>

namespace QuickFolder::Selection {

struct SelectedItem {
    std::wstring path;
    bool isDirectory = false;
    bool isShortcut = false; // .lnk or .url
};

struct SelectionAnalysis {
    std::vector<SelectedItem> items;
    std::wstring commonParent;
    bool hasDifferentParents = false;
    bool hasVirtualObjects = false;
    bool hasNonFilesystemItems = false;

    bool IsValid() const {
        return !items.empty() && !hasDifferentParents && !hasVirtualObjects && !commonParent.empty();
    }
};

// Analyzes an IShellItemArray provided by Windows Explorer
SelectionAnalysis AnalyzeShellItems(IShellItemArray* pItemArray);

// Analyzes a list of file system paths (used for CLI, tests, dev mode)
SelectionAnalysis AnalyzePaths(const std::vector<std::wstring>& paths);

} // namespace QuickFolder::Selection
