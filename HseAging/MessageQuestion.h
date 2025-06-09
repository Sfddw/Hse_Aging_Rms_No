#pragma once


// CMessageQuestion 대화 상자

class CMessageQuestion : public CDialog
{
	DECLARE_DYNAMIC(CMessageQuestion)

public:
	CMessageQuestion(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CMessageQuestion();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MESSAGE_QUESTION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
// 사용자 정의 Function
///////////////////////////////////////////////////////////////////////////
public:
	CString m_strQMessage;
	CString m_strLButton;
	CString m_strRButton;


protected:


	void Lf_InitLocalValue();
	void Lf_InitFontset();
	void Lf_InitColorBrush();
	void Lf_InitDialogDesign();

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
	afx_msg void OnBnClickedBtnMqConfirm();
	afx_msg void OnBnClickedBtnMqCancel();
	CButton m_btnMqConfirm;
	CButton m_btnMqCancel;
	CStatic m_picMqQuestion;
};
