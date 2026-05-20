#include "stdafx.h"
#include "DemoMFC.h"
#include "MainFrm.h"
#include "ControlPanel.h"
#include <commdlg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Dialog template for the control panel
static DLGTEMPLATE s_dlgTemplate = {
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
    0, 0, 0, 0, 0,
    0, 0, (WORD)0xFFFF, (WORD)0x0000, 0, 0, 0, 0, nullptr, 0, nullptr, 0
};

BEGIN_MESSAGE_MAP(CControlPanelBar, CDialogBar)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_HSCROLL()
    ON_COMMAND(IDC_PROPERTY_SET, OnPropertySet)
    ON_COMMAND(IDC_PROPERTY_REFRESH, OnPropertyRefresh)
    ON_COMMAND(IDC_PLAY_BUTTON, OnPlay)
    ON_COMMAND(IDC_PAUSE_BUTTON, OnPause)
    ON_COMMAND(IDC_STOP_BUTTON, OnStop)
    ON_COMMAND(IDC_SCREENSHOT_BUTTON, OnScreenshot)
    ON_BN_CLICKED(IDC_MUTE_CHECK, OnMuteCheck)
    ON_EN_CHANGE(IDC_FPS_EDIT, OnFpsChange)
    ON_LBN_SELCHANGE(IDC_PROPERTIES_LIST, OnPropertySet)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CControlPanelBar, CDialogBar)

CControlPanelBar::CControlPanelBar()
    : m_pPropertyList(nullptr) {
}

CControlPanelBar::~CControlPanelBar() {
    if (m_pPropertyList) {
        WE_PropertyList_Destroy(m_pPropertyList);
        m_pPropertyList = nullptr;
    }
}

BOOL CControlPanelBar::Create(CWnd* pParentWnd, UINT nIDTemplate,
    UINT nStyle, UINT nID) {

    // Override the template to use our own
    return CDialogBar::Create(pParentWnd, &s_dlgTemplate, nStyle, nID);
}

CSize CControlPanelBar::CalcDynamicLayout(int nLength, DWORD dwMode) {
    // Return a fixed width for the control panel
    if (dwMode & LM_HORZDOCK) {
        return CSize(280, 32767);
    }
    if (dwMode & LM_VERTDOCK) {
        return CSize(280, 32767);
    }
    if (dwMode & LM_MRUWIDTH) {
        return CSize(280, 400);
    }
    return CSize(280, 400);
}

