#include "pch.h"
#include "HseAging.h"
#include "PidInputBox.h"
#include "afxdialogex.h"
#include <UxTheme.h>

#pragma comment(lib, "UxTheme.lib")

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

	DDX_Control(pDX, IDC_PID_INPUT_BOX, m_edtPid);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CPidInputBox, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDOK, &CPidInputBox::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPidInputBox::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CPidInputBox::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (!m_strTitle.IsEmpty())
		SetWindowText(m_strTitle);

	Lf_InitFontset();
	Lf_InitColorBrush();
	Lf_InitDialogDesign();

	SendMessageToDescendants(
		WM_SETFONT,
		(WPARAM)m_Font[0].GetSafeHandle(),
		1,
		TRUE,
		FALSE
	);

	m_strPid.Empty();
	m_strKeyBuffer.Empty();

	m_edtPid.SetWindowText(_T(""));
	m_edtPid.SetLimitText(PID_LENGTH);
	m_edtPid.SetFont(&m_Font[2]);
	m_edtPid.SetFocus();

	return FALSE;
}

void CPidInputBox::OnDestroy()
{
	CDialogEx::OnDestroy();

	for (int i = 0; i < COLOR_IDX_MAX; i++)
	{
		m_Brush[i].DeleteObject();
	}

	for (int i = 0; i < FONT_IDX_MAX; i++)
	{
		m_Font[i].DeleteObject();
	}
}

void CPidInputBox::Lf_InitFontset()
{
	for (int i = 0; i < FONT_IDX_MAX; i++)
	{
		m_Font[i].DeleteObject();
	}

	// 기본 폰트
	m_Font[0].CreateFont(
		15, 6, 0, 0,
		FW_BOLD,
		0, 0, 0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		DEFAULT_FONT
	);

	// 버튼용
	m_Font[1].CreateFont(
		16, 7, 0, 0,
		FW_BOLD,
		0, 0, 0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		DEFAULT_FONT
	);

	// PID 입력 Edit용
	m_Font[2].CreateFont(
		22, 9, 0, 0,
		FW_BOLD,
		0, 0, 0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		DEFAULT_FONT
	);
}

void CPidInputBox::Lf_InitColorBrush()
{
	for (int i = 0; i < COLOR_IDX_MAX; i++)
	{
		m_Brush[i].DeleteObject();
	}

	m_Brush[COLOR_IDX_BLACK].CreateSolidBrush(COLOR_BLACK);
	m_Brush[COLOR_IDX_WHITE].CreateSolidBrush(COLOR_WHITE);
	m_Brush[COLOR_IDX_RED].CreateSolidBrush(COLOR_RED);
	m_Brush[COLOR_IDX_SKYBLUE].CreateSolidBrush(COLOR_SKYBLUE);
	m_Brush[COLOR_IDX_JADEBLUE].CreateSolidBrush(COLOR_JADEBLUE);
	m_Brush[COLOR_IDX_LIGHT_GREEN].CreateSolidBrush(COLOR_LIGHT_GREEN);
	m_Brush[COLOR_IDX_GRAY192].CreateSolidBrush(COLOR_GRAY192);
	m_Brush[COLOR_IDX_GRAY224].CreateSolidBrush(COLOR_GRAY224);
	m_Brush[COLOR_IDX_DARK_NAVY].CreateSolidBrush(COLOR_DARK_NAVY);

	// 혹시 색상 인덱스가 프로젝트에 있으면 유지
	// PidInput.cpp에서도 사용하는 브러시
	m_Brush[COLOR_IDX_ITEM_TITLE].CreateSolidBrush(COLOR_ITEM_TITLE);
	m_Brush[COLOR_IDX_ITEM_HEAD].CreateSolidBrush(COLOR_ITEM_HEAD);
	m_Brush[COLOR_IDX_DARK_BG].CreateSolidBrush(COLOR_DARK_BG);
}

void CPidInputBox::Lf_InitDialogDesign()
{
	// 기본 윈도우 테마 제거
	if (GetDlgItem(IDC_PID_INPUT_BOX) != nullptr)
		SetWindowTheme(GetDlgItem(IDC_PID_INPUT_BOX)->GetSafeHwnd(), L"", L"");

	if (GetDlgItem(IDOK) != nullptr)
		SetWindowTheme(GetDlgItem(IDOK)->GetSafeHwnd(), L"", L"");

	if (GetDlgItem(IDCANCEL) != nullptr)
		SetWindowTheme(GetDlgItem(IDCANCEL)->GetSafeHwnd(), L"", L"");

	// Edit Control 디자인
	m_edtPid.SetFont(&m_Font[2]);

	// IDOK 버튼 디자인
	m_btnOk.SetFont(&m_Font[1]);
	m_btnOk.EnableWindowsTheming(FALSE);
	m_btnOk.SetFaceColor(COLOR_GRAY224);
	m_btnOk.SetTextColor(COLOR_BLACK);
	m_btnOk.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_OX_O));

	// CANCEL 버튼도 같이 맞춤
	m_btnCancel.SetFont(&m_Font[1]);
	m_btnCancel.EnableWindowsTheming(FALSE);
	m_btnCancel.SetFaceColor(COLOR_GRAY224);
	m_btnCancel.SetTextColor(COLOR_BLACK);
	m_btnCancel.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_OX_X));
}

HBRUSH CPidInputBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (nCtlColor)
	{
	case CTLCOLOR_DLG:
		pDC->SetBkColor(COLOR_GRAY192);
		return m_Brush[COLOR_IDX_GRAY192];

	case CTLCOLOR_EDIT:
		if (pWnd->GetDlgCtrlID() == IDC_PID_INPUT_BOX)
		{
			// PidInput.cpp의 PID 입력 대기 색상과 동일
			pDC->SetBkColor(COLOR_WHITE);
			pDC->SetTextColor(COLOR_BLACK);
			return m_Brush[COLOR_IDX_WHITE];
		}
		break;

	case CTLCOLOR_STATIC:
		pDC->SetBkColor(COLOR_GRAY192);
		pDC->SetTextColor(COLOR_BLACK);
		return m_Brush[COLOR_IDX_GRAY192];

	case CTLCOLOR_BTN:
		pDC->SetBkColor(COLOR_GRAY192);
		pDC->SetTextColor(COLOR_BLACK);
		return m_Brush[COLOR_IDX_GRAY192];
	}

	return hbr;
}

void CPidInputBox::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	// PidInput.cpp의 OnPaint와 동일한 배경 느낌
	dc.FillSolidRect(rect, COLOR_GRAY192);

	// 상단 네이비 타이틀 영역을 쓰고 싶으면 아래 사용
	CRect titleRect = rect;
	titleRect.bottom = 45;
	dc.FillSolidRect(titleRect, COLOR_DARK_NAVY);

	CDialogEx::OnPaint();
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
	m_edtPid.SetSel(m_strKeyBuffer.GetLength(), m_strKeyBuffer.GetLength());
}

BOOL CPidInputBox::CompletePidInput()
{
	CString pid;
	m_edtPid.GetWindowText(pid);
	pid.Trim();

	if (pid.GetLength() != PID_LENGTH)
	{
		CString msg;
		msg.Format(
			_T("PID length must be %d.\r\nCurrent PID: [%s]"),
			PID_LENGTH,
			pid.GetString()
		);

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