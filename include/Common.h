#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <propsys.h>
#include <uxtheme.h>
#include <dwmapi.h>

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <filesystem>
#include <format>
#include <optional>

// Application Constants
#define QUICKFOLDER_APP_NAME L"QuickFolder"
#define QUICKFOLDER_VERSION L"0.2.0"

// CLSID: {4B2F7E41-8A19-4C1B-94E5-46D9B6E1C87D}
// {4B2F7E41-8A19-4C1B-94E5-46D9B6E1C87D}
inline constexpr GUID CLSID_QuickFolderCommand = {
    0x4b2f7e41, 0x8a19, 0x4c1b, {0x94, 0xe5, 0x46, 0xd9, 0xb6, 0xe1, 0xc8, 0x7d}
};

inline constexpr wchar_t CLSID_QuickFolderCommand_String[] = L"{4B2F7E41-8A19-4C1B-94E5-46D9B6E1C87D}";