int CControlPanelBar::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CDialogBar::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    // Create controls programmatically
    CRect rect(5, 5, 270, 200);

    // Playback controls group
    rect = CRect(5, 5, 270, 80);
    CWnd* pGroup = new CWnd;
    pGroup->Create(_T("Playback"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        rect, this, 0);

    // Play button
    m_wndPlayBtn.Create(_T("Play"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(15, 20, 90, 40), this, IDC_PLAY_BUTTON);
    m_wndPlayBtn.SetFont(GetFont());

    // Pause button
    m_wndPauseBtn.Create(_T("Pause"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(100, 20, 175, 40), this, IDC_PAUSE_BUTTON);
    m_wndPauseBtn.SetFont(GetFont());

    // Stop button
    m_wndStopBtn.Create(_T("Stop"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(185, 20, 260, 40), this, IDC_STOP_BUTTON);
    m_wndStopBtn.SetFont(GetFont());

    // Volume slider
    m_wndVolumeSlider.Create(WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_HORZ,
        CRect(15, 50, 200, 65), this, IDC_VOLUME_SLIDER);
    m_wndVolumeSlider.SetRange(0, 128);
    m_wndVolumeSlider.SetPos(15);

    CWnd* pVolLabel = new CWnd;
    pVolLabel->Create(_T("Volume:"), WS_CHILD | WS_VISIBLE,
        CRect(15, 48, 60, 58), this, 0);
    pVolLabel->SetFont(GetFont());

    // Mute checkbox
    m_wndMuteCheck.Create(_T("Mute"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        CRect(205, 48, 260, 62), this, IDC_MUTE_CHECK);
    m_wndMuteCheck.SetFont(GetFont());

    // Screenshot button
    m_wndScreenshotBtn.Create(_T("Screenshot"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(15, 200, 260, 220), this, IDC_SCREENSHOT_BUTTON);
    m_wndScreenshotBtn.SetFont(GetFont());

    // FPS control
    rect = CRect(5, 225, 270, 280);
    pGroup = new CWnd;
    pGroup->Create(_T("Settings"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        rect, this, 0);

    CWnd* pFpsLabel = new CWnd;
    pFpsLabel->Create(_T("Max FPS:"), WS_CHILD | WS_VISIBLE,
        CRect(15, 243, 70, 253), this, 0);
    pFpsLabel->SetFont(GetFont());

    m_wndFpsEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        CRect(75, 240, 120, 256), this, IDC_FPS_EDIT);
    m_wndFpsEdit.SetFont(GetFont());
    m_wndFpsEdit.SetWindowText(_T("30"));

    // Properties list group
    rect = CRect(5, 285, 270, 580);
    pGroup = new CWnd;
    pGroup->Create(_T("Properties"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        rect, this, 0);

    // Properties list
    m_wndPropertiesList.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        WS_BORDER | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        CRect(15, 300, 260, 470), this, IDC_PROPERTIES_LIST);
    m_wndPropertiesList.SetFont(GetFont());

    // Property value edit
    CWnd* pValLabel = new CWnd;
    pValLabel->Create(_T("Value:"), WS_CHILD | WS_VISIBLE,
        CRect(15, 475, 60, 485), this, 0);
    pValLabel->SetFont(GetFont());

    m_wndPropertyValue.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        CRect(15, 490, 260, 506), this, IDC_PROPERTY_VALUE);
    m_wndPropertyValue.SetFont(GetFont());

    // Set button
    m_wndPropertySet.Create(_T("Set"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(15, 512, 120, 532), this, IDC_PROPERTY_SET);
    m_wndPropertySet.SetFont(GetFont());

    // Refresh button
    m_wndPropertyRefresh.Create(_T("Refresh"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(130, 512, 255, 532), this, IDC_PROPERTY_REFRESH);
    m_wndPropertyRefresh.SetFont(GetFont());

    // Initialize
    UpdateVolumeSlider();
    UpdateFpsEdit();
    UpdatePlaybackButtons();
    RefreshProperties();

    return 0;
}

void CControlPanelBar::OnSize(UINT nType, int cx, int cy) {
    CDialogBar::OnSize(nType, cx, cy);
}

void CControlPanelBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
    if (pScrollBar && (CWnd*)pScrollBar == &m_wndVolumeSlider) {
        int volume = m_wndVolumeSlider.GetPos();
        if (theApp.m_engine) {
            WE_SetVolume(theApp.m_engine, volume);
        }
    }
    CDialogBar::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CControlPanelBar::OnPlay() {
    if (theApp.m_engine) {
        if (WE_Play(theApp.m_engine)) {
            UpdatePlaybackButtons();
        }
    }
}

void CControlPanelBar::OnPause() {
    if (theApp.m_engine) {
        if (WE_Pause(theApp.m_engine)) {
            UpdatePlaybackButtons();
        }
    }
}

void CControlPanelBar::OnStop() {
    if (theApp.m_engine) {
        if (WE_Stop(theApp.m_engine)) {
            UpdatePlaybackButtons();
        }
    }
}

void CControlPanelBar::OnScreenshot() {
    if (!theApp.m_engine) {
        return;
    }

    CFileDialog dlg(FALSE, _T("png"), _T("screenshot"),
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("PNG Files (*.png)|*.png|All Files (*.*)|*.*||"),
        this);

    if (dlg.DoModal() == IDOK) {
        if (WE_TakeScreenshot(theApp.m_engine, CT2A(dlg.GetPathName()))) {
            AfxMessageBox(_T("Screenshot saved!"));
        } else {
            AfxMessageBox(_T("Failed to take screenshot: ") +
                CString(WE_GetLastError(theApp.m_engine)));
        }
    }
}

void CControlPanelBar::OnMuteCheck() {
    if (theApp.m_engine) {
        BOOL mute = m_wndMuteCheck.GetCheck();
        WE_SetMuted(theApp.m_engine, mute ? TRUE : FALSE);
    }
}

void CControlPanelBar::OnFpsChange() {
    CString str;
    m_wndFpsEdit.GetWindowText(str);
    int fps = _ttoi(str);
    if (fps > 0 && theApp.m_engine) {
        WE_SetMaxFPS(theApp.m_engine, fps);
    }
}

void CControlPanelBar::OnPropertySet() {
    int sel = m_wndPropertiesList.GetCurSel();
    if (sel == LB_ERR || !theApp.m_engine) {
        return;
    }

    // Get property name from list
    CString text;
    m_wndPropertiesList.GetText(sel, text);

    // Parse property name (format: "name = value")
    int eqPos = text.Find(_T(" = "));
    if (eqPos == -1) {
        return;
    }
    CString propName = text.Left(eqPos);

    // Get new value
    CString newValue;
    m_wndPropertyValue.GetWindowText(newValue);

    // Set property
    if (WE_SetProperty(theApp.m_engine, CT2A(propName), CT2A(newValue))) {
        RefreshProperties();
    } else {
        AfxMessageBox(_T("Failed to set property: ") +
            CString(WE_GetLastError(theApp.m_engine)));
    }
}

void CControlPanelBar::OnPropertyRefresh() {
    RefreshProperties();
}

void CControlPanelBar::RefreshProperties() {
    if (!theApp.m_engine) {
        return;
    }

    m_wndPropertiesList.ResetContent();

    // Free old property list
    if (m_pPropertyList) {
        WE_PropertyList_Destroy(m_pPropertyList);
        m_pPropertyList = nullptr;
    }

    // Get new property list
    m_pPropertyList = WE_ListProperties(theApp.m_engine);
    if (!m_pPropertyList) {
        return;
    }

    // Populate list
    int count = WE_PropertyList_GetCount(m_pPropertyList);
    for (int i = 0; i < count; i++) {
        const char* name = WE_PropertyList_GetName(m_pPropertyList, i);
        const char* value = WE_PropertyList_GetValue(m_pPropertyList, i);

        CString item;
        item.Format(_T("%s = %s"), CString(name), CString(value));
        m_wndPropertiesList.AddString(item);
    }
}

void CControlPanelBar::UpdateVolumeSlider() {
    if (theApp.m_engine) {
        // Default volume 15
        m_wndVolumeSlider.SetPos(15);
    }
}

void CControlPanelBar::UpdateFpsEdit() {
    if (theApp.m_engine) {
        m_wndFpsEdit.SetWindowText(_T("30"));
    }
}

void CControlPanelBar::UpdatePlaybackButtons() {
    if (!theApp.m_engine) {
        m_wndPlayBtn.EnableWindow(FALSE);
        m_wndPauseBtn.EnableWindow(FALSE);
        m_wndStopBtn.EnableWindow(FALSE);
        return;
    }

    bool isPlaying = WE_IsPlaying(theApp.m_engine) != 0;
    bool isPaused = WE_IsPaused(theApp.m_engine) != 0;

    m_wndPlayBtn.EnableWindow(!isPlaying || isPaused);
    m_wndPauseBtn.EnableWindow(isPlaying && !isPaused);
    m_wndStopBtn.EnableWindow(isPlaying);
}
