#include "stdafx.h"
#include "DemoMFC.h"
#include "MainFrm.h"
#include "ControlPanel.h"

#include <fstream>

CDemoMFCApp theApp;

BEGIN_MESSAGE_MAP(CDemoMFCApp, CWinApp)
    ON_COMMAND(IDM_HELP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()

CDemoMFCApp::CDemoMFCApp()
    : m_engine(nullptr)
    , m_assetsPath(_T(""))
    , m_wallpaperPath(_T("")) {
}

BOOL CDemoMFCApp::InitInstance() {
    CWinApp::InitInstance();

    // Initialize OLE libraries
    if (!AfxOleInit()) {
        AfxMessageBox(_T("OLE initialization failed"));
        return FALSE;
    }

    // Enable DDE Execute open
    EnableShellOpen();
    RegisterShellFileTypes(TRUE);

    // Create main frame window
    CMainFrame* pFrame = new CMainFrame;
    if (!pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, nullptr, nullptr)) {
        delete pFrame;
        return FALSE;
    }
    m_pMainWnd = pFrame;
    pFrame->ShowWindow(SW_SHOW);
    pFrame->UpdateWindow();

    // Create the wallpaper engine instance
    m_engine = WE_Create();
    if (!m_engine) {
        AfxMessageBox(_T("Failed to create wallpaper engine!"));
        return FALSE;
    }

    // Load configuration from ini file
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    CString strPath(szPath);
    int pos = strPath.ReverseFind(_T('\\'));
    if (pos != -1) {
        strPath = strPath.Left(pos + 1);
    }
    strPath += _T("demo_mfc.ini");

    // Try to load assets path from ini
    TCHAR assetsPath[MAX_PATH];
    if (GetPrivateProfileString(_T("Config"), _T("AssetsPath"), _T(""), assetsPath, MAX_PATH, strPath)) {
        m_assetsPath = assetsPath;
        if (!m_assetsPath.IsEmpty()) {
            WE_SetAssetsPath(m_engine, CT2A(m_assetsPath));
        }
    }

    // Try to load wallpaper path from ini
    TCHAR wallpaperPath[MAX_PATH];
    if (GetPrivateProfileString(_T("Config"), _T("WallpaperPath"), _T(""), wallpaperPath, MAX_PATH, strPath)) {
        m_wallpaperPath = wallpaperPath;
    }

    return TRUE;
}

int CDemoMFCApp::ExitInstance() {
    // Save configuration to ini file
    if (m_engine) {
        WE_Stop(m_engine);

        TCHAR szPath[MAX_PATH];
        GetModuleFileName(nullptr, szPath, MAX_PATH);
        CString strPath(szPath);
        int pos = strPath.ReverseFind(_T('\\'));
        if (pos != -1) {
            strPath = strPath.Left(pos + 1);
        }
        strPath += _T("demo_mfc.ini");

        if (!m_assetsPath.IsEmpty()) {
            WritePrivateProfileString(_T("Config"), _T("AssetsPath"), m_assetsPath, strPath);
        }
        if (!m_wallpaperPath.IsEmpty()) {
            WritePrivateProfileString(_T("Config"), _T("WallpaperPath"), m_wallpaperPath, strPath);
        }

        WE_Destroy(m_engine);
        m_engine = nullptr;
    }

    return CWinApp::ExitInstance();
}

void CDemoMFCApp::OnAppAbout() {
    CDialog aboutDlg(IDD_ABOUTBOX);
    aboutDlg.DoModal();
}
