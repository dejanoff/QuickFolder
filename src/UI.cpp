#include "UI.h"
#include "Resource.h"
#include "PathUtils.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <windowsx.h>
#include <cwchar>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace QuickFolder::UI {

namespace {

HFONT g_hFont = NULL;
bool g_isDarkMode = false;
HBRUSH g_hDarkBgBrush = NULL;
HBRUSH g_hDarkEditBrush = NULL;

bool DetectWindowsDarkMode() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD val = 1;
        DWORD size = sizeof(val);
        if (RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, (LPBYTE)&val, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return (val == 0);
        }
        RegCloseKey(hKey);
    }
    return false;
}

LANGID GetCurrentLangId() {
    return PRIMARYLANGID(GetUserDefaultUILanguage());
}

} // namespace

std::wstring GetString(UINT stringId) {
    LANGID lang = GetCurrentLangId();

    if (lang == LANG_RUSSIAN || lang == LANG_UKRAINIAN) {
        switch (stringId) {
        case IDS_APP_TITLE: return L"QuickFolder";
        case IDS_MOVE_TO_NEW_FOLDER: return L"Переместить в новую папку...";
        case IDS_FOLDER_NAME_LABEL: return L"Имя папки:";
        case IDS_BTN_MOVE_TEXT: return L"Переместить";
        case IDS_BTN_CANCEL_TEXT: return L"Отмена";
        case IDS_ITEMS_SELECTED_1: return L"Выбран 1 элемент";
        case IDS_ITEMS_SELECTED_N: return L"Выбрано элементов: %zu";
        case IDS_ERR_DIFFERENT_PARENTS: return L"Выбранные элементы находятся в разных папках.";
        case IDS_ERR_INVALID_NAME: return L"Имя папки содержит недопустимые символы или зарезервировано системой.";
        case IDS_ERR_DEST_INSIDE_SELECTION: return L"Целевая папка не может находиться внутри выбранных элементов.";
        case IDS_ERR_DEST_EXISTS: return L"Папка «%s» уже существует.";
        case IDS_ERR_MOVE_FAILED: return L"Не удалось переместить выбранные элементы.";
        case IDS_OPT_USE_EXISTING: return L"Использовать существующую папку";
        case IDS_OPT_CHOOSE_ANOTHER: return L"Выбрать другое имя";
        case IDS_FOLDER_SUFFIX: return L" - Папка";
        case IDS_RECENT_FOLDERS_LABEL: return L"Недавние папки:";
        case IDS_CLEAR_RECENT_TOOLTIP: return L"Очистить недавние";
        case IDS_NO_RECENT_FOLDERS: return L"(нет недавних)";
        case IDS_CONFLICT_TITLE: return L"Конфликт имён файлов";
        case IDS_CONFLICT_PROMPT: return L"В целевой папке уже есть файлы с такими именами.";
        case IDS_CONFLICT_RENAME_OPT: return L"Автоматически переименовать";
        case IDS_CONFLICT_OVERWRITE_OPT: return L"Перезаписать";
        default: break;
        }
    } else if (lang == LANG_GERMAN) {
        switch (stringId) {
        case IDS_APP_TITLE: return L"QuickFolder";
        case IDS_MOVE_TO_NEW_FOLDER: return L"In neuen Ordner verschieben...";
        case IDS_FOLDER_NAME_LABEL: return L"Ordnername:";
        case IDS_BTN_MOVE_TEXT: return L"Verschieben";
        case IDS_BTN_CANCEL_TEXT: return L"Abbrechen";
        case IDS_ITEMS_SELECTED_1: return L"1 Element ausgewählt";
        case IDS_ITEMS_SELECTED_N: return L"%zu Elemente ausgewählt";
        case IDS_ERR_DIFFERENT_PARENTS: return L"Die ausgewählten Elemente befinden sich in unterschiedlichen Ordnern.";
        case IDS_ERR_INVALID_NAME: return L"Der angegebene Ordnername enthält unzulässige Zeichen oder ist reserviert.";
        case IDS_ERR_DEST_INSIDE_SELECTION: return L"Der Zielordner darf sich nicht innerhalb der Auswahl befinden.";
        case IDS_ERR_DEST_EXISTS: return L"Der Ordner \"%s\" ist bereits vorhanden.";
        case IDS_ERR_MOVE_FAILED: return L"Beim Verschieben der Elemente ist ein Fehler aufgetreten.";
        case IDS_OPT_USE_EXISTING: return L"Vorhandenen Ordner verwenden";
        case IDS_OPT_CHOOSE_ANOTHER: return L"Anderen Namen wählen";
        case IDS_FOLDER_SUFFIX: return L" - Ordner";
        case IDS_RECENT_FOLDERS_LABEL: return L"Zuletzt verwendet:";
        case IDS_CLEAR_RECENT_TOOLTIP: return L"Verlauf leeren";
        case IDS_NO_RECENT_FOLDERS: return L"(keine)";
        case IDS_CONFLICT_TITLE: return L"Dateikonflikt erkannt";
        case IDS_CONFLICT_PROMPT: return L"Im Zielordner sind bereits gleichnamige Dateien vorhanden.";
        case IDS_CONFLICT_RENAME_OPT: return L"Automatisch umbenennen";
        case IDS_CONFLICT_OVERWRITE_OPT: return L"Überschreiben";
        default: break;
        }
    }

    // Default: English
    switch (stringId) {
    case IDS_APP_TITLE: return L"QuickFolder";
    case IDS_MOVE_TO_NEW_FOLDER: return L"Move to new folder...";
    case IDS_FOLDER_NAME_LABEL: return L"Folder name:";
    case IDS_BTN_MOVE_TEXT: return L"Move";
    case IDS_BTN_CANCEL_TEXT: return L"Cancel";
    case IDS_ITEMS_SELECTED_1: return L"1 item selected";
    case IDS_ITEMS_SELECTED_N: return L"%zu items selected";
    case IDS_ERR_DIFFERENT_PARENTS: return L"Selected items are located in different folders.";
    case IDS_ERR_INVALID_NAME: return L"A folder name cannot contain any of the following characters: \\ / : * ? \" < > |";
    case IDS_ERR_DEST_INSIDE_SELECTION: return L"The destination folder cannot be inside the selected items.";
    case IDS_ERR_DEST_EXISTS: return L"The folder \"%s\" already exists.";
    case IDS_ERR_MOVE_FAILED: return L"An error occurred while moving items.";
    case IDS_OPT_USE_EXISTING: return L"Use existing folder";
    case IDS_OPT_CHOOSE_ANOTHER: return L"Choose another name";
    case IDS_FOLDER_SUFFIX: return L" - Folder";
    case IDS_RECENT_FOLDERS_LABEL: return L"Recent folders:";
    case IDS_CLEAR_RECENT_TOOLTIP: return L"Clear recent history";
    case IDS_NO_RECENT_FOLDERS: return L"(none)";
    case IDS_CONFLICT_TITLE: return L"File conflict detected";
    case IDS_CONFLICT_PROMPT: return L"The destination folder already contains files with the same names.";
    case IDS_CONFLICT_RENAME_OPT: return L"Auto-rename";
    case IDS_CONFLICT_OVERWRITE_OPT: return L"Overwrite";
    default: return L"";
    }
}

