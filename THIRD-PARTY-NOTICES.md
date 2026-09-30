# Third-Party Notices

QuickFolder is an original open-source project written from scratch in native C++20 and Win32.
No third-party libraries or proprietary code are statically incorporated into the QuickFolder codebase.

The build environment and installer utilize the following external open-source tools:

---

### 1. Inno Setup
- **Project**: Inno Setup
- **Author**: Jordan Russell / Martijn Laan
- **URL**: https://jrsoftware.org/isinfo.php
- **License**: Modified BSD / Inno Setup License
- **Usage**: Used to compile the installation setup package (`QuickFolder-Setup-0.1.0.exe`).

---

### 2. LLVM-MinGW
- **Project**: LLVM-MinGW Toolchain
- **Author**: Martin Storsjö
- **URL**: https://github.com/mstorsjo/llvm-mingw
- **License**: ISC License / Apache 2.0 with LLVM Exception
- **Usage**: Used as the x64 C++20 compiler and toolchain for building native binaries.
