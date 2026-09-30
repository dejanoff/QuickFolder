# QuickFolder

**QuickFolder** adds a simple command to Windows Explorer:

> **Select files → Right-click → Move to new folder... → Type a name → Done.**

A lightweight, high-performance native Windows utility written in C++20 (Win32 / Unicode / x64) that helps you quickly organize files and folders without bloat, scripts, or runtime dependencies.

---

## Features

- **Blazing Fast**: Native C++20 application with instant startup and zero background services.
- **Explorer Integrated**: Seamless classic Windows context menu entry ("Move to new folder...").
- **Scalable**: Handles 1, 20, 500, or 10,000+ files via standard Windows Shell COM interfaces (`IShellItemArray`), bypassing command-line length limits.
- **Safe File Operations**: Utilizes Windows `IFileOperation` for native progress dialogs, conflict resolution ("Replace / Skip / Keep both"), UAC elevation, and **Undo (`Ctrl+Z`) support**.
- **Smart Defaults**:
  - Selecting a single file proposes the filename without extension (e.g. `IMG_1234.jpg` → `IMG_1234`).
  - Selecting a directory proposes `<Name> - Folder` to avoid collision with itself.
- **Strict Data Protection**:
  - Automatically verifies common parent directories (protects against Search Results / Library view mismatches).
  - Prevents recursive moves (moving a folder inside itself).
  - Checks if destination exists and prompts for action ("Use existing folder", "Choose another name", "Cancel").
  - Cleans up newly created empty folders if an operation is canceled or fails.
- **Shortcuts Handled Correctly**: Moving `.lnk` or `.url` shortcuts moves the shortcut file itself, never following or moving the target executable.
- **Long Path Support**: Fully supports Windows paths exceeding 260 characters (`\\?\` extended-length paths).
- **Modern Display & Theming**: Per-Monitor v2 DPI awareness (sharp on 4K and multi-monitor setups) and automatic Windows Dark Mode support.
- **Multi-Language**: Built-in localized UI for **English**, **German** (Deutsch), and **Russian** (Русский).
- **100% Private**: Works completely offline. No telemetry, no network access, no accounts.

---

## Installation

### Standard Setup (Recommended)
Download the latest `QuickFolder-Setup-0.1.0.exe` from [GitHub Releases](https://github.com/QuickFolder/QuickFolder/releases).

Run the installer:
- Installs per-user to `%LOCALAPPDATA%\Programs\QuickFolder` by default (no Administrator privileges required).
- Automatically registers the Explorer context menu and COM Local Server.

### Silent Installation (For Admins & Deployment)
```cmd
QuickFolder-Setup-0.1.0.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART
```

---

## Usage

1. Open **Windows Explorer** and navigate to any folder (e.g. `D:\Photos`).
2. Select one or more files and/or folders.
3. Right-click your selection → choose **Move to new folder...** (on Windows 11, choose "Show more options" or press `Shift+F10`).
4. Type the new folder name in the dialog and press `Enter` (or click **Move**).
5. QuickFolder creates the folder and moves all selected items into it.

### Command-Line Usage (for Power Users and Developers)
```cmd
# Register context menu for current user
QuickFolder.exe --register

# Unregister context menu cleanly
QuickFolder.exe --unregister

# Silent registration / unregistration
QuickFolder.exe --register --silent
QuickFolder.exe --unregister --silent

# Preview the input dialog without selecting files
QuickFolder.exe --test-dialog

# Move specific files directly via CLI
QuickFolder.exe --folder-name "Holiday" "D:\Photos\a.jpg" "D:\Photos\b.jpg"

# Move batch files from a text list without command-line limits
QuickFolder.exe --folder-name "Batch" @filelist.txt

# Show version
QuickFolder.exe --version
```

---

## Supported Windows Versions

- **Windows 11 x64** (Primary target)
- **Windows 10 x64** (Fully supported)
- Architecture: 64-bit (`x64`)

---

## Privacy Policy

QuickFolder works entirely locally and does not collect or transmit user data.
- No telemetry
- No analytics
- No accounts
- No remote network requests
- No file scanning or indexing

---

## Building from Source

### Prerequisites
- **Visual Studio 2022** (v143 C++ tools, Windows SDK) OR **LLVM-MinGW** (Clang++ with UCRT).
- **Inno Setup 6** (optional, for building the installer).
- **PowerShell 7** or Windows PowerShell.

### One-Command Build:
```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
```

The script will:
1. Clean output directories.
2. Compile resources and icons.
3. Build and execute all 15 automated unit tests (`UnitTests.exe`).
4. Build `dist/QuickFolder.exe` (statically linked, zero external runtime dependencies).
5. Build `dist/QuickFolder-Setup-0.1.0.exe` installer.

---

## Known Limitations

- **Search Results with Multiple Folders**: If files selected in a Search Results or Library view reside in different physical folders, QuickFolder aborts the move to prevent unintended destination placement.
- **Windows 11 Compact Menu**: v1 integrates with the classic Windows context menu (Shift+Right Click or classic shell). Integration into the top-level compact Windows 11 menu via Sparse Package is scheduled for v1.1.0.

---

## Roadmap

- [ ] **v1.1.0**: Windows 11 modern context menu integration via Sparse Package (`IExplorerCommand`).
- [ ] **v1.2.0**: Optional duplicate auto-numbering (`Folder (2)`).
- [ ] **v1.3.0**: Move each file into individual subfolders based on filename.
- [ ] **v2.0.0**: Microsoft Store publication (MSIX package).

---

## Uninstall

You can uninstall QuickFolder at any time:
1. Open **Windows Settings → Apps → Installed apps**.
2. Find **QuickFolder** and click **Uninstall**.

Or run the uninstaller directly:
```cmd
"%LOCALAPPDATA%\Programs\QuickFolder\unins000.exe" /VERYSILENT /SUPPRESSMSGBOXES
```
Uninstallation completely removes all QuickFolder executables, COM registrations, and context menu registry entries.

---

## License

This project is licensed under the [MIT License](LICENSE).
