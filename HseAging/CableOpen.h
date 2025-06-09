#pragma once


// CCableOpen 대화 상자

class CCableOpen : public CDialog
{
	DECLARE_DYNAMIC(CCableOpen)

public:
	CCableOpen(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CCableOpen();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CABLE_OPEN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()




///////////////////////////////////////////////////////////////////////////
// 사용자 정의 Function
///////////////////////////////////////////////////////////////////////////
public:
	int m_nRackNo;

protected:
	LPMODELINFO		lpModelInfo;
	LPSYSTEMINFO	lpSystemInfo;
	LPINSPWORKINFO	lpInspWorkInfo;
	CStatic* m_pSttCableOpen[MAX_LAYER][MAX_LAYER_CHANNEL];

	void Lf_InitLocalValue();
	void Lf_InitFontset();
	void Lf_InitColorBrush();
	void Lf_InitDialogDesign();
	void Lf_refreshCableOpen();
	void Lf_updateCableOpenResult();


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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedMbcCoRetry();
	afx_msg void OnBnClickedMbcCoCancel();
	CMFCButton m_mbcCoRetry;
	CMFCButton m_mbcCoCancel;
};
