# Microsoft Store Distribution Strategy and Roadmap

## Overview
While QuickFolder v1 targets standalone distribution via GitHub Releases and an Inno Setup installer, its architecture has been purposefully designed to allow seamless future publication to the **Microsoft Store** without rewriting business logic.

---

## 1. Store Distribution Models for Win32 Apps

Microsoft provides two primary models for publishing Win32 (desktop) applications in the Microsoft Store:

### Option A: Standard Win32 Installer Distribution (Direct EXE/Inno Setup)
- Microsoft Store allows developers to publish standard Win32 setup packages (such as `QuickFolder-Setup.exe` built with Inno Setup).
- **Pros**: Zero modification to binaries; identical build artifacts used for GitHub Releases and Store; supports current COM Local Server architecture directly.
- **Cons**: App updates are managed either by the app itself or via URL redirect, without atomic OS-managed updates.

### Option B: MSIX Packaging with Package Identity
- QuickFolder is packaged as an `.msix` / `.msixbundle` container.
- **Pros**:
  - Full Microsoft Store integration with automatic, differential background updates.
  - Integration with the Windows 11 Modern (Compact) Context Menu via `<desktop4:FileExplorerContextMenus>` in `AppxManifest.xml`.
  - Clean containerized install and uninstall managed directly by Windows.
- **Cons**: Requires code signing certificate (or Store ingestion signing) and manifest declarations for COM Local Server.

---

## 2. Windows 11 Modern Context Menu via Sparse Package

To show a context menu entry in the top-level Windows 11 context menu without migrating the entire app to a full MSIX sandbox, Microsoft supports **Sparse Packages** (Package Identity for unpackaged desktop apps):

1. **Package Identity**:
   A lightweight `AppxManifest.xml` registers an identity (e.g. `QuickFolder.App`) and points to an `IExplorerCommand` implementation.
2. **Registration**:
   Registered at install time via the WinRT `AddPackageByUriAsync` API or `powershell Add-AppxPackage -Register AppxManifest.xml`.
3. **Execution**:
   The `IExplorerCommand::Invoke()` method delegates selection directly to the QuickFolder COM Local Server executable or executes the command out-of-process.

---

## 3. Code Signing & Store Validation Requirements

For Store submission:
1. **Developer Account**: Registered Microsoft Partner Center developer account ($19 one-time fee).
2. **Certificates**:
   - For standalone MSIX side-loading: A trusted code signing certificate (e.g., Sectigo, DigiCert, or Azure Trusted Signing).
   - For Microsoft Store: Microsoft automatically signs the package during Store ingestion.
3. **Privacy Compliance**:
   QuickFolder strictly complies with Store policies: zero telemetry, zero background network connections, zero advertising, and all file operations are user-initiated and local.

---

## 4. Migration Roadmap
- **v1.0.0 (Current)**: Native Win32 x64, Inno Setup installer, Classic Context Menu integration via COM Local Server (`DelegateExecute`).
- **v1.1.0**: Add Sparse Package manifest (`AppxManifest.xml`) and modern Windows 11 context menu handler.
- **v1.2.0**: Publish to Microsoft Store using the Win32 / Inno Setup store submission pipeline.
- **v2.0.0**: Optional full MSIX package build in GitHub Actions workflow for pure Store deployment.
