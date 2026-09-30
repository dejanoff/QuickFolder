# Changelog

All notable changes to **QuickFolder** will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

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
