# QuickFolder Automated Build & Packaging Script
[CmdletBinding()]
param (
    [switch]$SkipTests = $false,
    [switch]$SkipInstaller = $false
)

$ErrorActionPreference = "Stop"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host " QuickFolder Build System (x64 Release)" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

$rootDir = Split-Path -Parent $PSScriptRoot
Set-Location $rootDir

# 1. Locate Compilers and Tools
$foundClang = Get-Command "clang++" -ErrorAction SilentlyContinue
if ($foundClang) {
    $clangPath = $foundClang.Source
} else {
    $clangPath = "C:\Users\Dejan\AppData\Local\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260616-ucrt-x86_64\bin\clang++.exe"
}

$foundWindres = Get-Command "windres" -ErrorAction SilentlyContinue
if ($foundWindres) {
    $windresPath = $foundWindres.Source
} else {
    $windresPath = "C:\Users\Dejan\AppData\Local\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260616-ucrt-x86_64\bin\windres.exe"
}

$foundIscc = Get-Command "iscc" -ErrorAction SilentlyContinue
if ($foundIscc) {
    $isccPath = $foundIscc.Source
} else {
    $isccPath = "C:\Users\Dejan\AppData\Local\Programs\Inno Setup 6\ISCC.exe"
}

Write-Host "C++ Compiler: $clangPath"
Write-Host "Resource Compiler: $windresPath"
Write-Host "Installer Compiler: $isccPath"

# 2. Clean directories
Write-Host "`n[1/5] Cleaning output directories..." -ForegroundColor Yellow
if (Test-Path "build") { Remove-Item -Recurse -Force "build" }
if (Test-Path "dist") { Remove-Item -Recurse -Force "dist" }
New-Item -ItemType Directory -Force -Path "build" | Out-Null
New-Item -ItemType Directory -Force -Path "dist" | Out-Null

# 3. Generate icon if missing
if (-not (Test-Path "res\QuickFolder.ico")) {
    Write-Host "Generating icon res/QuickFolder.ico..."
    python scripts\generate_icon.py
}

# 4. Compile Resource file
Write-Host "`n[2/5] Compiling resources..." -ForegroundColor Yellow
& $windresPath -Iinclude -Ires -i res\QuickFolder.rc -O coff -o build\QuickFolder.res
if ($LASTEXITCODE -ne 0) {
    Write-Error "windres failed with exit code $LASTEXITCODE"
}

# 5. Build and Run Unit Tests
if (-not $SkipTests) {
    Write-Host "`n[3/5] Building and running automated unit tests..." -ForegroundColor Yellow
    & $clangPath -std=c++20 -static -O2 -Iinclude -Ires `
        tests\UnitTests.cpp src\PathUtils.cpp src\SelectionHelper.cpp `
        -lole32 -loleaut32 -luuid -lshell32 -lshlwapi -lcomctl32 -luxtheme -ldwmapi `
        -o build\UnitTests.exe
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Unit test compilation failed with exit code $LASTEXITCODE"
    }

    & "build\UnitTests.exe"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Unit tests failed!"
    }
} else {
    Write-Host "`n[3/5] Skipping unit tests." -ForegroundColor Gray
}

# 6. Build QuickFolder.exe
Write-Host "`n[4/5] Building QuickFolder.exe (x64 Release)..." -ForegroundColor Yellow
& $clangPath -std=c++20 -static -O2 -mwindows -municode -Iinclude -Ires `
    src\Main.cpp src\PathUtils.cpp src\SelectionHelper.cpp `
    src\FileOperations.cpp src\UI.cpp src\ShellCommand.cpp `
    build\QuickFolder.res `
    -lole32 -loleaut32 -luuid -lshell32 -lshlwapi -lcomctl32 -luxtheme -ldwmapi `
    -o dist\QuickFolder.exe
if ($LASTEXITCODE -ne 0) {
    Write-Error "QuickFolder.exe compilation failed with exit code $LASTEXITCODE"
}

$exeItem = Get-Item "dist\QuickFolder.exe"
Write-Host "QuickFolder.exe built successfully: $($exeItem.FullName) ($($exeItem.Length) bytes)" -ForegroundColor Green

# 7. Build Inno Setup Installer
if (-not $SkipInstaller -and (Test-Path $isccPath)) {
    Write-Host "`n[5/5] Building Inno Setup Installer..." -ForegroundColor Yellow
    & $isccPath /Q installer\QuickFolder.iss
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Inno Setup compilation failed with exit code $LASTEXITCODE"
    }
    $setupItem = Get-Item "dist\QuickFolder-Setup-*.exe" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    Write-Host "Installer built successfully: $($setupItem.FullName) ($($setupItem.Length) bytes)" -ForegroundColor Green
} else {
    Write-Host "`n[5/5] Skipping installer compilation." -ForegroundColor Gray
}

Write-Host "`n=========================================" -ForegroundColor Cyan
Write-Host " BUILD COMPLETE! Artifacts in dist/:" -ForegroundColor Cyan
Get-ChildItem "dist" | Select-Object Name, Length, LastWriteTime | Format-Table -AutoSize
Write-Host "=========================================" -ForegroundColor Cyan
