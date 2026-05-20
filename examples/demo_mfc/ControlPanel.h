#pragma once

#include <afxctrlb.h>
#include <afxwin.h>

class CControlPanelBar : public CDialogBar {
    DECLARE_DYNAMIC(CControlPanelBar)

public:
    CControlPanelBar();
    virtual ~CControlPanelBar();

    // Refresh the properties list
    void RefreshProperties();

protected:
    DECLARE_MESSAGE_MAP()

    virtual BOOL Create(CWnd* pParentWnd, UINT nIDTemplate,
        UINT nStyle = CBRS_LEFT, UINT nID = IDD_CONTROL_PANEL);
    virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnPropertySet();
    afx_msg void OnPropertyRefresh();
    afx_msg void OnPlay();
    afx_msg void OnPause();
    afx_msg void OnStop();
    afx_msg void OnScreenshot();
    afx_msg void OnMuteCheck();
    afx_msg void OnFpsChange();

private:
    // Controls
    CListBox m_wndPropertiesList;
    CEdit m_wndPropertyValue;
    CButton m_wndPropertySet;
    CButton m_wndPropertyRefresh;
    CSliderCtrl m_wndVolumeSlider;
    CButton m_wndMuteCheck;
    CEdit m_wndFpsEdit;
    CButton m_wndPlayBtn;
    CButton m_wndPauseBtn;
    CButton m_wndStopBtn;
    CButton m_wndScreenshotBtn;

    // Current properties
    WE_PropertyList* m_pPropertyList;

    // Setup controls helper
    void SetupControls();
    void UpdateVolumeSlider();
    void UpdateFpsEdit();
    void UpdatePlaybackButtons();
};
