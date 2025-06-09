#pragma once


// CSystem 대화 상자

class CSystem : public CDialog
{
	DECLARE_DYNAMIC(CSystem)

public:
	CSystem(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CSystem();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SYSTEM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
// 사용자 정의 Function
///////////////////////////////////////////////////////////////////////////
public:


protected:
	LPSYSTEMINFO lpSystemInfo;
	LPMODELINFO lpModelInfo;


	void Lf_InitLocalValue();
	void Lf_InitFontset();
	void Lf_InitColorBrush();
	void Lf_InitDialogDesign();
	void Lf_InitDialogControl();

	void Lf_saveSystemInfo();

private:
	CFont m_Font[FONT_IDX_MAX];
	CBrush m_Brush[COLOR_IDX_MAX];
	CFont* m_pDefaultFont;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////



public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedBtnSaveExit();
	afx_msg void OnBnClickedBtnCancel();
	CButton m_btnSySaveExit;
	CButton m_btnSyCancel;
	CComboBox m_cmbSyRecorderPort;
	CComboBox m_cmbSyTempLogInterval;
	CComboBox m_cmbSySensingLogInterval;
	afx_msg void OnBnClickedMbtSyPasswordChange();
};
