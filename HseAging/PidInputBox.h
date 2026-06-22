#pragma once
#include "afxdialogex.h"

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
	CString m_strTitle;      // 예: RACK01CH33
	CString m_strPid;        // 입력된 PID 결과

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

private:
	CEdit m_edtPid;
	CString m_strKeyBuffer;

	void AppendScannedKey(MSG* pMsg);
	BOOL CompletePidInput();
};