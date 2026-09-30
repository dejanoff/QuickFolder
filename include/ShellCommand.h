#pragma once

#include "Common.h"

namespace QuickFolder::Shell {

// COM Execution handler for Windows Explorer context menu
class CQuickFolderCommand final : public IExecuteCommand, public IObjectWithSelection, public IObjectWithSite {
public:
    CQuickFolderCommand();
    ~CQuickFolderCommand();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;

    // IObjectWithSelection
    IFACEMETHODIMP SetSelection(IShellItemArray* psiItemArray) override;
    IFACEMETHODIMP GetSelection(REFIID riid, void** ppv) override;

    // IExecuteCommand
    IFACEMETHODIMP SetKeyState(DWORD grfKeyState) override;
    IFACEMETHODIMP SetParameters(LPCWSTR pszParameters) override;
    IFACEMETHODIMP SetPosition(POINT pt) override;
    IFACEMETHODIMP SetShowWindow(int nShow) override;
    IFACEMETHODIMP SetNoShowUI(BOOL fNoShowUI) override;
    IFACEMETHODIMP SetDirectory(LPCWSTR pszDirectory) override;
    IFACEMETHODIMP Execute() override;

    // IObjectWithSite
    IFACEMETHODIMP SetSite(IUnknown* pUnkSite) override;
    IFACEMETHODIMP GetSite(REFIID riid, void** ppv) override;

private:
    long m_refCount = 1;
    HWND m_hwndOwner = NULL;
    IShellItemArray* m_pSelection = nullptr;
    IUnknown* m_pSite = nullptr;
    DWORD m_keyState = 0;
};

// COM Class Factory
class CQuickFolderClassFactory final : public IClassFactory {
public:
    CQuickFolderClassFactory();
    ~CQuickFolderClassFactory();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
    IFACEMETHODIMP LockServer(BOOL fLock) override;

private:
    long m_refCount = 1;
};

// Registry registration functions
HRESULT RegisterServer(bool perUser);
HRESULT UnregisterServer(bool perUser);

} // namespace QuickFolder::Shell
