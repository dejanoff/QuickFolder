#include "PathUtils.h"
#include "Resource.h"
#include <cwctype>

namespace QuickFolder::PathUtils {

namespace {

bool IsReservedName(const std::wstring& name) {
    // Find the base name before the first dot (e.g. "CON.txt" -> "CON")
    std::wstring base = name;
    size_t firstDot = base.find(L'.');
    if (firstDot != std::wstring::npos) {
        base = base.substr(0, firstDot);
    }

    // Convert to uppercase for case-insensitive comparison
    std::wstring upper;
    upper.reserve(base.length());
    for (wchar_t c : base) {
        upper.push_back(static_cast<wchar_t>(std::towupper(c)));
    }

    // Standard DOS reserved names
    static const wchar_t* reserved[] = {
        L"CON", L"PRN", L"AUX", L"NUL",
        L"COM1", L"COM2", L"COM3", L"COM4", L"COM5",
        L"COM6", L"COM7", L"COM8", L"COM9",
        L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5",
        L"LPT6", L"LPT7", L"LPT8", L"LPT9"
    };

    for (const wchar_t* r : reserved) {
        if (upper == r) {
            return true;
        }
    }

    // Windows Unicode superscript variants: COM¹ COM² COM³ LPT¹ LPT² LPT³
    static const wchar_t* unicodeReserved[] = {
        L"COM\u00B9", L"COM\u00B2", L"COM\u00B3",
        L"LPT\u00B9", L"LPT\u00B2", L"LPT\u00B3"
    };
    for (const wchar_t* r : unicodeReserved) {
        if (upper == r) {
            return true;
        }
    }

    return false;
}

} // namespace

ValidationResult ValidateFolderName(const std::wstring& name, std::wstring* outErrorMsg) {
    if (name.empty()) {
        if (outErrorMsg) *outErrorMsg = L"Folder name cannot be empty.";
        return ValidationResult::Empty;
    }

    // Check if entire string is only whitespace
    bool hasNonWhitespace = false;
    for (wchar_t c : name) {
        if (!std::iswspace(c)) {
            hasNonWhitespace = true;
            break;
        }
    }
    if (!hasNonWhitespace) {
        if (outErrorMsg) *outErrorMsg = L"Folder name cannot consist only of whitespace.";
        return ValidationResult::Empty;
    }

    // Check length (NTFS and Win32 limit for a single component is 255 UTF-16 code units)
    if (name.length() > 255) {
        if (outErrorMsg) *outErrorMsg = L"Folder name exceeds maximum allowed component length (255 characters).";
        return ValidationResult::TooLong;
    }

    // Relative navigation names are strictly forbidden
    if (name == L"." || name == L"..") {
        if (outErrorMsg) *outErrorMsg = L"Folder name cannot be '.' or '..'.";
        return ValidationResult::DotOrDotDot;
    }

    // Check for trailing dot or space (Windows filesystem restrictions)
    wchar_t lastChar = name.back();
    if (lastChar == L'.' || lastChar == L' ') {
        if (outErrorMsg) *outErrorMsg = L"Folder name cannot end with a dot or a space.";
        return ValidationResult::TrailingDotOrSpace;
    }

    // Check for invalid filesystem characters
    // Forbidden: \ / : * ? " < > | and control characters (0-31)
    static const wchar_t invalidChars[] = L"\\/:*?\"<>|";
    for (wchar_t c : name) {
        if (c < 32) {
            if (outErrorMsg) *outErrorMsg = L"Folder name contains unprintable control characters.";
            return ValidationResult::InvalidCharacters;
        }
        for (const wchar_t* p = invalidChars; *p != L'\0'; ++p) {
            if (c == *p) {
                if (outErrorMsg) {
                    *outErrorMsg = std::wstring(L"Folder name contains forbidden character: '") + c + L"'.";
                }
                return ValidationResult::InvalidCharacters;
            }
        }
    }

    // Check for reserved DOS device names
    if (IsReservedName(name)) {
        if (outErrorMsg) *outErrorMsg = L"The specified name is a reserved Windows device name (such as CON, PRN, AUX, NUL, COM1-9, LPT1-9).";
        return ValidationResult::ReservedName;
    }

    if (outErrorMsg) outErrorMsg->clear();
    return ValidationResult::Valid;
}

std::wstring GetValidationErrorMessage(ValidationResult result) {
    switch (result) {
    case ValidationResult::Valid:
        return L"";
    case ValidationResult::Empty:
        return L"Please enter a folder name.";
    case ValidationResult::InvalidCharacters:
        return L"A folder name cannot contain any of the following characters: \\ / : * ? \" < > |";
    case ValidationResult::ReservedName:
        return L"The folder name is a reserved Windows system name (e.g., CON, PRN, AUX, NUL, COM1-9, LPT1-9).";
    case ValidationResult::TrailingDotOrSpace:
        return L"The folder name cannot end with a dot (.) or a space.";
    case ValidationResult::DotOrDotDot:
        return L"The folder name cannot be \".\" or \"..\".";
    case ValidationResult::TooLong:
        return L"The folder name is too long (maximum 255 characters).";
    default:
        return L"The folder name is invalid.";
    }
}

std::wstring SanitizeSuggestedName(const std::wstring& rawName) {
    std::wstring result;
    result.reserve(rawName.length());
    static const wchar_t invalidChars[] = L"\\/:*?\"<>|";

    for (wchar_t c : rawName) {
        if (c < 32) continue;
        bool isInvalid = false;
        for (const wchar_t* p = invalidChars; *p != L'\0'; ++p) {
            if (c == *p) {
                isInvalid = true;
                break;
            }
        }
        if (!isInvalid) {
            result.push_back(c);
        }
    }

    // Trim trailing dots or spaces
    while (!result.empty() && (result.back() == L'.' || result.back() == L' ')) {
        result.pop_back();
    }

    return result;
}

std::wstring SuggestFolderNameForSingleItem(const std::wstring& itemPath, bool isDirectory) {
    std::wstring leaf = GetFileName(itemPath);

    if (isDirectory) {
        // For a directory, proposing the same name creates an immediate collision with itself.
        // Propose "<DirectoryName> - Folder" without awkward punctuation like tildes.
        std::wstring suggested = leaf + L" - Folder";
        return SanitizeSuggestedName(suggested);
    } else {
        // For a file, propose the filename without its extension
        std::wstring baseName = StripExtension(leaf);
        if (baseName.empty()) {
            baseName = leaf;
        }
        return SanitizeSuggestedName(baseName);
    }
}

std::wstring NormalizePath(const std::wstring& path) {
    if (path.empty()) return L"";

    std::wstring result = path;
    // Replace all '/' with '\'
    for (wchar_t& c : result) {
        if (c == L'/') c = L'\\';
    }

    // Remove redundant trailing slashes, but keep root like "C:\" or "\\"
    while (result.length() > 3 && result.back() == L'\\') {
        result.pop_back();
    }

    return result;
}

std::wstring GetParentDirectory(const std::wstring& path) {
    std::wstring norm = NormalizePath(path);
    size_t lastSlash = norm.rfind(L'\\');
    if (lastSlash == std::wstring::npos) {
        return L"";
    }

    // If "C:\", preserve the root slash
    if (lastSlash == 2 && norm[1] == L':') {
        return norm.substr(0, 3);
    }
    // If "\\server\share" root, preserve
    if (lastSlash == 0) {
        return norm.substr(0, 1);
    }

    return norm.substr(0, lastSlash);
}

std::wstring GetFileName(const std::wstring& path) {
    std::wstring norm = NormalizePath(path);
    size_t lastSlash = norm.rfind(L'\\');
    if (lastSlash == std::wstring::npos) {
        return norm;
    }
    return norm.substr(lastSlash + 1);
}

std::wstring StripExtension(const std::wstring& filename) {
    size_t lastDot = filename.rfind(L'.');
    if (lastDot == std::wstring::npos || lastDot == 0) {
        return filename;
    }
    return filename.substr(0, lastDot);
}

std::wstring GetExtension(const std::wstring& filename) {
    size_t lastDot = filename.rfind(L'.');
    if (lastDot == std::wstring::npos || lastDot == 0) {
        return L"";
    }
    return filename.substr(lastDot);
}

std::wstring GenerateUniqueName(
    const std::wstring& destDir,
    const std::wstring& srcPath,
    const std::vector<std::wstring>& alreadyClaimedInBatch
) {
    std::wstring fileName = GetFileName(srcPath);
    bool isDir = IsDirectory(srcPath);

    std::wstring stem;
    std::wstring ext;
    if (isDir) {
        stem = fileName;
        ext = L"";
    } else {
        stem = StripExtension(fileName);
        ext = GetExtension(fileName);
    }

    auto IsClaimed = [&](const std::wstring& name) {
        for (const auto& claimed : alreadyClaimedInBatch) {
            if (_wcsicmp(claimed.c_str(), name.c_str()) == 0) {
                return true;
            }
        }
        return false;
    };

    // If destination does not have this file and not claimed in batch, original is unique
    std::wstring originalTarget = CombinePath(destDir, fileName);
    if (!PathExists(originalTarget) && !IsClaimed(fileName)) {
        return fileName;
    }

    // Otherwise, generate candidate "stem (2).ext", "stem (3).ext", etc.
    int counter = 2;
    while (true) {
        std::wstring candidate = stem + L" (" + std::to_wstring(counter) + L")" + ext;
        std::wstring candidatePath = CombinePath(destDir, candidate);
        if (!PathExists(candidatePath) && !IsClaimed(candidate)) {
            return candidate;
        }
        counter++;
    }
}

std::wstring CombinePath(const std::wstring& dir, const std::wstring& subName) {
    std::wstring normDir = NormalizePath(dir);
    if (normDir.empty()) return subName;
    if (normDir.back() == L'\\') {
        return normDir + subName;
    }
    return normDir + L'\\' + subName;
}

bool ArePathsEqual(const std::wstring& path1, const std::wstring& path2) {
    std::wstring n1 = NormalizePath(path1);
    std::wstring n2 = NormalizePath(path2);
    return _wcsicmp(n1.c_str(), n2.c_str()) == 0;
}

bool IsSubdirectoryOf(const std::wstring& childPath, const std::wstring& parentPath) {
    std::wstring child = NormalizePath(childPath);
    std::wstring parent = NormalizePath(parentPath);

    if (ArePathsEqual(child, parent)) {
        return true;
    }

    if (parent.back() != L'\\') {
        parent += L'\\';
    }

    if (child.length() < parent.length()) {
        return false;
    }

    return _wcsnicmp(child.c_str(), parent.c_str(), parent.length()) == 0;
}

std::wstring EnsureLongPathPrefix(const std::wstring& path) {
    if (path.empty()) return L"";

    // If already has prefix \\?\ (4 chars: \ \ ? \)
    if (path.length() >= 4 && path[0] == L'\\' && path[1] == L'\\' && path[2] == L'?' && path[3] == L'\\') {
        return path;
    }
    // Check for NT device namespace prefix
    if (path.length() >= 4 && path[0] == L'\\' && path[1] == L'?' && path[2] == L'?' && path[3] == L'\\') {
        return path;
    }

    // UNC path: \\server\share -> \\?\UNC\server\share
    if (path.length() >= 2 && path[0] == L'\\' && path[1] == L'\\') {
        return std::wstring(L"\\\\?\\UNC\\") + path.substr(2);
    }

    // Drive letter: C:\path -> \\?\C:\path
    if (path.length() >= 2 && std::iswalpha(path[0]) && path[1] == L':') {
        return std::wstring(L"\\\\?\\") + path;
    }

    return path;
}

bool PathExists(const std::wstring& path) {
    std::wstring longPath = EnsureLongPathPrefix(path);
    DWORD attrib = GetFileAttributesW(longPath.c_str());
    return (attrib != INVALID_FILE_ATTRIBUTES);
}

bool IsDirectory(const std::wstring& path) {
    std::wstring longPath = EnsureLongPathPrefix(path);
    DWORD attrib = GetFileAttributesW(longPath.c_str());
    if (attrib == INVALID_FILE_ATTRIBUTES) return false;
    return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool IsDirectoryEmpty(const std::wstring& path) {
    std::wstring searchPattern = CombinePath(path, L"*");
    std::wstring longSearch = EnsureLongPathPrefix(searchPattern);

    WIN32_FIND_DATAW fd = {};
    HANDLE hFind = FindFirstFileW(longSearch.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return true; // Cannot read or empty
    }

    bool hasItems = false;
    do {
        if (wcscmp(fd.cFileName, L".") != 0 && wcscmp(fd.cFileName, L"..") != 0) {
            hasItems = true;
            break;
        }
    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
    return !hasItems;
}

} // namespace QuickFolder::PathUtils
