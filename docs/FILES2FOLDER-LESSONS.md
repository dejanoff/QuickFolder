# Lessons Learned from Files 2 Folder and Edge Cases in QuickFolder

This document analyzes historical failure classes, architectural flaws, and user-reported bugs from the original **Files 2 Folder** utility (by Skwire / Jody Holmes, 2009–2017) and related shell tools, documenting specifically how **QuickFolder** prevents each issue by design.

---

### 1. Illegal Characters in Folder Names
- **Files 2 Folder Issue**: The original tool initially performed no validation on user input, causing Windows filesystem calls to fail unpredictably or create corrupted paths when characters such as `:`, `*`, `?`, `"`, `<`, `>`, or `|` were entered.
- **QuickFolder Solution**: Robust Win32 validation in `PathUtils::ValidateFolderName()` that disallows all invalid Windows characters (`\`, `/`, `:`, `*`, `?`, `"`, `<`, `>`, `|`, and ASCII control characters `0x00`-`0x1F`). In addition, leading/trailing spaces and dots are stripped or disallowed, and forbidden component names (`.`, `..`) are rejected before any filesystem call is attempted.

### 2. Replacement Must Affect Only the Folder Name, Not the Whole Path
- **Files 2 Folder Issue** (Changelog v1.0.5): A character replacement / sanitization rule intended for the new folder name was applied to the entire path string. For example, replacing hyphens or underscores altered parent directory names (e.g. `D:\My_Photos\Test-Files\file.txt` had its parent paths corrupted into `D:\My Photos\Test Files\...`).
- **QuickFolder Solution**: Path manipulation strictly isolates the leaf destination component. The common parent path is treated as an immutable base, and sanitization/validation is executed solely on the user-provided folder name string.

### 3. Hanging Process / Orphan Resident Processes
- **Files 2 Folder Issue** (Changelog v1.0.8 & forum discussions): After completing an operation or encountering an error, the AutoHotkey process sometimes remained resident in memory, leaking resources and blocking subsequent executions.
- **QuickFolder Solution**: QuickFolder has a deterministic lifecycle. As a COM Local Server or standalone CLI utility, `wWinMain()` runs an explicit message loop that terminates upon dialog closure (`PostQuitMessage(0)`). COM class objects are revoked (`CoRevokeClassObject`), and the process unconditionally exits with code 0 (or error code). There are no background threads running after UI closure.

### 4. Multi-Selection Limitations
- **Files 2 Folder Issue**: The shell verb passed arguments via command line or a temporary clipboard/drop mechanism (`Shell.ahk`). When selecting hundreds or thousands of files, Windows truncated the command line (exceeding the ~8191 or 32767 character limit) or silently dropped files.
- **QuickFolder Solution**: QuickFolder uses Windows Shell's `DelegateExecute` COM verb handler with `IExecuteCommand` and `IObjectWithSelection`. Windows Explorer delivers an `IShellItemArray` COM interface directly across processes. This scales reliably from 1 file to 10,000+ files without any command-line length limitations. Furthermore, `MultiSelectModel = "Document"` is registered in the registry to prevent Explorer from disabling the menu when more than 15 items are selected.

### 5. Folder Support and Mixed Selections
- **Files 2 Folder Issue** (Changelog v1.1.3): Early versions only supported files. When folder support was added, it had arbitrary limitations (e.g., disabling individual subfolder modes when a folder was in the selection).
- **QuickFolder Solution**: QuickFolder natively treats files, directories, and mixed selections identically. Each selected item is processed through `IShellItem` and moved via `IFileOperation`.

### 6. Single Directory Default Naming (Avoiding Tilde `~`)
- **Files 2 Folder Issue** (Help v1.1.3): When a single folder was selected, Files 2 Folder automatically created a new folder with the same name plus a tilde (`Holiday Photos~`), which looked ugly and confusing.
- **QuickFolder Solution**: When a single directory is selected, QuickFolder proposes a clean default such as `<DirectoryName> - Folder` (or leaves it focused for custom input), completely avoiding arbitrary punctuation like `~` while avoiding collision with the selected directory itself.

### 7. Unicode and Character Encoding
- **Files 2 Folder Issue**: Built with ANSI/MBCS AutoHotkey, causing filenames with foreign scripts (Cyrillic, German umlauts, Arabic, Chinese, Japanese, Emoji) to be damaged by codepage conversion (`?` or mojibake).
- **QuickFolder Solution**: Native C++20 with UTF-16 wide-character APIs (`wchar_t`, `std::wstring`) throughout. All Windows APIs used are the `-W` variants (`CreateDirectoryW`, `IShellItem::GetDisplayName`, `IFileOperation`). QuickFolder is fully verified with Cyrillic, German umlauts, Polish, Japanese, Chinese, Arabic, and Unicode Emoji.

### 8. Desktop Items: Real Files vs Virtual Objects
- **Files 2 Folder Issue**: Desktop items produced inconsistent behavior because the Desktop folder in Windows Shell combines the user profile Desktop (`%USERPROFILE%\Desktop`) and the Common Desktop (`%PUBLIC%\Desktop`), alongside virtual namespace items (This PC, Recycle Bin, Network).
- **QuickFolder Solution**: QuickFolder checks whether each selected `IShellItem` has the `SFGAO_FILESYSTEM` attribute and resolves to a real filesystem path via `SIGDN_FILESYSPATH`. Virtual shell objects (like "Recycle Bin") are rejected gracefully without error cascades.

### 9. Shortcut Files (`.lnk` and `.url`)
- **Files 2 Folder Issue**: When a `.lnk` or `.url` on the Desktop was selected, the utility resolved the shortcut and attempted to move the target executable/document rather than moving the shortcut file itself!
- **QuickFolder Solution**: **Absolute rule: never move a different file.** QuickFolder never calls `IShellLink::Resolve` or queries target paths. `IShellItem::GetDisplayName(SIGDN_FILESYSPATH)` returns the path to the `.lnk` or `.url` file itself, ensuring that only the selected shortcut file is moved.

### 10. UAC and Protected Directories
- **Files 2 Folder Issue**: In directories like `C:\Program Files`, operations failed silently or required the entire program to always be run elevated as Administrator.
- **QuickFolder Solution**: QuickFolder utilizes `IFileOperation`. If an operation requires administrative permissions, Windows Explorer's native UAC elevation dialog is triggered specifically for that file operation without requiring the QuickFolder application or Explorer to run elevated.

### 11. Application Relocation and Broken Shell Registration
- **Files 2 Folder Issue**: Being a portable executable that registered absolute paths in the registry, moving the executable broke the context menu and left orphaned registry keys.
- **QuickFolder Solution**: QuickFolder is packaged with an Inno Setup installer that installs to a fixed, stable location (`%LOCALAPPDATA%\Programs\QuickFolder` for per-user, or `%ProgramFiles%\QuickFolder` for machine-wide). It registers COM LocalServer32 pointing to the installed executable and provides clean uninstallation (`--unregister` or Add/Remove Programs) that removes all registry keys.

### 12. Long Paths (> 260 Characters) and the CWD Fallback Catastrophe
- **Files 2 Folder Issue**: When paths exceeded `MAX_PATH` (260 characters), Win32 API calls failed, and relative or fallback path resolution caused files to be accidentally moved into Files 2 Folder's own installation directory!
- **QuickFolder Solution**:
  1. The application manifest includes `<longPathAware>true</longPathAware>`.
  2. All internal path handling uses `std::wstring` and handles `\\?\` prefixes for paths exceeding `MAX_PATH`.
  3. **Strict invariant**: QuickFolder NEVER relies on or uses the current working directory (`GetCurrentDirectory()`) as a fallback. All destinations must be explicit, verified absolute paths. If path creation fails, the operation is aborted and nothing is moved.

### 13. High-DPI and Multi-Monitor Scaling (120 DPI, 4K)
- **Files 2 Folder Issue** (Changelog v1.1.2 & forum reports): The dialog rendered with truncated controls or unreadable fonts on 120 DPI, 150 DPI, and 4K displays.
- **QuickFolder Solution**: The application manifest specifies `<dpiAwareness>PerMonitorV2, PerMonitor</dpiAwareness>`. All dialog coordinates, fonts, and controls dynamically adjust to the DPI of the monitor where the window is displayed, handling `WM_DPICHANGED` messages seamlessly.

### 14. Existing Destination Handling
- **Files 2 Folder Issue**: Either silently merged items into an existing folder or failed abruptly.
- **QuickFolder Solution**: QuickFolder explicitly checks if the target folder already exists before moving files. If it exists, the user is presented with clear options: "Use existing folder", "Choose another name", or "Cancel".

### 15. Search Results and Library Views (Multiple Parent Directories)
- **Files 2 Folder Issue**: Selecting files in a Search Results view or Windows Library where items originated from different physical folders caused unpredictable destination placements or file moves into unintended locations.
- **QuickFolder Solution**: QuickFolder inspects the physical parent directory of every selected item using `psi->GetParent()` and `SIGDN_FILESYSPATH`. If the physical parents differ across items, QuickFolder shows a clear message: `"Selected items are located in different folders."` and safely aborts.

### 16. Recursive Move Prevention (Destination Inside Selection)
- **Files 2 Folder Issue**: Moving a directory into a newly created subfolder within itself or creating a cyclic dependency caused infinite loops or filesystem corruption.
- **QuickFolder Solution**: QuickFolder validates that the destination path is not equal to, nor a subdirectory of, any selected directory before executing the move.

### 17. Explorer Stability (Out-of-Process Architecture)
- **Files 2 Folder Issue**: In-process shell extensions or buggy hooks can crash or hang `explorer.exe`.
- **QuickFolder Solution**: Out-of-process COM Local Server architecture. Explorer interacts with QuickFolder via standard COM IPC (`DelegateExecute`). No QuickFolder code is injected into `explorer.exe`, guaranteeing that Explorer stability is never compromised.
