#pragma once

#include "resource.h"

class CDemoMFCApp : public CWinApp {
public:
    CDemoMFCApp();

public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    DECLARE_MESSAGE_MAP()

    // Engine instance
    WE_Engine* m_engine;

    // Configuration
    CString m_assetsPath;
    CString m_wallpaperPath;
};

extern CDemoMFCApp theApp;
