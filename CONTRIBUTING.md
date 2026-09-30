# Contributing to QuickFolder

Thank you for your interest in contributing to QuickFolder!

## Development Guidelines
1. **Language & Standards**: QuickFolder is written in standard **C++20** using native Win32/Unicode APIs. Avoid non-portable external dependencies or heavy frameworks.
2. **Safety First**: Never perform silent overwrites. Validate all filenames before initiating filesystem modifications.
3. **Out-of-Process Integrity**: Maintain the COM Local Server out-of-process isolation to prevent any Explorer shell instability.
4. **Testing**: Any modification to path logic or validation must be accompanied by corresponding unit tests in `tests/UnitTests.cpp`.

## Pull Request Process
1. Fork the repository and create your feature branch.
2. Ensure `scripts\build.ps1` runs cleanly and all automated unit tests pass.
3. Update `CHANGELOG.md` and documentation if applicable.
4. Open a pull request describing the changes and problem solved.
