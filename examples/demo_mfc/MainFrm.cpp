#include "stdafx.h"
#include "DemoMFC.h"
#include "MainFrm.h"
#include <commdlg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_COMMAND(IDM_FILE_LOAD_WALLPAPER, OnFileLoadWallpaper)
    ON_COMMAND(IDM_FILE_SET_ASSETS, OnFileSetAssets)
    ON_COMMAND(IDM_FILE_EXIT, OnFileExit)
    ON_COMMAND(IDM_PLAYBACK_PLAY, OnPlaybackPlay)
    ON_COMMAND(IDM_PLAYBACK_PAUSE, OnPlaybackPause)
    ON_COMMAND(IDM_PLAYBACK_STOP, OnPlaybackStop)
    ON_COMMAND(IDM_VIEW_CONTROLPANEL, OnViewControlPanel)
    ON_UPDATE_COMMAND_UI(IDM_PLAYBACK_PLAY, OnUpdatePlaybackPlay)
    ON_UPDATE_COMMAND_UI(IDM_PLAYBACK_PAUSE, OnUpdatePlaybackPause)
    ON_UPDATE_COMMAND_UI(IDM_PLAYBACK_STOP, OnUpdatePlaybackStop)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

static UINT indicators[] = {
    ID_SEPARATOR,
    ID_STATUS_BAR_PANE
};

CMainFrame::CMainFrame()
    : m_hWallpaperWnd(nullptr) {
}

CMainFrame::~CMainFrame() {
    if (m_hWallpaperWnd && IsWindow(m_hWallpaperWnd)) {
        DestroyWindow(m_hWallpaperWnd);
    }
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) {
    if (!CFrameWnd::PreCreateWindow(cs)) {
        return FALSE;
    }
    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.lpszClass = AfxRegisterWndClass(0);
    return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    // Create status bar
    if (!m_wndStatusBar.Create(this)) {
        TRACE0("Failed to create status bar\n");
        return -1;
    }
    m_wndStatusBar.SetIndicators(indicators, 1);

    // Create control panel (initially docked to the right)
    if (!m_wndControlPanel.Create(this, IDD_CONTROL_PANEL,
        CBRS_RIGHT | CBRS_FLOAT_MULTI, IDD_CONTROL_PANEL)) {
        TRACE0("Failed to create control panel\n");
        return -1;
    }
    m_wndControlPanel.SetBarStyle(m_wndControlPanel.GetBarStyle() |
        CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
    m_wndControlPanel.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndControlPanel);

    // Show the control panel initially
    ShowControlBar(&m_wndControlPanel, TRUE, FALSE);

    // Get client rect for wallpaper window
    CRect rect;
    GetClientRect(&rect);
    if (m_wndControlPanel.IsWindowVisible()) {
        CRect controlRect;
        m_wndControlPanel.GetWindowRect(&controlRect);
        rect.right -= controlRect.Width();
    }

    // Create wallpaper window
    if (!CreateWallpaperWindow(rect)) {
        TRACE0("Failed to create wallpaper window\n");
    }

    return 0;
}

BOOL CMainFrame::CreateWallpaperWindow(const CRect& rect) {
    if (m_hWallpaperWnd && IsWindow(m_hWallpaperWnd)) {
        DestroyWindow(m_hWallpaperWnd);
        m_hWallpaperWnd = nullptr;
    }

    // Register window class
    static LPCTSTR pszClassName = nullptr;
    if (!pszClassName) {
        WNDCLASS wc = {0};
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = ::DefWindowProc;
        wc.hInstance = AfxGetInstanceHandle();
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.lpszClassName = _T("WallpaperRenderWnd");
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

        if (AfxRegisterClass(&wc)) {
            pszClassName = wc.lpszClassName;
        } else {
            return FALSE;
        }
    }

    // Create the window
    m_hWallpaperWnd = ::CreateWindowEx(
        0,
        pszClassName,
        _T(""),
        WS_CHILD | WS_VISIBLE,
        rect.left, rect.top,
        rect.Width(), rect.Height(),
        m_hWnd,
        (HMENU)IDC_WALLPAPER_WINDOW,
        AfxGetInstanceHandle(),
        nullptr
    );

    if (m_hWallpaperWnd) {
        // Inject the window handle to the engine
        if (theApp.m_engine) {
            WE_SetWindowHandle(theApp.m_engine, m_hWallpaperWnd);
            WE_ResizeWindow(theApp.m_engine, 0, 0, rect.Width(), rect.Height());
        }
        return TRUE;
    }

    return FALSE;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) {
    CFrameWnd::OnSize(nType, cx, cy);

    // Resize wallpaper window
    if (m_hWallpaperWnd && IsWindow(m_hWallpaperWnd)) {
        CRect rect(0, 0, cx, cy);

        // Account for control panel if visible
        if (m_wndControlPanel.IsWindowVisible()) {
            CRect controlRect;
            m_wndControlPanel.GetWindowRect(&controlRect);
            ScreenToClient(&controlRect);
            rect.right = controlRect.left;
        }

        ::SetWindowPos(m_hWallpaperWnd, nullptr,
            rect.left, rect.top,
            rect.Width(), rect.Height(),
            SWP_NOZORDER);

        // Update engine with new size
        if (theApp.m_engine) {
            WE_ResizeWindow(theApp.m_engine, 0, 0, rect.Width(), rect.Height());
        }
    }
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) {
    return TRUE;  // Don't erase background to avoid flicker
}

