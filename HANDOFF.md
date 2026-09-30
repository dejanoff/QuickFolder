# QuickFolder - Agent Handoff Document

## Current Status
- **Milestone 1 (MVP) is 100% COMPLETE and fully operational.**
- Clean native C++20 Win32 application compiled and linked statically with zero external runtime dependencies.
- Inno Setup installer package built and tested.
- All 15 automated unit tests passing (`UnitTests.exe`).
- Integration tests verified for single file, 20 files, 500 files, 1,000 files, mixed files and subfolders, `.lnk` shortcut files, Unicode (Cyrillic, German umlauts, emoji), long paths (> 260 characters), different parent directory rejection, existing destination handling, silent registration/unregistration, and clean uninstallation.
- Zero orphan processes left behind after operation.

---

## What Works
1. **Classic Explorer Context Menu Integration**:
   - COM Local Server (`DelegateExecute` pointing to `CLSID {4B2F7E41-8A19-4C1B-94E5-46D9B6E1C87D}`).
   - Registered under `HKCU\Software\Classes\AllFilesystemObjects\shell\QuickFolder` and `Directory\shell\QuickFolder`.
   - `MultiSelectModel = "Document"` configured, allowing unlimited multi-selection.
2. **Out-of-Process Execution**:
   - No DLL is injected into `explorer.exe`. Explorer stability is protected, no Explorer restart needed on update/uninstall.
   - `IExecuteCommand`, `IObjectWithSelection`, and `IObjectWithSite` interfaces fully implemented.
3. **Safe File Operations (`IFileOperation`)**:
   - Uses Windows Shell file engine for native progress, collision UI ("Replace / Skip / Keep both"), UAC elevation, and Windows Explorer Undo (`Ctrl+Z`).
   - Newly created empty destination folders are automatically cleaned up if operation is aborted or fails.
