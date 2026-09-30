#pragma once

#include "Common.h"

namespace QuickFolder::PathUtils {

enum class ValidationResult {
    Valid,
    Empty,
    InvalidCharacters,
    ReservedName,
    TrailingDotOrSpace,
    DotOrDotDot,
    TooLong
};

// Validates a single folder name component (not an absolute path!)
ValidationResult ValidateFolderName(const std::wstring& name, std::wstring* outErrorMsg = nullptr);

// Formats a user-friendly error message for a validation result
std::wstring GetValidationErrorMessage(ValidationResult result);

// Generates the default suggested folder name for a single selected item
std::wstring SuggestFolderNameForSingleItem(const std::wstring& itemPath, bool isDirectory);

// Strips invalid characters if needed for suggestions
std::wstring SanitizeSuggestedName(const std::wstring& rawName);

// Normalizes path separators and trims trailing slashes
std::wstring NormalizePath(const std::wstring& path);

// Extracts the parent directory of an absolute path
std::wstring GetParentDirectory(const std::wstring& path);

// Extracts the filename/leaf component
std::wstring GetFileName(const std::wstring& path);

// Strips file extension
std::wstring StripExtension(const std::wstring& filename);

// Extracts file extension including dot (e.g. ".txt", or "" if none)
std::wstring GetExtension(const std::wstring& filename);

// Generates unique auto-renamed filename if collision exists (e.g. "name (2).ext", "name (3).ext")
std::wstring GenerateUniqueName(
    const std::wstring& destDir,
    const std::wstring& srcPath,
    const std::vector<std::wstring>& alreadyClaimedInBatch = {}
);

// Combines a directory path with a folder/file name
std::wstring CombinePath(const std::wstring& dir, const std::wstring& subName);

// Checks if path1 and path2 point to the same filesystem location (case-insensitive)
bool ArePathsEqual(const std::wstring& path1, const std::wstring& path2);

// Checks if childPath is located inside parentPath (prevents recursive moves)
bool IsSubdirectoryOf(const std::wstring& childPath, const std::wstring& parentPath);

// Adds extended-length prefix (\\?\) for long paths if needed
std::wstring EnsureLongPathPrefix(const std::wstring& path);

// Checks if path exists on the filesystem
bool PathExists(const std::wstring& path);

// Checks if path is an existing directory
bool IsDirectory(const std::wstring& path);

// Checks if directory is empty
bool IsDirectoryEmpty(const std::wstring& path);

} // namespace QuickFolder::PathUtils
