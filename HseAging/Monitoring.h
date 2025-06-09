#pragma once


// CMonitoring 대화 상자

class CMonitoring : public CDialog
{
	DECLARE_DYNAMIC(CMonitoring)

public:
	CMonitoring(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CMonitoring();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MONITORING };
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
	LPINSPWORKINFO lpInspWorkInfo;

	CListCtrl* pListControl[MAX_RACK];

	void Lf_InitLocalValue();
	void Lf_InitFontset();
	void Lf_InitColorBrush();
	void Lf_InitDialogDesign();

	void Lf_InitListControl();
	void Lf_InsertItemListControl();

	void Lf_updatePowerMeasValue();

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
	CListCtrl m_lstMtListRack1;
	CListCtrl m_lstMtListRack2;
	CListCtrl m_lstMtListRack3;
	CListCtrl m_lstMtListRack4;
	CListCtrl m_lstMtListRack5;
	CListCtrl m_lstMtListRack6;
	afx_msg void OnNMCustomdrawLstMtRack1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawLstMtRack2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawLstMtRack3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawLstMtRack4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawLstMtRack5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawLstMtRack6(NMHDR* pNMHDR, LRESULT* pResult);
};
