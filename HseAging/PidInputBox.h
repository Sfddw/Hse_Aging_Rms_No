#pragma once

#include "afxdialogex.h"
#include <afxbutton.h>
#include "resource.h"

// FONT_IDX_MAX, COLOR_IDX_MAX, COLOR_WHITE 같은 값이
// _GlobalDefine.h에 있으면 아래 include 사용
#include "_GlobalDefine.h"

class CPidInputBox : public CDialogEx
{
	DECLARE_DYNAMIC(CPidInputBox)

public:
	CPidInputBox(CWnd* pParent = nullptr);
	virtual ~CPidInputBox();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PID_BOX };
#endif

public:
	CString m_strPid;
	CString m_strTitle;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

private:
	CEdit m_edtPid;
	CMFCButton m_btnOk;
	CMFCButton m_btnCancel;

	CString m_strKeyBuffer;

	CFont m_Font[FONT_IDX_MAX];
	CBrush m_Brush[COLOR_IDX_MAX];

	void Lf_InitFontset();
	void Lf_InitColorBrush();
	void Lf_InitDialogDesign();

	void AppendScannedKey(MSG* pMsg);
	BOOL CompletePidInput();
};