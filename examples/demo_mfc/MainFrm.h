#pragma once

#include "ControlPanel.h"

class CMainFrame : public CFrameWnd {
    DECLARE_DYNAMIC(CMainFrame)

public:
    CMainFrame();
    virtual ~CMainFrame();

    // Window for embedding wallpaper
    HWND GetWallpaperWindow() const { return m_hWallpaperWnd; }

protected:
    DECLARE_MESSAGE_MAP()

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);

    // Menu commands
    afx_msg void OnFileLoadWallpaper();
    afx_msg void OnFileSetAssets();
    afx_msg void OnFileExit();
    afx_msg void OnPlaybackPlay();
    afx_msg void OnPlaybackPause();
    afx_msg void OnPlaybackStop();
    afx_msg void OnViewControlPanel();
    afx_msg void OnUpdatePlaybackPlay(CCmdUI* pCmdUI);
    afx_msg void OnUpdatePlaybackPause(CCmdUI* pCmdUI);
    afx_msg void OnUpdatePlaybackStop(CCmdUI* pCmdUI);

private:
    // Child window for wallpaper rendering
    HWND m_hWallpaperWnd;

    // Control panel
    CControlPanelBar m_wndControlPanel;

    // Status bar
    CStatusBar m_wndStatusBar;

    // Splitter for multi-screen support
    BOOL CreateWallpaperWindow(const CRect& rect);

    // Load wallpaper helper
    BOOL LoadWallpaper(const CString& path);
};