void InitializeUI() {
    INITCOMMONCONTROLSEX icex = {};
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    g_isDarkMode = DetectWindowsDarkMode();
    if (g_isDarkMode) {
        g_hDarkBgBrush = CreateSolidBrush(RGB(32, 32, 32));
        g_hDarkEditBrush = CreateSolidBrush(RGB(45, 45, 45));
    }

    // Create system font (Segoe UI or system default)
    NONCLIENTMETRICSW ncm = {};
    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) {
        g_hFont = CreateFontIndirectW(&ncm.lfMessageFont);
    }
    if (!g_hFont) {
        g_hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }
}

void ApplyTheme(HWND hwnd) {
    if (g_isDarkMode) {
        BOOL dark = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
    }
}

UINT GetWindowDpi(HWND hwnd) {
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef UINT (WINAPI *PFN_GetDpiForWindow)(HWND);
        auto pfn = (PFN_GetDpiForWindow)GetProcAddress(hUser32, "GetDpiForWindow");
        if (pfn && hwnd) {
            UINT dpi = pfn(hwnd);
            if (dpi != 0) return dpi;
        }
    }
    HDC hdc = GetDC(hwnd);
    if (hdc) {
        UINT dpi = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(hwnd, hdc);
        if (dpi != 0) return dpi;
    }
    return 96;
}

UINT GetMonitorDpi(HWND hwndOwner) {
    HMONITOR hMon = NULL;
    POINT pt = {};
    GetCursorPos(&pt);
    hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    if (!hMon && hwndOwner && IsWindow(hwndOwner)) {
        hMon = MonitorFromWindow(hwndOwner, MONITOR_DEFAULTTONEAREST);
    }

    HMODULE hShcore = LoadLibraryW(L"shcore.dll");
    if (hShcore) {
        typedef HRESULT (WINAPI *PFN_GetDpiForMonitor)(HMONITOR, int, UINT*, UINT*);
        auto pfn = (PFN_GetDpiForMonitor)GetProcAddress(hShcore, "GetDpiForMonitor");
        if (pfn && hMon) {
            UINT dpiX = 96, dpiY = 96;
            if (SUCCEEDED(pfn(hMon, 0 /* MDT_EFFECTIVE_DPI */, &dpiX, &dpiY))) {
                FreeLibrary(hShcore);
                return dpiY ? dpiY : 96;
            }
        }
        FreeLibrary(hShcore);
    }
    return 96;
}