void CMainFrame::OnFileLoadWallpaper() {
    CFileDialog dlg(TRUE, nullptr, nullptr,
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
        _T("Wallpaper Folders|*.workshop|All Files (*.*)|*.*||"),
        this);

    if (dlg.DoModal() == IDOK) {
        LoadWallpaper(dlg.GetPathName());
    }
}

void CMainFrame::OnFileSetAssets() {
    CFileDialog dlg(TRUE, nullptr, nullptr,
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
        _T("All Files (*.*)|*.*||"),
        this);

    dlg.m_ofn.lpstrTitle = _T("Select Wallpaper Engine Assets Folder");
    dlg.m_ofn.Flags |= OFN_ENABLESIZING;

    BROWSEINFO bi = {0};
    bi.hwndOwner = m_hWnd;
    bi.pszDisplayName = nullptr;
    bi.lpszTitle = _T("Select Wallpaper Engine Assets Folder");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl) {
        TCHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
            theApp.m_assetsPath = path;
            if (theApp.m_engine) {
                if (WE_SetAssetsPath(theApp.m_engine, CT2A(path))) {
                    m_wndStatusBar.SetWindowText(_T("Assets path set: ") + path);
                } else {
                    AfxMessageBox(_T("Failed to set assets path: ") +
                        CString(WE_GetLastError(theApp.m_engine)));
                }
            }
        }
        CoTaskMemFree(pidl);
    }
}

void CMainFrame::OnFileExit() {
    PostMessage(WM_CLOSE);
}

void CMainFrame::OnPlaybackPlay() {
    if (theApp.m_engine) {
        if (WE_Play(theApp.m_engine)) {
            m_wndStatusBar.SetWindowText(_T("Playing..."));
            if (m_hWallpaperWnd && IsWindow(m_hWallpaperWnd)) {
                ::ShowWindow(m_hWallpaperWnd, SW_SHOW);
            }
        } else {
            AfxMessageBox(_T("Failed to play: ") +
                CString(WE_GetLastError(theApp.m_engine)));
        }
    }
}

void CMainFrame::OnPlaybackPause() {
    if (theApp.m_engine) {
        if (WE_Pause(theApp.m_engine)) {
            m_wndStatusBar.SetWindowText(_T("Paused"));
        }
    }
}

void CMainFrame::OnPlaybackStop() {
    if (theApp.m_engine) {
        if (WE_Stop(theApp.m_engine)) {
            m_wndStatusBar.SetWindowText(_T("Stopped"));
        }
    }
}

void CMainFrame::OnViewControlPanel() {
    ShowControlBar(&m_wndControlPanel,
        !m_wndControlPanel.IsWindowVisible(), FALSE);
}

void CMainFrame::OnUpdatePlaybackPlay(CCmdUI* pCmdUI) {
    if (theApp.m_engine) {
        pCmdUI->Enable(!WE_IsPlaying(theApp.m_engine) ||
            WE_IsPaused(theApp.m_engine));
    } else {
        pCmdUI->Enable(FALSE);
    }
}

void CMainFrame::OnUpdatePlaybackPause(CCmdUI* pCmdUI) {
    if (theApp.m_engine) {
        pCmdUI->Enable(WE_IsPlaying(theApp.m_engine) &&
            !WE_IsPaused(theApp.m_engine));
    } else {
        pCmdUI->Enable(FALSE);
    }
}

void CMainFrame::OnUpdatePlaybackStop(CCmdUI* pCmdUI) {
    if (theApp.m_engine) {
        pCmdUI->Enable(WE_IsPlaying(theApp.m_engine));
    } else {
        pCmdUI->Enable(FALSE);
    }
}

BOOL CMainFrame::LoadWallpaper(const CString& path) {
    if (!theApp.m_engine) {
        AfxMessageBox(_T("Engine not initialized!"));
        return FALSE;
    }

    // Check if assets path is set
    if (theApp.m_assetsPath.IsEmpty()) {
        AfxMessageBox(_T("Please set assets path first (File > Set Assets Path)"));
        return FALSE;
    }

    // Stop current playback if any
    if (WE_IsPlaying(theApp.m_engine)) {
        WE_Stop(theApp.m_engine);
    }

    // Load the wallpaper
    if (WE_LoadWallpaper(theApp.m_engine, CT2A(path))) {
        theApp.m_wallpaperPath = path;
        m_wndStatusBar.SetWindowText(_T("Loaded: ") + path);

        // Refresh control panel
        m_wndControlPanel.RefreshProperties();

        // Auto-play
        OnPlaybackPlay();
        return TRUE;
    } else {
        AfxMessageBox(_T("Failed to load wallpaper: ") +
            CString(WE_GetLastError(theApp.m_engine)));
        return FALSE;
    }
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra,
    AFX_CMDHANDLERINFO* pHandlerInfo) {
    // Let control panel handle its commands first
    if (m_wndControlPanel.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo)) {
        return TRUE;
    }

    return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
