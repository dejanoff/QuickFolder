# Changelog

All notable changes to **QuickFolder** will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [0.2.1] - 2026-09-30

### Improved
- **Universal Shell Registration**:
  - Registered context menu verb under `Software\Classes\*` (all file types), `Software\Classes\Folder` (all folders), `Software\Classes\Directory` (filesystem folders), and `Software\Classes\AllFilesystemObjects` (universal mixed filesystem selections).
  - Guarantees `Move to new folder...` / `In neuen Ordner verschieben...` appears consistently on any file type (documents, videos, archives, source files, executables, photos, etc.) and folders throughout Windows Explorer.

## [0.2.0] - 2026-09-30

### Added
- **Recent Folders History (MRU)**:
  - Displays the 6 most recently used folder names as clickable chip buttons directly above the folder name edit box.
  - Clicking any recent folder button immediately populates the edit box and selects the text for fast re-use or editing.
  - History is persisted cleanly in `HKCU\Software\QuickFolder\RecentFolders`.
- **Trash Bin History Clear Button (`🗑`)**:
  - Small trash icon button adjacent to recent folders header to wipe recent folder history in one click.
  - Dynamically updates chip buttons to `(empty)` and disables the trash button once cleared.
- **Silent Folder Merge**:
  - If the target folder already exists in the parent directory, selected files are moved into it directly without any redundant warning dialog.
- **File Collision Handling & Auto-Rename Algorithm**:
  - Pre-flight check scans destination folder for name collisions before initiating file operations.
  - If matching filenames exist, displays a localized conflict dialog offering **Auto-rename** (`Automatisch umbenennen` / `Автоматически переименовать`) or **Overwrite** (`Überschreiben` / `Перезаписать`).
  - Auto-rename algorithm appends incremental numbers (e.g. `file (2).jpg`, `file (3).jpg`) while strictly preserving extensions and handling batch collisions.
- **Automated Unit Tests**:
  - Added unit tests for file extension parsing and sequential auto-rename logic (17/17 tests passing).

## [0.1.0] - 2026-09-30

### Added
- **Core Functionality**:
  - Context menu command `Move to new folder...` in classic Windows Explorer for files, directories, and mixed selections.
  - Native Win32 modal dialog displaying item count, folder name edit box, and Move/Cancel buttons.
  - Keyboard accelerators: `Enter` triggers Move, `Escape` triggers Cancel, `Ctrl+A` selects all text.
- **Architecture & Reliability**:
  - Out-of-process COM Local Server architecture (`DelegateExecute` / `IExecuteCommand` / `IObjectWithSelection`) preventing any Explorer instability or process locks.
  - Multi-selection support using `IShellItemArray` with `MultiSelectModel="Document"`, scaling smoothly to 1,000+ files without command-line buffer limits.
  - File operations powered by `IFileOperation` with native progress, collision UI ("Replace / Skip / Keep both"), UAC elevation, and Windows Explorer Undo (`Ctrl+Z`).
- **Data Safety & Path Validation**:
  - Strict folder name validation disallowing invalid characters (`\/:*?"<>|`), control characters, trailing dots/spaces, relative traversal (`.`, `..`), and reserved DOS device names (`CON`, `PRN`, `AUX`, `NUL`, `COM1-9`, `LPT1-9`).
  - Common parent directory verification across selected items to protect against Search Results / Library view mismatches.
  - Recursive self-move prevention (moving a folder inside itself or its children).
  - Existing destination collision prompt ("Use existing folder", "Choose another name", "Cancel").
  - Shortcut preservation: moving `.lnk` or `.url` moves the shortcut file itself, never following target executables.
  - Automatic rollback of newly created empty directories if an operation fails or is canceled.
  - Long path support (`\\?\` prefix and manifest `longPathAware`) handling paths exceeding 260 characters.
  - Strict invariant: Current Working Directory is never used as a fallback destination.
- **UI & Theming**:
  - Per-Monitor v2 DPI awareness (`WM_DPICHANGED` scaling for 100%, 125%, 150%, 200%, 250%, 4K).
  - Automatic Windows Dark Mode and High Contrast theme support.
  - Centering relative to the owning Explorer window or active monitor.
  - Built-in multi-language localization for English, German (Deutsch), and Russian (Русский).
- **Packaging & Deployment**:
  - Inno Setup 6 installer supporting per-user silent installation without admin privileges (`/VERYSILENT`).
  - Clean uninstallation removing all binaries, COM classes, and registry entries.
  - Standalone single-file binary with static runtime linking (zero external DLL dependencies).
  - Automated build script (`scripts/build.ps1`) and Visual Studio 2022 solution (`QuickFolder.sln`).
  - Automated unit test suite with 15 test cases covering all edge cases.

### Fixed
- **UI High-DPI Scaling & Control Clipping**: Fixed issue where the dialog window height was hardcoded to 195px unscaled while child controls scaled with DPI, causing the Edit control and buttons to be clipped/hidden on Windows 11 high-DPI displays (125%, 150%, 200%). Now calculates required client area (440x170 base), scales dimensions using `AdjustWindowRectExForDpi`, applies per-DPI Segoe UI fonts, and auto-focuses the edit control with text selected.

