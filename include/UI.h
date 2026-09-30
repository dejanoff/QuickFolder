#pragma once

#include "Common.h"
#include "FileOperations.h"

namespace QuickFolder::UI {

struct DialogParams {
    HWND hwndOwner = NULL;
    size_t itemCount = 0;
    std::wstring suggestedName;
    std::wstring commonParent;
    std::wstring chosenFolderName;
    bool confirmed = false;
};

// Initializes Common Controls and DPI awareness
void InitializeUI();

// Shows the main native QuickFolder dialog
bool ShowQuickFolderDialog(DialogParams& params);

// Prompts the user when the target folder already exists
FileOps::ExistingDestAction PromptExistingFolder(HWND hwndOwner, const std::wstring& folderName);

// Shows a localized error message dialog
void ShowErrorMessage(HWND hwndOwner, const std::wstring& message);

// Shows a localized info message dialog
void ShowInfoMessage(HWND hwndOwner, const std::wstring& message);

// Retrieves a localized string based on the user's UI language
std::wstring GetString(UINT stringId);

// Centers a window relative to an owner window, or the cursor's monitor
void CenterWindow(HWND hwnd, HWND hwndOwner);

// Applies dark mode to a window if Windows Dark Mode is enabled
void ApplyTheme(HWND hwnd);

} // namespace QuickFolder::UI
