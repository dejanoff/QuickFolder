#pragma once

#include "Common.h"
#include <vector>

namespace QuickFolder::FileOps {

enum class ExistingDestAction {
    UseExisting,
    ChooseAnother,
    Cancel
};

struct MoveResult {
    bool success = false;
    bool cancelled = false;
    bool abortedBeforeMove = false;
    HRESULT hr = S_OK;
    std::wstring createdDirectory;
    size_t itemsMoved = 0;
    std::wstring errorMessage;
};

// Executes moving the selected source paths into destinationDirectory using IFileOperation
MoveResult ExecuteMove(
    HWND hwndOwner,
    const std::vector<std::wstring>& sourcePaths,
    const std::wstring& destinationDirectory,
    bool destFolderAlreadyExisted
);

// Cleans up newly created directory if the move operation failed or was cancelled and folder remains empty
void SafeCleanupCreatedDirectory(const std::wstring& directoryPath, bool destFolderAlreadyExisted);

} // namespace QuickFolder::FileOps
