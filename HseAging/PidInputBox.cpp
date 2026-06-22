#include "pch.h"
#include "HseAging.h"
#include "PidInputBox.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CPidInputBox, CDialogEx)

CPidInputBox::CPidInputBox(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PID_BOX, pParent)
{
	m_strPid.Empty();
	m_strTitle.Empty();
	m_strKeyBuffer.Empty();
}

CPidInputBox::~CPidInputBox()
{
}

void CPidInputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	// Edit Control ID는 실제 리소스 ID에 맞게 수정
	DDX_Control(pDX, IDC_PID_INPUT_BOX, m_edtPid);
}

BEGIN_MESSAGE_MAP(CPidInputBox, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPidInputBox::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPidInputBox::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CPidInputBox::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (!m_strTitle.IsEmpty())
		SetWindowText(m_strTitle);

	m_strPid.Empty();
	m_strKeyBuffer.Empty();

	m_edtPid.SetWindowText(_T(""));
	m_edtPid.SetLimitText(PID_LENGTH);
	m_edtPid.SetFocus();

	return FALSE; // Edit Control에 포커스를 주기 위해 FALSE
}

BOOL CPidInputBox::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_F4)
	{
		if (::GetKeyState(VK_MENU) < 0)
			return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_ESCAPE:
			OnBnClickedCancel();
			return TRUE;

		case VK_RETURN:
			return CompletePidInput();

		case VK_SPACE:
			return TRUE;
		}

		// 바코드 스캐너 문자 입력
		if ((pMsg->wParam >= 33) && (pMsg->wParam <= 'z'))
		{
			AppendScannedKey(pMsg);
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CPidInputBox::AppendScannedKey(MSG* pMsg)
{
	if (m_strKeyBuffer.GetLength() >= PID_LENGTH)
		return;

	CString s;
	s.Format(_T("%c"), pMsg->wParam);

	m_strKeyBuffer.Append(s);
	m_edtPid.SetWindowText(m_strKeyBuffer);

	if (m_strKeyBuffer.GetLength() == PID_LENGTH)
	{
		// 스캐너가 Enter를 자동으로 보내는 경우가 많으므로
		// 여기서 바로 닫지 않고 Enter를 기다리는 방식.
		// 바로 닫고 싶으면 아래 한 줄 사용 가능:
		// CompletePidInput();
	}
}

BOOL CPidInputBox::CompletePidInput()
{
	CString pid;
	m_edtPid.GetWindowText(pid);
	pid.Trim();

	if (pid.GetLength() != PID_LENGTH)
	{
		CString msg;
		msg.Format(_T("PID length must be %d.\r\nCurrent PID: [%s]"),
			PID_LENGTH,
			pid.GetString());

		AfxMessageBox(msg, MB_ICONWARNING);

		m_strKeyBuffer.Empty();
		m_edtPid.SetWindowText(_T(""));
		m_edtPid.SetFocus();

		return TRUE;
	}

	m_strPid = pid;
	EndDialog(IDOK);

	return TRUE;
}

void CPidInputBox::OnBnClickedOk()
{
	CompletePidInput();
}

void CPidInputBox::OnBnClickedCancel()
{
	m_strPid.Empty();
	EndDialog(IDCANCEL);
}