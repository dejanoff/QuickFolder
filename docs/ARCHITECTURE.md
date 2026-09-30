# QuickFolder Architecture

## Overview
QuickFolder is a lightweight, high-performance native Windows utility written in C++20 (Win32/Unicode) designed to move selected files and directories into a new subfolder with a single context-menu action.

## 1. Chosen Shell Integration Architecture

### Selected Mechanism: COM Local Server via `DelegateExecute`
QuickFolder implements an out-of-process COM Local Server exposing two standard Windows Shell COM interfaces:
- **`IExecuteCommand`**: Standard verb execution interface for shell verbs.
- **`IObjectWithSelection`**: Interface used by Windows Explorer to pass selection arrays (`IShellItemArray`) across process boundaries.

### Why This Architecture Was Chosen
1. **Zero Command-Line Limits**: Unlike the naive command-line verb `%1`, which truncates around 8,191 characters (or 32,767 in some APIs), `IShellItemArray` is passed as a COM interface pointer via standard COM marshaling. QuickFolder can receive 1, 20, 500, or 10,000+ items without limitation.
2. **Absolute Explorer Stability (Out-of-Process)**: No QuickFolder DLL is loaded inside `explorer.exe`. If QuickFolder crashes, freezes, or exits, `explorer.exe` is completely unaffected.
3. **No File Locking on Update/Uninstall**: Traditional in-process Shell Extension DLLs are kept locked by `explorer.exe` until a user logs off or restarts Explorer. With an out-of-process COM Local Server executable, updating or uninstalling QuickFolder never prompts the user to restart Explorer.
4. **Per-User Installation Without Administrator Privileges**: The COM class (`CLSID`) and context menu verbs (`HKCU\Software\Classes\AllFilesystemObjects\shell\QuickFolder`) can be registered entirely in the user's `HKEY_CURRENT_USER` hive, allowing installation without UAC administrator prompts.

### Alternatives Considered and Why They Were Rejected
- **Naive `%1` Command-Line Invocation**:
  * *Rejected*: Fails with multi-selection beyond a handful of files due to command-line buffer limits; cannot differentiate between batch launches and single invocations.
- **In-Process `IContextMenu` / `IShellExtInit` DLL**:
  * *Rejected*: Runs directly inside `explorer.exe`. Memory leaks, crashes, or unhandled exceptions crash the user's Explorer shell. Keeps DLL files locked, complicating uninstall.
- **In-Process `IExplorerCommand` DLL**:
  * *Rejected*: While cleaner than `IContextMenu`, still runs in-process with the same file locking and crash risks.
- **DropTarget Handler (`IDropTarget`)**:
  * *Rejected*: Requires `IDataObject` handling and custom drag-and-drop semantics, which is more complex and less standard for simple verb execution than `DelegateExecute`.

---

## 2. COM Registration Model

QuickFolder registers the following keys in `HKCU\Software\Classes` (per-user) or `HKLM\Software\Classes` (machine-wide):

### Context Menu Verb:
```ini
[HKEY_CURRENT_USER\Software\Classes\AllFilesystemObjects\shell\QuickFolder]
@="Move to new folder..."
"MUIVerb"="Move to new folder..."
"Icon"="\"C:\\Path\\To\\QuickFolder.exe\",0"
"MultiSelectModel"="Document"

[HKEY_CURRENT_USER\Software\Classes\AllFilesystemObjects\shell\QuickFolder\command]
"DelegateExecute"="{4B2F7E41-8A19-4C1B-94E5-46D9B6E1C87D}"
```
*Note*: `MultiSelectModel="Document"` is critical. Without this value, Windows Explorer disables the context menu item when more than 15 files are selected.

### COM Local Server:
```ini
[HKEY_CURRENT_USER\Software\Classes\CLSID\{4B2F7E41-8A19-4C1B-94E5-46D9B6E1C87D}]
@="QuickFolder Command Executor"

[HKEY_CURRENT_USER\Software\Classes\CLSID\{4B2F7E41-8A19-4C1B-94E5-46D9B6E1C87D}\LocalServer32]
@="\"C:\\Path\\To\\QuickFolder.exe\" -Embedding"
```