void AdjustWindowRectForDpiHelper(LPRECT prc, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi) {
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *PFN_AdjustWindowRectExForDpi)(LPRECT, DWORD, BOOL, DWORD, UINT);
        auto pfn = (PFN_AdjustWindowRectExForDpi)GetProcAddress(hUser32, "AdjustWindowRectExForDpi");
        if (pfn) {
            if (pfn(prc, dwStyle, bMenu, dwExStyle, dpi)) {
                return;
            }
        }
    }
    AdjustWindowRectEx(prc, dwStyle, bMenu, dwExStyle);
}

void CenterWindow(HWND hwnd, HWND hwndOwner) {
    RECT rcDlg = {};
    GetWindowRect(hwnd, &rcDlg);
    int dlgWidth = rcDlg.right - rcDlg.left;
    int dlgHeight = rcDlg.bottom - rcDlg.top;

    POINT ptCursor = {};
    GetCursorPos(&ptCursor);

    HMONITOR hMon = MonitorFromPoint(ptCursor, MONITOR_DEFAULTTONEAREST);
    if (!hMon && hwndOwner && IsWindow(hwndOwner)) {
        hMon = MonitorFromWindow(hwndOwner, MONITOR_DEFAULTTONEAREST);
    }

    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    RECT rcTarget = {};
    if (hMon && GetMonitorInfoW(hMon, &mi)) {
        rcTarget = mi.rcWork;
    } else {
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &rcTarget, 0);
        mi.rcWork = rcTarget;
    }

    int x = rcTarget.left + ((rcTarget.right - rcTarget.left) - dlgWidth) / 2;
    int y = rcTarget.top + ((rcTarget.bottom - rcTarget.top) - dlgHeight) / 2;

    // Ensure dialog stays within work area bounds
    if (x < mi.rcWork.left) x = mi.rcWork.left;
    if (y < mi.rcWork.top) y = mi.rcWork.top;
    if (x + dlgWidth > mi.rcWork.right) x = mi.rcWork.right - dlgWidth;
    if (y + dlgHeight > mi.rcWork.bottom) y = mi.rcWork.bottom - dlgHeight;

    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

std::vector<std::wstring> GetRecentFolders() {
    std::vector<std::wstring> list;
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\QuickFolder\\RecentFolders", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD count = 0;
        DWORD size = sizeof(count);
        if (RegQueryValueExW(hKey, L"Count", nullptr, nullptr, (LPBYTE)&count, &size) == ERROR_SUCCESS) {
            if (count > 6) count = 6;
            for (DWORD i = 0; i < count; ++i) {
                wchar_t valName[32];
                swprintf_s(valName, L"Folder%u", i);
                wchar_t buf[512] = {};
                DWORD bufSize = sizeof(buf);
                if (RegQueryValueExW(hKey, valName, nullptr, nullptr, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS) {
                    if (buf[0] != L'\0') {
                        list.push_back(buf);
                    }
                }
            }
        }
        RegCloseKey(hKey);
    }
    return list;
}

void SaveRecentFolders(const std::vector<std::wstring>& list) {
    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\QuickFolder\\RecentFolders", 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD count = static_cast<DWORD>(list.size() > 6 ? 6 : list.size());
        RegSetValueExW(hKey, L"Count", 0, REG_DWORD, (const BYTE*)&count, sizeof(count));
        for (DWORD i = 0; i < count; ++i) {
            wchar_t valName[32];
            swprintf_s(valName, L"Folder%u", i);
            RegSetValueExW(hKey, valName, 0, REG_SZ,
                (const BYTE*)list[i].c_str(), static_cast<DWORD>((list[i].length() + 1) * sizeof(wchar_t)));
        }
        for (DWORD i = count; i < 6; ++i) {
            wchar_t valName[32];
            swprintf_s(valName, L"Folder%u", i);
            RegDeleteValueW(hKey, valName);
        }
        RegCloseKey(hKey);
    }
}

void AddRecentFolder(const std::wstring& folderName) {
    if (folderName.empty()) return;
    auto list = GetRecentFolders();
    list.erase(std::remove_if(list.begin(), list.end(), [&](const std::wstring& s) {
        return _wcsicmp(s.c_str(), folderName.c_str()) == 0;
    }), list.end());
    list.insert(list.begin(), folderName);
    if (list.size() > 6) {
        list.resize(6);
    }
    SaveRecentFolders(list);
}

