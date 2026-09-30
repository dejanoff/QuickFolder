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

void CenterWindow(HWND hwnd, HWND hwndOwner) {
    RECT rcDlg = {};
    GetWindowRect(hwnd, &rcDlg);
    int dlgWidth = rcDlg.right - rcDlg.left;
    int dlgHeight = rcDlg.bottom - rcDlg.top;

    HMONITOR hMon = NULL;
    RECT rcTarget = {};

    if (hwndOwner && IsWindow(hwndOwner) && IsWindowVisible(hwndOwner)) {
        GetWindowRect(hwndOwner, &rcTarget);
        hMon = MonitorFromWindow(hwndOwner, MONITOR_DEFAULTTONEAREST);
    } else {
        POINT ptCursor = {};
        GetCursorPos(&ptCursor);
        hMon = MonitorFromPoint(ptCursor, MONITOR_DEFAULTTONEAREST);
    }

    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    if (hMon && GetMonitorInfoW(hMon, &mi)) {
        if (!hwndOwner || !IsWindow(hwndOwner) || !IsWindowVisible(hwndOwner)) {
            rcTarget = mi.rcWork;
        }
    } else {
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &rcTarget, 0);
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

struct DialogState {
    DialogParams* params = nullptr;
    HWND hStaticCount = NULL;
    HWND hStaticPrompt = NULL;
    HWND hEdit = NULL;
    HWND hBtnMove = NULL;
    HWND hBtnCancel = NULL;
    UINT currentDpi = 96;
};

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

        // Calculate scaling factor from DPI
        UINT dpi = 96;
        HDC hdc = GetDC(hwnd);
        if (hdc) {
            dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(hwnd, hdc);
        }
        state->currentDpi = dpi;
        auto Scale = [dpi](int val) -> int {
            return MulDiv(val, dpi, 96);
        };

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
            Scale(18), Scale(16), Scale(360), Scale(20),
            hwnd, (HMENU)IDC_STATIC_COUNT, GetModuleHandleW(NULL), NULL
        );

        state->hStaticPrompt = CreateWindowExW(
            0, L"STATIC", GetString(IDS_FOLDER_NAME_LABEL).c_str(),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            Scale(18), Scale(46), Scale(360), Scale(18),
            hwnd, (HMENU)IDC_STATIC_PROMPT, GetModuleHandleW(NULL), NULL
        );

        state->hEdit = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", state->params->suggestedName.c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            Scale(18), Scale(68), Scale(360), Scale(26),
            hwnd, (HMENU)IDC_EDIT_FOLDERNAME, GetModuleHandleW(NULL), NULL
        );

        state->hBtnMove = CreateWindowExW(
            0, L"BUTTON", GetString(IDS_BTN_MOVE_TEXT).c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            Scale(186), Scale(110), Scale(92), Scale(28),
            hwnd, (HMENU)IDC_BTN_MOVE, GetModuleHandleW(NULL), NULL
        );

        state->hBtnCancel = CreateWindowExW(
            0, L"BUTTON", GetString(IDS_BTN_CANCEL_TEXT).c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            Scale(286), Scale(110), Scale(92), Scale(28),
            hwnd, (HMENU)IDC_BTN_CANCEL, GetModuleHandleW(NULL), NULL
        );

        // Apply fonts
        SendMessageW(state->hStaticCount, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageW(state->hStaticPrompt, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageW(state->hEdit, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageW(state->hBtnMove, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageW(state->hBtnCancel, WM_SETFONT, (WPARAM)g_hFont, TRUE);

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

    case WM_CLOSE: {
        state->params->confirmed = false;
        DestroyWindow(hwnd);
        return 0;
    }

    case WM_DESTROY: {
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

    // Initial base window size (approx 400x190 at 96 DPI)
    int baseWidth = 405;
    int baseHeight = 195;

    HWND hwndOwner = params.hwndOwner;
    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        CLASS_NAME,
        GetString(IDS_APP_TITLE).c_str(),
        WS_POPUPWINDOW | WS_CAPTION | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, baseWidth, baseHeight,
        hwndOwner, NULL, hInstance, &state
    );

    if (!hwnd) {
        UnregisterClassW(CLASS_NAME, hInstance);
        return false;
    }

    CenterWindow(hwnd, hwndOwner);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

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

void ShowErrorMessage(HWND hwndOwner, const std::wstring& message) {
    MessageBoxW(hwndOwner, message.c_str(), GetString(IDS_APP_TITLE).c_str(), MB_OK | MB_ICONERROR);
}

void ShowInfoMessage(HWND hwndOwner, const std::wstring& message) {
    MessageBoxW(hwndOwner, message.c_str(), GetString(IDS_APP_TITLE).c_str(), MB_OK | MB_ICONINFORMATION);
}

} // namespace QuickFolder::UI
