#pragma once


// CUserID 대화 상자

class CUserID : public CDialog
{
	DECLARE_DYNAMIC(CUserID)

public:
	CUserID(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CUserID();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_USER_ID };
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
	LPINSPWORKINFO lpInspWorkInfo;

	void Lf_InitLocalValue();
	void Lf_InitFontset();
	void Lf_InitColorBrush();
	void Lf_InitDialogDesign();

	BOOL Lf_confirmUserID();

private:
	CFont m_Font[FONT_IDX_MAX];
	CBrush m_Brush[COLOR_IDX_MAX];
	CFont* m_pDefaultFont;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////



public:
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	CStatic m_picUiImage;
	afx_msg void OnBnClickedBtnUiLogin();
	CButton m_btnUiLogin;
};