void ClearRecentFolders() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\QuickFolder\\RecentFolders", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        for (DWORD i = 0; i < 6; ++i) {
            wchar_t valName[32];
            swprintf_s(valName, L"Folder%u", i);
            RegDeleteValueW(hKey, valName);
        }
        DWORD zero = 0;
        RegSetValueExW(hKey, L"Count", 0, REG_DWORD, (const BYTE*)&zero, sizeof(zero));
        RegCloseKey(hKey);
    }
}

struct DialogState {
    DialogParams* params = nullptr;
    HWND hStaticCount = NULL;
    HWND hStaticRecent = NULL;
    HWND hBtnClearRecent = NULL;
    HWND hBtnRecents[6] = { NULL };
    HWND hStaticPrompt = NULL;
    HWND hEdit = NULL;
    HWND hBtnMove = NULL;
    HWND hBtnCancel = NULL;
    UINT currentDpi = 96;
    HFONT hFont = NULL;
    HFONT hFontBold = NULL;
    HFONT hFontSmall = NULL;
    std::vector<std::wstring> recentFolders;
};

void UpdateDialogFont(DialogState* state, UINT dpi) {
    if (!state) return;
    if (state->hFont) {
        DeleteObject(state->hFont);
        state->hFont = NULL;
    }
    if (state->hFontBold && state->hFontBold != state->hFont) {
        DeleteObject(state->hFontBold);
        state->hFontBold = NULL;
    }
    if (state->hFontSmall) {
        DeleteObject(state->hFontSmall);
        state->hFontSmall = NULL;
    }

    int fontHeight = -MulDiv(9, dpi, 72);
    state->hFont = CreateFontW(
        fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
    if (!state->hFont) {
        state->hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }

    state->hFontBold = CreateFontW(
        fontHeight, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
    if (!state->hFontBold) {
        state->hFontBold = state->hFont;
    }

    int smallFontHeight = -MulDiv(8, dpi, 72);
    state->hFontSmall = CreateFontW(
        smallFontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
    if (!state->hFontSmall) {
        state->hFontSmall = state->hFont;
    }

    if (state->hStaticCount) SendMessageW(state->hStaticCount, WM_SETFONT, (WPARAM)state->hFontBold, TRUE);
    if (state->hStaticRecent) SendMessageW(state->hStaticRecent, WM_SETFONT, (WPARAM)state->hFont, TRUE);
    if (state->hBtnClearRecent) SendMessageW(state->hBtnClearRecent, WM_SETFONT, (WPARAM)state->hFont, TRUE);
    for (int i = 0; i < 6; ++i) {
        if (state->hBtnRecents[i]) {
            SendMessageW(state->hBtnRecents[i], WM_SETFONT, (WPARAM)state->hFontSmall, TRUE);
        }
    }
    if (state->hStaticPrompt) SendMessageW(state->hStaticPrompt, WM_SETFONT, (WPARAM)state->hFont, TRUE);
    if (state->hEdit) SendMessageW(state->hEdit, WM_SETFONT, (WPARAM)state->hFont, TRUE);
    if (state->hBtnMove) SendMessageW(state->hBtnMove, WM_SETFONT, (WPARAM)state->hFont, TRUE);
    if (state->hBtnCancel) SendMessageW(state->hBtnCancel, WM_SETFONT, (WPARAM)state->hFont, TRUE);
}

void LayoutDialogControls(DialogState* state, UINT dpi) {
    if (!state) return;
    auto Scale = [dpi](int val) -> int {
        return MulDiv(val, dpi, 96);
    };

    // Base client width: 440, height: 232 at 96 DPI
    int padX = Scale(20);
    int contentW = Scale(400);

    // Static count: Y=14, H=18
    if (state->hStaticCount) {
        SetWindowPos(state->hStaticCount, NULL, padX, Scale(14), contentW, Scale(18), SWP_NOZORDER | SWP_NOACTIVATE);
    }

    // Recent Folders Header:
    // Label "Zuletzt verwendet:": X=20, Y=36, W=368, H=18
    if (state->hStaticRecent) {
        SetWindowPos(state->hStaticRecent, NULL, padX, Scale(36), Scale(368), Scale(18), SWP_NOZORDER | SWP_NOACTIVATE);
    }

    // Trash button: X=440-20-24=396, Y=34, W=24, H=22
    if (state->hBtnClearRecent) {
        SetWindowPos(state->hBtnClearRecent, NULL, Scale(396), Scale(34), Scale(24), Scale(22), SWP_NOZORDER | SWP_NOACTIVATE);
    }

    // 6 Recent Buttons: 2 rows of 3 buttons
    int btnW = Scale(128);
    int btnH = Scale(24);
    int gapX = Scale(8);

    int colX[3] = {
        padX,
        padX + btnW + gapX,
        padX + (btnW + gapX) * 2
    };

    int rowY[2] = {
        Scale(58),
        Scale(86)
    };

    for (int i = 0; i < 6; ++i) {
        if (state->hBtnRecents[i]) {
            int r = i / 3;
            int c = i % 3;
            SetWindowPos(state->hBtnRecents[i], NULL, colX[c], rowY[r], btnW, btnH, SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }

    // Prompt label ("Ordnername:"): Y=118, H=18
    if (state->hStaticPrompt) {
        SetWindowPos(state->hStaticPrompt, NULL, padX, Scale(118), contentW, Scale(18), SWP_NOZORDER | SWP_NOACTIVATE);
    }

    // Edit control: Y=138, H=28
    if (state->hEdit) {
        SetWindowPos(state->hEdit, NULL, padX, Scale(138), contentW, Scale(28), SWP_NOZORDER | SWP_NOACTIVATE);
    }

    // Buttons: Y=182, H=30, W=100
    int actBtnW = Scale(100);
    int actBtnH = Scale(30);
    int actBtnY = Scale(182);
    int cancelX = Scale(440) - padX - actBtnW;
    int moveX = cancelX - Scale(10) - actBtnW;

    if (state->hBtnMove) {
        SetWindowPos(state->hBtnMove, NULL, moveX, actBtnY, actBtnW, actBtnH, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    if (state->hBtnCancel) {
        SetWindowPos(state->hBtnCancel, NULL, cancelX, actBtnY, actBtnW, actBtnH, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

static LRESULT CALLBACK DialogWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    DialogState* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_NCCREATE: {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        state = reinterpret_cast<DialogState*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    case WM_CREATE: {
        ApplyTheme(hwnd);

        UINT dpi = GetWindowDpi(hwnd);
        state->currentDpi = dpi;

        state->recentFolders = GetRecentFolders();

        // Ensure window is sized properly for client dimensions at this DPI (440 x 232 base)
        int clientW = MulDiv(440, dpi, 96);
        int clientH = MulDiv(232, dpi, 96);
        RECT rc = { 0, 0, clientW, clientH };
        DWORD dwStyle = (DWORD)GetWindowLongPtrW(hwnd, GWL_STYLE);
        DWORD dwExStyle = (DWORD)GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        AdjustWindowRectForDpiHelper(&rc, dwStyle, FALSE, dwExStyle, dpi);

        int winW = rc.right - rc.left;
        int winH = rc.bottom - rc.top;
        SetWindowPos(hwnd, NULL, 0, 0, winW, winH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

        // Item count label text
        std::wstring countStr;
        if (state->params->itemCount == 1) {
            countStr = GetString(IDS_ITEMS_SELECTED_1);
        } else {
            wchar_t buf[128];
            swprintf_s(buf, GetString(IDS_ITEMS_SELECTED_N).c_str(), state->params->itemCount);
            countStr = buf;
        }

        // Create child controls
        state->hStaticCount = CreateWindowExW(
            0, L"STATIC", countStr.c_str(),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0,
            hwnd, (HMENU)IDC_STATIC_COUNT, GetModuleHandleW(NULL), NULL
        );

        // Static Recent label
        std::wstring recentLabel = GetString(IDS_RECENT_FOLDERS_LABEL);
        if (state->recentFolders.empty()) {
            recentLabel += L" " + GetString(IDS_NO_RECENT_FOLDERS);
        }
        state->hStaticRecent = CreateWindowExW(
            0, L"STATIC", recentLabel.c_str(),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0,
            hwnd, (HMENU)IDC_STATIC_RECENT, GetModuleHandleW(NULL), NULL
        );

        // Trash button for clearing recent list
        state->hBtnClearRecent = CreateWindowExW(
            0, L"BUTTON", L"\xD83D\xDDD1", // 🗑
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            0, 0, 0, 0,
            hwnd, (HMENU)IDC_BTN_CLEAR_RECENT, GetModuleHandleW(NULL), NULL
        );
        if (state->recentFolders.empty()) {
            EnableWindow(state->hBtnClearRecent, FALSE);
        }

        // 6 Recent Folder buttons
        for (int i = 0; i < 6; ++i) {
            std::wstring btnText;
            DWORD bStyle = WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON;
            if (i < static_cast<int>(state->recentFolders.size())) {
                btnText = state->recentFolders[i];
                bStyle |= WS_VISIBLE;
            }
            state->hBtnRecents[i] = CreateWindowExW(
                0, L"BUTTON", btnText.c_str(),
                bStyle,
                0, 0, 0, 0,
                hwnd, (HMENU)(INT_PTR)(IDC_BTN_RECENT_BASE + i), GetModuleHandleW(NULL), NULL
            );
        }

        state->hStaticPrompt = CreateWindowExW(
            0, L"STATIC", GetString(IDS_FOLDER_NAME_LABEL).c_str(),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0,
            hwnd, (HMENU)IDC_STATIC_PROMPT, GetModuleHandleW(NULL), NULL
        );

        state->hEdit = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", state->params->suggestedName.c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            0, 0, 0, 0,
            hwnd, (HMENU)IDC_EDIT_FOLDERNAME, GetModuleHandleW(NULL), NULL
        );

        state->hBtnMove = CreateWindowExW(
            0, L"BUTTON", GetString(IDS_BTN_MOVE_TEXT).c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            0, 0, 0, 0,
            hwnd, (HMENU)IDC_BTN_MOVE, GetModuleHandleW(NULL), NULL
        );

        state->hBtnCancel = CreateWindowExW(
            0, L"BUTTON", GetString(IDS_BTN_CANCEL_TEXT).c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            0, 0, 0, 0,
            hwnd, (HMENU)IDC_BTN_CANCEL, GetModuleHandleW(NULL), NULL
        );

        // Apply DPI fonts and layout
        UpdateDialogFont(state, dpi);
        LayoutDialogControls(state, dpi);

        // Set focus to Edit box and pre-select all text
        SetFocus(state->hEdit);
        SendMessageW(state->hEdit, EM_SETSEL, 0, -1);

        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        if (g_isDarkMode) {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(240, 240, 240));
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)g_hDarkBgBrush;
        }
        break;
    }

    case WM_CTLCOLORDLG: {
        if (g_isDarkMode) {
            return (LRESULT)g_hDarkBgBrush;
        }
        break;
    }

    case WM_CTLCOLOREDIT: {
        if (g_isDarkMode) {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkColor(hdc, RGB(45, 45, 45));
            return (LRESULT)g_hDarkEditBrush;
        }
        break;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);

        // Recent folder button clicked
        if (wmId >= IDC_BTN_RECENT_BASE && wmId < IDC_BTN_RECENT_BASE + 6) {
            int idx = wmId - IDC_BTN_RECENT_BASE;
            if (idx < static_cast<int>(state->recentFolders.size())) {
                SetWindowTextW(state->hEdit, state->recentFolders[idx].c_str());
                SetFocus(state->hEdit);
                SendMessageW(state->hEdit, EM_SETSEL, 0, -1);
            }
            return 0;
        }

        // Clear recent folders (trash bin button)
        if (wmId == IDC_BTN_CLEAR_RECENT) {
            ClearRecentFolders();
            state->recentFolders.clear();
            for (int i = 0; i < 6; ++i) {
                if (state->hBtnRecents[i]) {
                    ShowWindow(state->hBtnRecents[i], SW_HIDE);
                }
            }
            std::wstring emptyLabel = GetString(IDS_RECENT_FOLDERS_LABEL) + L" " + GetString(IDS_NO_RECENT_FOLDERS);
            SetWindowTextW(state->hStaticRecent, emptyLabel.c_str());
            EnableWindow(state->hBtnClearRecent, FALSE);
            SetFocus(state->hEdit);
            return 0;
        }

        if (wmId == IDC_BTN_MOVE) {
            // Read entered folder name
            int len = GetWindowTextLengthW(state->hEdit);
            std::wstring input;
            if (len > 0) {
                input.resize(len + 1);
                GetWindowTextW(state->hEdit, &input[0], len + 1);
                input.resize(len);
            }

            std::wstring validationErr;
            auto valRes = PathUtils::ValidateFolderName(input, &validationErr);
            if (valRes != PathUtils::ValidationResult::Valid) {
                ShowErrorMessage(hwnd, PathUtils::GetValidationErrorMessage(valRes));
                SetFocus(state->hEdit);
                SendMessageW(state->hEdit, EM_SETSEL, 0, -1);
                return 0;
            }

            state->params->chosenFolderName = input;
            state->params->confirmed = true;
            DestroyWindow(hwnd);
            return 0;
        } else if (wmId == IDC_BTN_CANCEL) {
            state->params->confirmed = false;
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    }

    case WM_DPICHANGED: {
        UINT newDpi = HIWORD(wParam);
        state->currentDpi = newDpi;
        RECT* prcNew = reinterpret_cast<RECT*>(lParam);
        SetWindowPos(hwnd, NULL,
            prcNew->left, prcNew->top,
            prcNew->right - prcNew->left,
            prcNew->bottom - prcNew->top,
            SWP_NOZORDER | SWP_NOACTIVATE);

        UpdateDialogFont(state, newDpi);
        LayoutDialogControls(state, newDpi);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }

    case WM_CLOSE: {
        state->params->confirmed = false;
        DestroyWindow(hwnd);
        return 0;
    }

    case WM_DESTROY: {
        if (state) {
            if (state->hFont) {
                DeleteObject(state->hFont);
                state->hFont = NULL;
            }
            if (state->hFontBold && state->hFontBold != state->hFont) {
                DeleteObject(state->hFontBold);
                state->hFontBold = NULL;
            }
            if (state->hFontSmall && state->hFontSmall != state->hFont) {
                DeleteObject(state->hFontSmall);
                state->hFontSmall = NULL;
            }
        }
        PostQuitMessage(0);
        return 0;
    }

    default:
        break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool ShowQuickFolderDialog(DialogParams& params) {
    InitializeUI();

    HINSTANCE hInstance = GetModuleHandleW(NULL);
    const wchar_t CLASS_NAME[] = L"QuickFolderDialogClass";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DialogWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = g_isDarkMode ? g_hDarkBgBrush : (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_QUICKFOLDER));
    if (!wc.hIcon) {
        wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    }

    RegisterClassExW(&wc);

    DialogState state;
    state.params = &params;

    HWND hwndOwner = params.hwndOwner;
    UINT targetDpi = GetMonitorDpi(hwndOwner);

    // Client dimensions: 440 x 232 at 96 DPI
    int initClientW = MulDiv(440, targetDpi, 96);
    int initClientH = MulDiv(232, targetDpi, 96);
    RECT initRc = { 0, 0, initClientW, initClientH };
    AdjustWindowRectForDpiHelper(&initRc, WS_POPUPWINDOW | WS_CAPTION | WS_CLIPCHILDREN, FALSE, WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, targetDpi);
    int initWinW = initRc.right - initRc.left;
    int initWinH = initRc.bottom - initRc.top;

    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        CLASS_NAME,
        GetString(IDS_APP_TITLE).c_str(),
        WS_POPUPWINDOW | WS_CAPTION | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, initWinW, initWinH,
        hwndOwner, NULL, hInstance, &state
    );

    if (!hwnd) {
        UnregisterClassW(CLASS_NAME, hInstance);
        return false;
    }

    CenterWindow(hwnd, hwndOwner);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Bring to foreground and focus edit box with all text selected
    SetForegroundWindow(hwnd);
    if (state.hEdit) {
        SetFocus(state.hEdit);
        SendMessageW(state.hEdit, EM_SETSEL, 0, -1);
    }

    // Disable owner window while modal dialog is active
    if (hwndOwner && IsWindow(hwndOwner)) {
        EnableWindow(hwndOwner, FALSE);
    }

    // Modal message loop
    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        // Handle Enter and Escape keyboard accelerators
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_RETURN) {
                SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_BTN_MOVE, BN_CLICKED), (LPARAM)state.hBtnMove);
                continue;
            } else if (msg.wParam == VK_ESCAPE) {
                SendMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_BTN_CANCEL, BN_CLICKED), (LPARAM)state.hBtnCancel);
                continue;
            }
        }

        if (!IsDialogMessageW(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    // Re-enable owner window
    if (hwndOwner && IsWindow(hwndOwner)) {
        EnableWindow(hwndOwner, TRUE);
        SetForegroundWindow(hwndOwner);
    }

    UnregisterClassW(CLASS_NAME, hInstance);
    return params.confirmed;
}

FileOps::ExistingDestAction PromptExistingFolder(HWND hwndOwner, const std::wstring& folderName) {
    wchar_t content[256];
    swprintf_s(content, GetString(IDS_ERR_DEST_EXISTS).c_str(), folderName.c_str());

    std::wstring optExisting = GetString(IDS_OPT_USE_EXISTING);
    std::wstring optAnother = GetString(IDS_OPT_CHOOSE_ANOTHER);
    std::wstring appTitle = GetString(IDS_APP_TITLE);

    // TaskDialog buttons
    TASKDIALOG_BUTTON buttons[] = {
        { 101, optExisting.c_str() },
        { 102, optAnother.c_str() }
    };

    TASKDIALOGCONFIG tdc = {};
    tdc.cbSize = sizeof(tdc);
    tdc.hwndParent = hwndOwner;
    tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
    tdc.pszWindowTitle = appTitle.c_str();
    tdc.pszMainIcon = TD_WARNING_ICON;
    tdc.pszMainInstruction = content;
    tdc.pButtons = buttons;
    tdc.cButtons = ARRAYSIZE(buttons);
    tdc.nDefaultButton = 101;

    int nButton = 0;
    HRESULT hr = TaskDialogIndirect(&tdc, &nButton, nullptr, nullptr);
    if (SUCCEEDED(hr)) {
        if (nButton == 101) return FileOps::ExistingDestAction::UseExisting;
        if (nButton == 102) return FileOps::ExistingDestAction::ChooseAnother;
        return FileOps::ExistingDestAction::Cancel;
    }

    // Fallback if TaskDialog fails
    int msgRes = MessageBoxW(hwndOwner, content, GetString(IDS_APP_TITLE).c_str(), MB_YESNOCANCEL | MB_ICONWARNING);
    if (msgRes == IDYES) return FileOps::ExistingDestAction::UseExisting;
    if (msgRes == IDNO) return FileOps::ExistingDestAction::ChooseAnother;
    return FileOps::ExistingDestAction::Cancel;
}

ConflictResolution PromptFileConflict(
    HWND hwndOwner,
    size_t conflictCount,
    const std::wstring& sampleName
) {
    std::wstring appTitle = GetString(IDS_APP_TITLE);

    LANGID lang = GetCurrentLangId();
    std::wstring instruction;
    std::wstring content;
    std::wstring optRename;
    std::wstring optOverwrite;

    if (lang == LANG_GERMAN) {
        instruction = L"Dateikonflikt erkannt";
        if (conflictCount == 1) {
            content = L"Im Zielordner ist bereits eine Datei namens \"" + sampleName + L"\" vorhanden.\nWie möchten Sie vorgehen?";
        } else {
            wchar_t buf[256];
            swprintf_s(buf, L"Im Zielordner sind bereits %zu Dateien mit denselben Namen vorhanden.\nWie möchten Sie vorgehen?", conflictCount);
            content = buf;
        }
        optRename = L"Automatisch umbenennen\nDateien erhalten eine Nummer (z. B. Datei (2).ext)";
        optOverwrite = L"Überschreiben\nVorhandene Dateien im Zielordner ersetzen";
    } else if (lang == LANG_RUSSIAN || lang == LANG_UKRAINIAN) {
        instruction = L"Конфликт имён файлов";
        if (conflictCount == 1) {
            content = L"В целевой папке уже есть файл с именем «" + sampleName + L"».\nЧто вы хотите сделать?";
        } else {
            wchar_t buf[256];
            swprintf_s(buf, L"В целевой папке уже существует файлов с совпадающими именами: %zu.\nЧто вы хотите сделать?", conflictCount);
            content = buf;
        }
        optRename = L"Автоматически переименовать\nК именам файлов будет добавлен номер (например, файл (2).ext)";
        optOverwrite = L"Перезаписать\nЗаменить существующие файлы в целевой папке";
    } else {
        instruction = L"File conflict detected";
        if (conflictCount == 1) {
            content = L"The destination folder already contains a file named \"" + sampleName + L"\".\nWhat would you like to do?";
        } else {
            wchar_t buf[256];
            swprintf_s(buf, L"The destination folder already contains %zu files with the same names.\nWhat would you like to do?", conflictCount);
            content = buf;
        }
        optRename = L"Auto-rename\nAdd a number to file names (e.g. file (2).ext)";
        optOverwrite = L"Overwrite\nReplace existing files in the destination folder";
    }

    TASKDIALOG_BUTTON buttons[] = {
        { 101, optRename.c_str() },
        { 102, optOverwrite.c_str() }
    };

    TASKDIALOGCONFIG tdc = {};
    tdc.cbSize = sizeof(tdc);
    tdc.hwndParent = hwndOwner;
    tdc.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
    tdc.pszWindowTitle = appTitle.c_str();
    tdc.pszMainIcon = TD_WARNING_ICON;
    tdc.pszMainInstruction = instruction.c_str();
    tdc.pszContent = content.c_str();
    tdc.pButtons = buttons;
    tdc.cButtons = ARRAYSIZE(buttons);
    tdc.nDefaultButton = 101;

    int nButton = 0;
    HRESULT hr = TaskDialogIndirect(&tdc, &nButton, nullptr, nullptr);
    if (SUCCEEDED(hr)) {
        if (nButton == 101) return ConflictResolution::AutoRename;
        if (nButton == 102) return ConflictResolution::Overwrite;
        return ConflictResolution::Cancel;
    }

    // Fallback if TaskDialog fails
    int msgRes = MessageBoxW(hwndOwner, content.c_str(), instruction.c_str(), MB_YESNOCANCEL | MB_ICONWARNING);
    if (msgRes == IDYES) return ConflictResolution::AutoRename;
    if (msgRes == IDNO) return ConflictResolution::Overwrite;
    return ConflictResolution::Cancel;
}

void ShowErrorMessage(HWND hwndOwner, const std::wstring& message) {
    MessageBoxW(hwndOwner, message.c_str(), GetString(IDS_APP_TITLE).c_str(), MB_OK | MB_ICONERROR);
}

void ShowInfoMessage(HWND hwndOwner, const std::wstring& message) {
    MessageBoxW(hwndOwner, message.c_str(), GetString(IDS_APP_TITLE).c_str(), MB_OK | MB_ICONINFORMATION);
}

} // namespace QuickFolder::UI
