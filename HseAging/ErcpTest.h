#pragma once
#include "afxdialogex.h"


// ErcpTest 대화 상자

class ErcpTest : public CDialogEx
{
	DECLARE_DYNAMIC(ErcpTest)

public:
	ErcpTest(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~ErcpTest();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ERCP_TEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
