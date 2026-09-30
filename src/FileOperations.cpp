#include "FileOperations.h"
#include "PathUtils.h"
#include "UI.h"

namespace QuickFolder::FileOps {

MoveResult ExecuteMove(
    HWND hwndOwner,
    const std::vector<std::wstring>& sourcePaths,
    const std::wstring& destinationDirectory,
    bool destFolderAlreadyExisted
) {
    MoveResult result;
    if (sourcePaths.empty()) {
        result.errorMessage = L"No items selected to move.";
        return result;
    }

    // Ensure destination directory exists. If it does not exist, create it.
    std::wstring longDest = PathUtils::EnsureLongPathPrefix(destinationDirectory);
    if (!PathUtils::PathExists(destinationDirectory)) {
        if (!CreateDirectoryW(longDest.c_str(), NULL)) {
            DWORD err = GetLastError();
            result.hr = HRESULT_FROM_WIN32(err);
            result.errorMessage = L"Failed to create target directory.";
            return result;
        }
        result.createdDirectory = destinationDirectory;
    }

    // Check for collisions with existing files in destination directory
    std::vector<std::wstring> conflictingNames;
    for (const auto& srcPath : sourcePaths) {
        std::wstring fileName = PathUtils::GetFileName(srcPath);
        std::wstring destTargetPath = PathUtils::CombinePath(destinationDirectory, fileName);
        if (PathUtils::PathExists(destTargetPath)) {
            conflictingNames.push_back(fileName);
        }
    }

    UI::ConflictResolution resolution = UI::ConflictResolution::AutoRename;
    if (!conflictingNames.empty()) {
        resolution = UI::PromptFileConflict(hwndOwner, conflictingNames.size(), conflictingNames[0]);
        if (resolution == UI::ConflictResolution::Cancel) {
            result.cancelled = true;
            SafeCleanupCreatedDirectory(result.createdDirectory, destFolderAlreadyExisted);
            return result;
        }
    }

    // CoCreate IFileOperation
    IFileOperation* pfo = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
    if (FAILED(hr) || !pfo) {
        result.hr = hr;
        result.errorMessage = L"Failed to initialize Windows File Operation engine.";
        SafeCleanupCreatedDirectory(result.createdDirectory, destFolderAlreadyExisted);
        return result;
    }

    // Configure flags:
    // FOF_ALLOWUNDO: Integrates with Windows Explorer Undo stack (Ctrl+Z)
    // FOF_NOCONFIRMMKDIR: Silent directory creation if needed
    // FOF_NOCONFIRMATION: Overwrite without redundant prompts if user chose Overwrite
    DWORD flags = FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR;
    if (resolution == UI::ConflictResolution::Overwrite) {
        flags |= FOF_NOCONFIRMATION;
    }
    pfo->SetOperationFlags(flags);
    if (hwndOwner) {
        pfo->SetOwnerWindow(hwndOwner);
    }

    // Create IShellItem for destination
    IShellItem* psiDest = nullptr;
    hr = SHCreateItemFromParsingName(destinationDirectory.c_str(), NULL, IID_PPV_ARGS(&psiDest));
    if (FAILED(hr) || !psiDest) {
        result.hr = hr;
        result.errorMessage = L"Failed to bind destination directory in Shell.";
        pfo->Release();
        SafeCleanupCreatedDirectory(result.createdDirectory, destFolderAlreadyExisted);
        return result;
    }

    // Queue move for each source item, auto-renaming colliding items if requested
    size_t queuedCount = 0;
    std::vector<std::wstring> batchClaimedNames;

    for (const auto& srcPath : sourcePaths) {
        std::wstring fileName = PathUtils::GetFileName(srcPath);
        std::wstring finalName = fileName;

        if (resolution == UI::ConflictResolution::AutoRename) {
            std::wstring destTargetPath = PathUtils::CombinePath(destinationDirectory, fileName);
            if (PathUtils::PathExists(destTargetPath)) {
                finalName = PathUtils::GenerateUniqueName(destinationDirectory, srcPath, batchClaimedNames);
                batchClaimedNames.push_back(finalName);
            }
        }

        IShellItem* psiSource = nullptr;
        HRESULT hrItem = SHCreateItemFromParsingName(srcPath.c_str(), NULL, IID_PPV_ARGS(&psiSource));
        if (SUCCEEDED(hrItem) && psiSource) {
            LPCWSTR pNewName = (finalName != fileName) ? finalName.c_str() : NULL;
            hrItem = pfo->MoveItem(psiSource, psiDest, pNewName, NULL);
            if (SUCCEEDED(hrItem)) {
                queuedCount++;
            }
            psiSource->Release();
        }
    }

    if (queuedCount == 0) {
        result.errorMessage = L"No valid items could be queued for move.";
        psiDest->Release();
        pfo->Release();
        SafeCleanupCreatedDirectory(result.createdDirectory, destFolderAlreadyExisted);
        return result;
    }

    // Perform the operations!
    hr = pfo->PerformOperations();
    result.hr = hr;

    BOOL anyOperationsAborted = FALSE;
    pfo->GetAnyOperationsAborted(&anyOperationsAborted);
    result.cancelled = (anyOperationsAborted != FALSE);

    psiDest->Release();
    pfo->Release();

    // Check result
    if (SUCCEEDED(hr) && !result.cancelled) {
        result.success = true;
        result.itemsMoved = queuedCount;
    } else {
        if (result.cancelled) {
            result.errorMessage = L"Operation was cancelled by user.";
        } else {
            result.errorMessage = L"File operation failed.";
        }
        // If created by us and no items were moved (destination folder is still empty), clean it up
        SafeCleanupCreatedDirectory(result.createdDirectory, destFolderAlreadyExisted);
    }

    return result;
}

void SafeCleanupCreatedDirectory(const std::wstring& directoryPath, bool destFolderAlreadyExisted) {
    if (destFolderAlreadyExisted || directoryPath.empty()) {
        return; // Never delete a pre-existing folder!
    }

    if (PathUtils::PathExists(directoryPath) && PathUtils::IsDirectoryEmpty(directoryPath)) {
        std::wstring longPath = PathUtils::EnsureLongPathPrefix(directoryPath);
        RemoveDirectoryW(longPath.c_str());
    }
}

} // namespace QuickFolder::FileOps
