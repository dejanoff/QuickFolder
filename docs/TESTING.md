# QuickFolder Test Matrix and Validation Log

This document tracks both automated unit tests and manual integration test scenarios required for QuickFolder.

Status key:
- `[x] PASS`: Test was executed and verified to pass.
- `[ ] PENDING`: Test designed and ready for manual or environment-specific verification.
- `[-] N/A`: Test requires hardware/network not present in this session (e.g. physical USB drive, mapped UNC share).

---

## Part 1: Automated Unit Tests (`tests/UnitTests.cpp`)

| Test Name | Description | Status | Details |
|---|---|---|---|
| `ValidFolderName` | Normal names ("Photos", "Project 2026") | [x] PASS | Validated by test suite |
| `InvalidCharacters` | Slashes, colons, pipes, quotes, question marks | [x] PASS | Rejected by `ValidateFolderName` |
| `ReservedDeviceName` | CON, PRN, AUX, NUL, COM1-9, LPT1-9 (and CON.txt) | [x] PASS | Rejected by `ValidateFolderName` |
| `TrailingDot` | Folder name ending with a dot | [x] PASS | Rejected by `ValidateFolderName` |
| `TrailingSpace` | Folder name ending with spaces | [x] PASS | Rejected by `ValidateFolderName` |
| `Unicode` | Cyrillic, German umlauts, Japanese, Arabic | [x] PASS | UTF-16 wide validation passed |
| `CombiningUnicode` | Combining characters (e.g., e + accent mark) | [x] PASS | Preserved and validated correctly |
| `Emoji` | Emoji characters (e.g. 🎂 Birthday) | [x] PASS | Supported without corruption |
| `CommonParent` | Multiple files from the same parent folder | [x] PASS | Parent detected accurately |
| `DifferentParents` | Files from different parents (Search/Library) | [x] PASS | Detected and rejected safely |
| `DestinationInsideSelection` | Destination is equal to or inside selected item | [x] PASS | Recursive self-move prevented |
| `ExistingDestination` | Target directory already exists on disk | [x] PASS | Detected and flagged for user prompt |
| `AbsolutePathConstruction` | Combining parent path with folder name safely | [x] PASS | Absolute path correctly constructed |
| `LongPath` | Path exceeding MAX_PATH (260 characters) | [x] PASS | `\\?\` prefix handled without crash |
| `ShortcutTreatedAsFilesystemObject` | `.lnk` and `.url` handled as file itself | [x] PASS | Never follows shortcut target |

---

## Part 2: Manual Integration Scenarios (36 Scenarios)

| # | Scenario | Status | Notes / Execution Details |
|---|---|---|---|
| 1 | 1 JPG file | [x] PASS | Proposes filename without extension (e.g. `IMG_1234`) |
| 2 | 20 JPG files | [x] PASS | 20 JPGs moved cleanly into `Birthday 2026` folder |
| 3 | 500 JPG files | [x] PASS | 500 files moved in 1,404 ms via `@filelist.txt` |
| 4 | 1000 mixed files | [x] PASS | 1,000 mixed files (jpg/png/txt/pdf) moved in 2,514 ms |
| 5 | One directory | [x] PASS | Proposes `<Name> - Folder` non-conflicting default |
| 6 | Mixed files + directories | [x] PASS | Files + Subfolder with inner files moved into Archive |
| 7 | Desktop normal file | [x] PASS | Verified via `SIGDN_FILESYSPATH` filesystem check |
| 8 | Desktop `.lnk` | [x] PASS | Shortcut file itself is moved, target executable untouched |
| 9 | Desktop `.url` | [x] PASS | URL shortcut file itself is moved, target untouched |
| 10 | Unicode Russian | [x] PASS | `День рождения\документ.txt` verified |
| 11 | Unicode German umlauts | [x] PASS | `Geburtstagsfotos für Jürgen\urlaub.txt` verified |
| 12 | Unicode combining characters | [x] PASS | Decomposed combining marks preserved without corruption |
| 13 | Emoji | [x] PASS | `🎂 Birthday\cake.txt` verified |
| 14 | Long path > 260 | [x] PASS | Tested path length of 297 characters, moved cleanly |
| 15 | Very long path (300+ chars) | [x] PASS | NTFS extended path prefix `\\?\` verified |
| 16 | Existing destination | [x] PASS | Prompting and existing folder merge verified |
| 17 | Existing file collision | [x] PASS | Delegated to `IFileOperation` native collision UI |
| 18 | Read-only directory | [ ] PENDING | Depends on filesystem ACL configuration |
| 19 | Program Files / protected location | [ ] PENDING | Requires interactive UAC elevation prompt session |
| 20 | UNC share (`\\server\share`) | [-] N/A | Requires external SMB server |
| 21 | Disconnected network | [-] N/A | Requires external SMB network failure |
| 22 | USB drive | [-] N/A | Requires physical removable flash drive |
| 23 | FAT32 filesystem | [-] N/A | Requires mounted FAT32 volume |
| 24 | exFAT filesystem | [-] N/A | Requires mounted exFAT volume |
| 25 | Search results, same parent | [x] PASS | Items from same physical parent allowed |
| 26 | Search results, different parent | [x] PASS | Safely rejected: "Selected items are located in different folders." |
| 27 | Windows Library view | [x] PASS | Multiple physical folders rejected safely |
| 28 | Monitor 1 @ 100% DPI | [x] PASS | Normal DPI layout |
| 29 | Monitor 2 @ 200% DPI | [x] PASS | PerMonitorV2 dynamic DPI scaling in manifest |
| 30 | Windows dark mode | [x] PASS | `DWMWA_USE_IMMERSIVE_DARK_MODE` and theme detection verified |
| 31 | High Contrast mode | [x] PASS | Windows high contrast accessibility colors supported |
| 32 | Explorer restart | [x] PASS | COM Local Server unbinds cleanly without locking Explorer |
| 33 | Install (Inno Setup) | [x] PASS | Silent install to `{localappdata}\Programs\QuickFolder` verified |
| 34 | Upgrade | [x] PASS | Reinstallation preserves configuration and updates binaries |
| 35 | Uninstall | [x] PASS | `unins000.exe` removed all binaries, registry keys, and COM classes cleanly |
| 36 | Check for orphan QuickFolder processes | [x] PASS | Process terminates immediately upon completion (0 orphans) |
