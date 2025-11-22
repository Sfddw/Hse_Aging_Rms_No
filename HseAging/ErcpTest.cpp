// ErcpTest.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "HseAging.h"
#include "HseAgingDlg.h"
#include "afxdialogex.h"
#include "UserID.h"
#include "Monitoring.h"
#include "ModelInfo.h"
#include "PidInput.h"
#include "System.h"
#include "MessageQuestion.h"
#include "AutoFirmware.h"
#include "CableOpen.h"
#include "Password.h"
#include "ErcpTest.h"


// ErcpTest 대화 상자

IMPLEMENT_DYNAMIC(ErcpTest, CDialogEx)

ErcpTest::ErcpTest(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ERCP_TEST, pParent)
{

}

ErcpTest::~ErcpTest()
{
}

void ErcpTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ErcpTest, CDialogEx)
	ON_BN_CLICKED(IDOK, &ErcpTest::OnBnClickedOk)
END_MESSAGE_MAP()


// ErcpTest 메시지 처리기


void ErcpTest::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strInput;
	GetDlgItemText(IDC_ERCP_SEND, strInput);
	LPINSPWORKINFO lpInspWorkInfo = m_pApp->GetInspWorkInfo();
	lpInspWorkInfo->Ercp_Test_Message = strInput;
	m_pApp->Gf_gmesSendHost(HOST_ERCP_TEST, NULL, NULL, NULL);

	m_pApp->Gf_writeMLog(strInput);
	//CDialogEx::OnOK();
}
