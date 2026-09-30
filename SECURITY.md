# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 0.1.x   | :white_check_mark: |

## Security Invariants of QuickFolder
QuickFolder is designed with strict security and data safety invariants:
1. **Never Move Unintended Objects**: Shortcut files (`.lnk` and `.url`) are moved as filesystem objects themselves; targets are never followed or resolved.
2. **Never Execute Shell Injection**: QuickFolder uses direct Win32/COM APIs (`IFileOperation`, `CreateDirectoryW`) and never passes strings to `cmd.exe` or `powershell.exe`.
3. **No Network Transmission**: QuickFolder has no network, telemetry, or remote logging capabilities.
4. **Explicit Absolute Paths**: QuickFolder never falls back to the Current Working Directory or application directory for destination resolution.

## Reporting a Vulnerability
If you discover a security vulnerability or critical bug involving potential data loss, please report it privately via GitHub Security Advisories or by contacting the maintainers directly.