4. **Data Protection Invariants**:
   - Strict validation of destination folder names (no illegal characters, trailing dots/spaces, relative traversal, or DOS device names like `CON`, `PRN`, `AUX`, `NUL`, `COM1-9`, `LPT1-9`).
   - Common parent directory verification across selected items (Search Results and multi-root Library views spanning different folders are safely rejected with a clear message).
   - Recursive move prevention (moving a directory inside itself or its subdirectories is disallowed).
   - Shortcuts (`.lnk` and `.url`) are moved as the shortcut files themselves, never resolving or moving target files.
   - Long paths (> 260 chars) handled safely using `\\?\` prefix and manifest `longPathAware`.
   - Current Working Directory is NEVER used as fallback destination.
5. **Native Win32 UI & Theming**:
   - Native dialog with item count label, folder name edit control, and Move/Cancel buttons.
   - Keyboard accelerators (`Enter` to Move, `Escape` to Cancel, `Ctrl+A` select all).
   - Per-Monitor v2 DPI aware (`WM_DPICHANGED` handling).
   - Automatic Windows Dark Mode detection (`DWMWA_USE_IMMERSIVE_DARK_MODE`).
   - Centered relative to owning Explorer window or active monitor.
   - Multi-language localization (English, German, Russian) loaded dynamically.
6. **Packaging & Deployment**:
   - Per-user Inno Setup installer (`dist/QuickFolder-Setup-0.1.0.exe`) requiring no admin privileges.
   - Silent installation (`/VERYSILENT /SUPPRESSMSGBOXES`).
   - Clean uninstaller (`unins000.exe`) removes all binaries, COM classes, and registry entries.
   - Standalone single-file binary (`dist/QuickFolder.exe`, ~1.25 MB).
   - Automated build script (`scripts/build.ps1`) and Visual Studio 2022 solution (`QuickFolder.sln`).

---

## What Is Broken / Known Issues
- None. Build, test suite, and manual integration scenarios all pass with code 0.
- Resolved: Window truncation bug under Windows 11 high-DPI scaling (125%/150%/200%) where the Edit control and buttons were clipped due to unscaled outer window dimensions. Fixed with dynamic Per-Monitor v2 client sizing and `AdjustWindowRectExForDpi`.
- Note: Remote SMB UNC network shares and removable media (USB/FAT32/exFAT) could not be physically connected in the virtual agent container and remain documented as `[-] N/A` in `docs/TESTING.md`.

---

## Last Successful Build
- **Date**: 2026-09-30 (Updated with High-DPI fix)
- **Compiler**: Clang++ 22.1.8 (LLVM-MinGW UCRT x64, `-std=c++20 -static -O2 -mwindows -municode`)
- **Resource Compiler**: windres 2.44
- **Installer Compiler**: Inno Setup 6.7.3 (`ISCC.exe`)
- **Unit Tests**: 15 Passed, 0 Failed
- **Binaries**:
  - `dist/QuickFolder.exe` (1,257,472 bytes)
  - `dist/QuickFolder-Setup-0.1.0.exe` (2,354,038 bytes)
  - Installed binary at `%LOCALAPPDATA%\Programs\QuickFolder\QuickFolder.exe` updated and active.

---

## Files Changed / Project Structure
```
QuickFolder/
├── .github/
│   └── workflows/
│       └── build.yml
├── docs/
│   ├── ARCHITECTURE.md
│   ├── FILES2FOLDER-LESSONS.md
│   ├── MICROSOFT-STORE.md
│   └── TESTING.md
├── include/
│   ├── Common.h
│   ├── FileOperations.h
│   ├── PathUtils.h
│   ├── Resource.h
│   ├── SelectionHelper.h
│   ├── ShellCommand.h
│   └── UI.h
├── installer/
│   └── QuickFolder.iss
├── res/
│   ├── QuickFolder.exe.manifest
│   ├── QuickFolder.ico
│   └── QuickFolder.rc
├── scripts/
│   ├── build.ps1
│   └── generate_icon.py
├── src/
│   ├── FileOperations.cpp
│   ├── Main.cpp
│   ├── PathUtils.cpp
│   ├── SelectionHelper.cpp
│   ├── ShellCommand.cpp
│   └── UI.cpp
├── tests/
│   └── UnitTests.cpp
├── .gitattributes
├── .gitignore
├── CHANGELOG.md
├── CONTRIBUTING.md
├── HANDOFF.md
├── LICENSE
├── QuickFolder.sln
├── QuickFolder.vcxproj
├── QuickFolder.vcxproj.filters
├── README.md
├── SECURITY.md
└── THIRD-PARTY-NOTICES.md
```

---

## Important Architecture Decisions
1. **COM Local Server (`DelegateExecute`)**: Explorer launches `QuickFolder.exe -Embedding` out-of-process and passes `IShellItemArray` via COM. No in-process DLL injection into `explorer.exe`, eliminating Explorer crashes and DLL file locks.
2. **`MultiSelectModel="Document"`**: Explicitly declared in registry to lift the default 15-file Explorer context-menu limit.
3. **`IFileOperation` vs `MoveFileExW`**: Native Explorer collision UI ("Replace/Skip/Keep both"), progress bar, elevation prompts, shell refresh notifications, and native Undo (`Ctrl+Z`) integration.
4. **Never Move Shortcut Targets**: Shortcuts (`.lnk` and `.url`) are checked and moved as the shortcut files themselves.
5. **Zero CWD Reliance**: Absolute paths are verified and used exclusively. No implicit fallback to current directory or application install directory.
6. **Single Folder Proposal**: Single directory selection proposes `<Name> - Folder` instead of creating an immediate naming conflict or using historical characters like `~`.

---

## Exact Commands to Build and Test
```powershell
# 1. Full clean build, unit test execution, and installer packaging:
powershell -ExecutionPolicy Bypass -File scripts\build.ps1

# 2. Run unit tests independently:
.\build\UnitTests.exe

# 3. Test silent installation:
.\dist\QuickFolder-Setup-0.1.0.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART

# 4. Test silent uninstallation:
& "$env:LOCALAPPDATA\Programs\QuickFolder\unins000.exe" /VERYSILENT /SUPPRESSMSGBOXES /NORESTART
```

---

## Next Recommended Tasks
1. **Windows 11 Modern Context Menu (v1.1.0)**: Add Sparse Package manifest (`AppxManifest.xml`) with package identity to show QuickFolder directly in Windows 11's top-level compact menu.
2. **Duplicate Auto-Numbering (v1.2.0)**: Add user setting to automatically append `(2)`, `(3)` if target folder exists without prompting.
3. **Microsoft Store Packaging (v2.0.0)**: Follow the roadmap in `docs/MICROSOFT-STORE.md` for Store distribution.