---

## 3. Selection Handling & Validation Pipeline

When the user clicks "Move to new folder...":
1. Windows Explorer launches `QuickFolder.exe -Embedding` (or binds to a running instance if already active).
2. Explorer queries `IObjectWithSelection` and calls `SetSelection(punkSelection)`.
3. QuickFolder casts `punkSelection` to `IShellItemArray` and inspects all selected items:
   - **Filesystem Verification**: Ensures items are physical filesystem entities (`SFGAO_FILESYSTEM` and `SIGDN_FILESYSPATH`). Virtual shell objects are rejected.
   - **Shortcut Integrity**: For `.lnk` and `.url` files, `SIGDN_FILESYSPATH` yields the shortcut file path itself. QuickFolder never queries or follows the shortcut target.
   - **Common Parent Directory Verification**: Checks the physical parent path of each item (`psi->GetParent()`). If any item belongs to a different physical directory (e.g. mixed Search Results or multi-root Library view), the operation safely aborts with:
     `"Selected items are located in different folders."`
4. QuickFolder determines default folder name:
   - If 1 file: name of file without extension.
   - If 1 folder: `<FolderName> - Folder` (preventing immediate conflict).
   - If multiple items: empty / prompt.
5. QuickFolder shows the modal UI dialog centered over the calling Explorer window (`IExecuteCommand::SetWindow(hwnd)`).

---

## 4. File Operations via `IFileOperation`

File moves are executed using the Windows Shell `IFileOperation` API rather than primitive `MoveFileExW` loops.

### Advantages of `IFileOperation`:
- **Native Collision UI**: If a filename collision occurs in the target folder, Windows displays its standard conflict dialog ("Replace, Skip, or Keep both").
- **Explorer Progress UI**: Large file operations automatically display the native Explorer transfer speed and progress dialog.
- **Undo Integration**: The operation registers in Windows Explorer's Undo stack (`Ctrl+Z`).
- **Elevation Support**: If the target location requires administrator rights (UAC), Windows prompts for elevation specifically for the file move, without requiring QuickFolder or Explorer to run elevated.
- **Shell Change Notifications**: Windows Explorer folders update immediately (`SHChangeNotify`) without manual refresh.

### Transaction Safety:
1. Target folder is created only after the user confirms a valid name.
2. If QuickFolder created the destination folder, but `IFileOperation::PerformOperations()` fails or is canceled with 0 items moved, QuickFolder automatically cleans up the newly created empty folder.

---

## 5. High-DPI and Modern UI Styling
- Manifest includes `<dpiAwareness>PerMonitorV2, PerMonitor</dpiAwareness>`.
- Supports dynamic `WM_DPICHANGED` messages, scaling fonts and control positions cleanly across multi-monitor setups with mixed scaling factors (100%, 125%, 150%, 200%, 250%, 4K).
- Supports Windows 10/11 Light and Dark themes via `DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, ...)` and system theming.

---

## 6. Installer & Deployment Strategy
- Built with **Inno Setup 6**.
- Supports per-user installation (`{localappdata}\Programs\QuickFolder`) without administrator rights.
- Supports administrative machine-wide installation (`{autopf}\QuickFolder`).
- Self-registration support via `QuickFolder.exe --register` and clean unregistration via `QuickFolder.exe --unregister`.
- Silent deployment via `/VERYSILENT /SUPPRESSMSGBOXES`.

---

## 7. Windows 11 Modern Context Menu Roadmap

In Windows 11, the top-level compact context menu requires an `IExplorerCommand` implementation packaged with Package Identity (Sparse Package or MSIX package).
- **v1 Focus**: Classic Windows Explorer context menu (which appears in Windows 10, Windows 11 "Show more options" / Shift+Right Click, and third-party classic shell tools).
- **Future v1.1+ Integration**: A companion Sparse Package manifest (`AppxManifest.xml`) and lightweight `IExplorerCommand` stub that delegates to `QuickFolder.exe` via COM LocalServer. Because all core business logic is encapsulated in `FileOperations` and `SelectionHelper`, the UI and execution layer remain 100% reusable.
