// System.cpp: 구현 파일
//

#include "pch.h"
#include "HseAging.h"
#include "System.h"
#include "afxdialogex.h"
#include "MessageQuestion.h"


// CSystem 대화 상자

IMPLEMENT_DYNAMIC(CSystem, CDialog)

CSystem::CSystem(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SYSTEM, pParent)
{
	m_pDefaultFont = new CFont();
	m_pDefaultFont->CreateFont(15, 6, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

CSystem::~CSystem()
{
	if (m_pDefaultFont != NULL)
	{
		delete m_pDefaultFont;
	}
}

void CSystem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_SY_SAVE_EXIT, m_btnSySaveExit);
	DDX_Control(pDX, IDC_BTN_SY_CANCEL, m_btnSyCancel);
	DDX_Control(pDX, IDC_CMB_SY_RECORDER_PORT, m_cmbSyRecorderPort);
	DDX_Control(pDX, IDC_CMB_SY_TEMP_LOG_INTERVAL, m_cmbSyTempLogInterval);
	DDX_Control(pDX, IDC_CMB_SY_SENSING_LOG_INTERVAL, m_cmbSySensingLogInterval);
}


BEGIN_MESSAGE_MAP(CSystem, CDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_SY_SAVE_EXIT, &CSystem::OnBnClickedBtnSaveExit)
	ON_BN_CLICKED(IDC_BTN_SY_CANCEL, &CSystem::OnBnClickedBtnCancel)
	ON_BN_CLICKED(IDC_MBT_SY_PASSWORD_CHANGE, &CSystem::OnBnClickedMbtSyPasswordChange)
END_MESSAGE_MAP()


// CSystem 메시지 처리기


BOOL CSystem::OnInitDialog()
{
	CDialog::OnInitDialog();
	lpSystemInfo = m_pApp->GetSystemInfo();
	lpModelInfo = m_pApp->GetModelInfo();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_pApp->Gf_writeMLog(_T("<TEST> 'System' Dialog Open"));

	// Dialog의 기본 FONT 설정.
	SendMessageToDescendants(WM_SETFONT, (WPARAM)m_pDefaultFont->GetSafeHandle(), 1, TRUE, FALSE);

	Lf_InitLocalValue();
	Lf_InitFontset();
	Lf_InitColorBrush();
	Lf_InitDialogDesign();
	Lf_InitDialogControl();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CSystem::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	for (int i = 0; i < COLOR_IDX_MAX; i++)
	{
		m_Brush[i].DeleteObject();
	}

	for (int i = 0; i < FONT_IDX_MAX; i++)
	{
		m_Font[i].DeleteObject();
	}
}


BOOL CSystem::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_F4)
	{
		if (::GetKeyState(VK_MENU) < 0)	return TRUE;
	}

	// 일반 Key 동작에 대한 Event
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
			case VK_ESCAPE:
				return 1;
			case VK_RETURN:
				return 1;
			case VK_SPACE:
				return 1;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CSystem::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  여기서 DC의 특성을 변경합니다.
	switch (nCtlColor)
	{
		case CTLCOLOR_MSGBOX:
			break;
		case CTLCOLOR_EDIT:
			break;
		case CTLCOLOR_LISTBOX:
			break;
		case CTLCOLOR_SCROLLBAR:
			break;
		case CTLCOLOR_BTN:
			break;
		case CTLCOLOR_STATIC:		// Static, CheckBox control
			if (pWnd->GetDlgCtrlID() == IDC_STATIC)
			{
				pDC->SetBkColor(COLOR_SKYBLUE);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_SKYBLUE];
			}
			if (pWnd->GetDlgCtrlID() == IDC_STT_SY_TITLE)
			{
				pDC->SetBkColor(COLOR_DARK_NAVY);
				pDC->SetTextColor(COLOR_WHITE);
				return m_Brush[COLOR_IDX_DARK_NAVY];
			}
			if ((pWnd->GetDlgCtrlID() == IDC_STT_SY_STATION_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_SY_PASSWORD_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_SY_MES_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_SY_EAS_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_SY_REFRESH_TITLE)
				)
			{
				pDC->SetBkColor(COLOR_DARK_NAVY);
				pDC->SetTextColor(COLOR_WHITE);
				return m_Brush[COLOR_IDX_DARK_NAVY];
			}

			break;
	}

	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.
	return hbr;
}


void CSystem::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.

	CRect rect, rectOri;
	GetClientRect(&rect);
	rectOri = rect;

	rect.bottom = 80;
	dc.FillSolidRect(rect, COLOR_DARK_NAVY);

	rect.top = rect.bottom;
	rect.bottom = rectOri.bottom;
	dc.FillSolidRect(rect, COLOR_GRAY192);
}

void CSystem::OnBnClickedMbtSyPasswordChange()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString setPw, curPw, newPw, confirmPw;

	Read_SysIniFile(_T("SYSTEM"), _T("PASSWORD"), &setPw);
	GetDlgItem(IDC_EDT_SY_CUR_PASSWORD)->GetWindowText(curPw);
	GetDlgItem(IDC_EDT_SY_NEW_PASSWORD)->GetWindowText(newPw);
	GetDlgItem(IDC_EDT_SY_CONFIRM_PASSWORD)->GetWindowText(confirmPw);

	if (setPw != curPw)
	{
		AfxMessageBox(_T("Current password is wrong."), MB_ICONERROR);
		return;
	}

	if (newPw != confirmPw)
	{
		AfxMessageBox(_T("New password is different."), MB_ICONERROR);
		return;
	}

	Write_SysIniFile(_T("SYSTEM"), _T("PASSWORD"), newPw);

	AfxMessageBox(_T("Password has been successfully changed."), MB_ICONINFORMATION);
}


void CSystem::OnBnClickedBtnSaveExit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CMessageQuestion msg_dlg;

	msg_dlg.m_strQMessage.Format(_T("System information Save ?"));
	if (msg_dlg.DoModal() == IDOK)
	{
		Lf_saveSystemInfo();

		CDialog::OnOK();
	}

}


void CSystem::OnBnClickedBtnCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialog::OnCancel();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSystem::Lf_InitLocalValue()
{

}

void CSystem::Lf_InitFontset()
{
	m_Font[0].CreateFont(150, 70, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[1].CreateFont(44, 20, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_SY_TITLE)->SetFont(&m_Font[1]);

	m_Font[2].CreateFont(32, 14, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[3].CreateFont(26, 12, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_BTN_SY_SAVE_EXIT)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_BTN_SY_CANCEL)->SetFont(&m_Font[3]);

	m_Font[4].CreateFont(19, 8, 0, 0, FW_BOLD, TRUE, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[5].CreateFont(16, 7, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

void CSystem::Lf_InitColorBrush()
{
	// 각 Control의 COLOR 설정을 위한 Brush를 Setting 한다.
	m_Brush[COLOR_IDX_BLACK].CreateSolidBrush(COLOR_BLACK);
	m_Brush[COLOR_IDX_WHITE].CreateSolidBrush(COLOR_WHITE);
	m_Brush[COLOR_IDX_RED].CreateSolidBrush(COLOR_RED);
	m_Brush[COLOR_IDX_RED128].CreateSolidBrush(COLOR_RED128);
	m_Brush[COLOR_IDX_GREEN].CreateSolidBrush(COLOR_GREEN);
	m_Brush[COLOR_IDX_GREEN128].CreateSolidBrush(COLOR_GREEN128);
	m_Brush[COLOR_IDX_BLUE].CreateSolidBrush(COLOR_BLUE);
	m_Brush[COLOR_IDX_SEABLUE].CreateSolidBrush(COLOR_SEABLUE);
	m_Brush[COLOR_IDX_SKYBLUE].CreateSolidBrush(COLOR_SKYBLUE);
	m_Brush[COLOR_IDX_ORANGE].CreateSolidBrush(COLOR_ORANGE);
	m_Brush[COLOR_IDX_VERDANT].CreateSolidBrush(COLOR_VERDANT);
	m_Brush[COLOR_IDX_JADEGREEN].CreateSolidBrush(COLOR_JADEGREEN);
	m_Brush[COLOR_IDX_JADEBLUE].CreateSolidBrush(COLOR_JADEBLUE);
	m_Brush[COLOR_IDX_JADERED].CreateSolidBrush(COLOR_JADERED);
	m_Brush[COLOR_IDX_LIGHT_RED].CreateSolidBrush(COLOR_LIGHT_RED);
	m_Brush[COLOR_IDX_LIGHT_GREEN].CreateSolidBrush(COLOR_LIGHT_GREEN);
	m_Brush[COLOR_IDX_LIGHT_BLUE].CreateSolidBrush(COLOR_LIGHT_BLUE);
	m_Brush[COLOR_IDX_LIGHT_ORANGE].CreateSolidBrush(COLOR_LIGHT_ORANGE);
	m_Brush[COLOR_IDX_DARK_RED].CreateSolidBrush(COLOR_DARK_RED);
	m_Brush[COLOR_IDX_DARK_ORANGE].CreateSolidBrush(COLOR_DARK_ORANGE);
	m_Brush[COLOR_IDX_GRAY64].CreateSolidBrush(COLOR_GRAY64);
	m_Brush[COLOR_IDX_GRAY128].CreateSolidBrush(COLOR_GRAY128);
	m_Brush[COLOR_IDX_GRAY159].CreateSolidBrush(COLOR_GRAY159);
	m_Brush[COLOR_IDX_GRAY192].CreateSolidBrush(COLOR_GRAY192);
	m_Brush[COLOR_IDX_GRAY224].CreateSolidBrush(COLOR_GRAY224);
	m_Brush[COLOR_IDX_BLUISH].CreateSolidBrush(COLOR_BLUISH);
	m_Brush[COLOR_IDX_DARK_BLUE].CreateSolidBrush(COLOR_DARK_BLUE);
	m_Brush[COLOR_IDX_DARK_NAVY].CreateSolidBrush(COLOR_DARK_NAVY);
}

void CSystem::Lf_InitDialogDesign()
{
	m_btnSySaveExit.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_OX_O));
	m_btnSyCancel.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_OX_X));
}

void CSystem::Lf_InitDialogControl()
{
	CString sdata;

	GetDlgItem(IDC_EDT_SY_CHAMBER_NO)->SetWindowText(lpSystemInfo->m_sChamberNo);
	GetDlgItem(IDC_EDT_SY_EQP_NAME)->SetWindowText(lpSystemInfo->m_sEqpName);
	m_cmbSyRecorderPort.SetCurSel(lpSystemInfo->m_nTempRecorderPort);

	GetDlgItem(IDC_EDT_SY_MES_SERVICE_PORT)->SetWindowText(lpSystemInfo->m_sMesServicePort);
	GetDlgItem(IDC_EDT_SY_MES_NETWORK)->SetWindowText(lpSystemInfo->m_sMesNetWork);
	GetDlgItem(IDC_EDT_SY_MES_DAEMON_PORT)->SetWindowText(lpSystemInfo->m_sMesDaemonPort);
	GetDlgItem(IDC_EDT_SY_MES_LOCAL_SUBJECT)->SetWindowText(lpSystemInfo->m_sMesLocalSubject);
	GetDlgItem(IDC_EDT_SY_MES_REMOTE_SUBJECT)->SetWindowText(lpSystemInfo->m_sMesRemoteSubject);

	GetDlgItem(IDC_EDT_SY_EAS_SERVICE_PORT)->SetWindowText(lpSystemInfo->m_sEasServicePort);
	GetDlgItem(IDC_EDT_SY_EAS_NETWORK)->SetWindowText(lpSystemInfo->m_sEasNetWork);
	GetDlgItem(IDC_EDT_SY_EAS_DAEMON_PORT)->SetWindowText(lpSystemInfo->m_sEasDaemonPort);
	GetDlgItem(IDC_EDT_SY_EAS_LOCAL_SUBJECT)->SetWindowText(lpSystemInfo->m_sEasLocalSubject);
	GetDlgItem(IDC_EDT_SY_EAS_REMOTE_SUBJECT)->SetWindowText(lpSystemInfo->m_sEasRemoteSubject);

	sdata.Format(_T("%.1f"), lpSystemInfo->m_fRefreshAgingStatusTime);
	GetDlgItem(IDC_EDT_SY_REFRESH_AGING_STATUS)->SetWindowText(sdata);
	sdata.Format(_T("%.1f"), lpSystemInfo->m_fRefreshPowerMeasureTime);
	GetDlgItem(IDC_EDT_SY_REFRESH_POWER_MEASURE)->SetWindowText(sdata);

	m_cmbSyTempLogInterval.SetCurSel(lpSystemInfo->m_nTempLogInterval);
	m_cmbSySensingLogInterval.SetCurSel(lpSystemInfo->m_nSensingLogInterval);
}

void CSystem::Lf_saveSystemInfo()
{
	CString sdata = _T("");

	m_pApp->Gf_writeMLog(_T("<WND> SYSTEM Setting Save"));

	GetDlgItem(IDC_EDT_SY_CHAMBER_NO)->GetWindowText(sdata);
	lpSystemInfo->m_sChamberNo.Format(_T("%s"), sdata);
	Write_SysIniFile(_T("SYSTEM"), _T("CHAMBER_NO"), lpSystemInfo->m_sChamberNo);

	GetDlgItem(IDC_EDT_SY_EQP_NAME)->GetWindowText(sdata);
	lpSystemInfo->m_sEqpName.Format(_T("%s"), sdata);
	m_pApp->pCimNet->SetMachineName(lpSystemInfo->m_sEqpName);
	Write_SysIniFile(_T("SYSTEM"), _T("EQP_NAME"), lpSystemInfo->m_sEqpName);

	lpSystemInfo->m_nTempRecorderPort = m_cmbSyRecorderPort.GetCurSel();
	Write_SysIniFile(_T("SYSTEM"), _T("TEMP_RECORDER_PORT"), lpSystemInfo->m_nTempRecorderPort);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MES, EAS Setting
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	GetDlgItem(IDC_EDT_SY_MES_SERVICE_PORT)->GetWindowText(lpSystemInfo->m_sMesServicePort);
	Write_SysIniFile(_T("MES"), _T("MES_SERVICE_PORT"), lpSystemInfo->m_sMesServicePort);

	GetDlgItem(IDC_EDT_SY_MES_NETWORK)->GetWindowText(lpSystemInfo->m_sMesNetWork);
	Write_SysIniFile(_T("MES"), _T("MES_NETWORK"), lpSystemInfo->m_sMesNetWork);

	GetDlgItem(IDC_EDT_SY_MES_DAEMON_PORT)->GetWindowText(lpSystemInfo->m_sMesDaemonPort);
	Write_SysIniFile(_T("MES"), _T("MES_DAEMON_PORT"), lpSystemInfo->m_sMesDaemonPort);

	GetDlgItem(IDC_EDT_SY_MES_LOCAL_SUBJECT)->GetWindowText(lpSystemInfo->m_sMesLocalSubject);
	Write_SysIniFile(_T("MES"), _T("MES_LOCAL_SUBJECT"), lpSystemInfo->m_sMesLocalSubject);

	GetDlgItem(IDC_EDT_SY_MES_REMOTE_SUBJECT)->GetWindowText(lpSystemInfo->m_sMesRemoteSubject);
	Write_SysIniFile(_T("MES"), _T("MES_REMOTE_SUBJECT"), lpSystemInfo->m_sMesRemoteSubject);

	GetDlgItem(IDC_EDT_SY_EAS_SERVICE_PORT)->GetWindowText(lpSystemInfo->m_sEasServicePort);
	Write_SysIniFile(_T("EAS"), _T("EAS_SERVICE_PORT"), lpSystemInfo->m_sEasServicePort);

	GetDlgItem(IDC_EDT_SY_EAS_NETWORK)->GetWindowText(lpSystemInfo->m_sEasNetWork);
	Write_SysIniFile(_T("EAS"), _T("EAS_NETWORK"), lpSystemInfo->m_sEasNetWork);

	GetDlgItem(IDC_EDT_SY_EAS_DAEMON_PORT)->GetWindowText(lpSystemInfo->m_sEasDaemonPort);
	Write_SysIniFile(_T("EAS"), _T("EAS_DAEMON_PORT"), lpSystemInfo->m_sEasDaemonPort);

	GetDlgItem(IDC_EDT_SY_EAS_LOCAL_SUBJECT)->GetWindowText(lpSystemInfo->m_sEasLocalSubject);
	Write_SysIniFile(_T("EAS"), _T("EAS_LOCAL_SUBJECT"), lpSystemInfo->m_sEasLocalSubject);

	GetDlgItem(IDC_EDT_SY_EAS_REMOTE_SUBJECT)->GetWindowText(lpSystemInfo->m_sEasRemoteSubject);
	Write_SysIniFile(_T("EAS"), _T("EAS_REMOTE_SUBJECT"), lpSystemInfo->m_sEasRemoteSubject);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Refresh Time 설정
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	GetDlgItem(IDC_EDT_SY_REFRESH_AGING_STATUS)->GetWindowText(sdata);
	lpSystemInfo->m_fRefreshAgingStatusTime = (float)_tstof(sdata);
	Write_SysIniFile(_T("SYSTEM"), _T("REFRESH_AGING_STATUS_TIME"), lpSystemInfo->m_fRefreshAgingStatusTime);

	GetDlgItem(IDC_EDT_SY_REFRESH_POWER_MEASURE)->GetWindowText(sdata);
	lpSystemInfo->m_fRefreshPowerMeasureTime = (float)_tstof(sdata);
	Write_SysIniFile(_T("SYSTEM"), _T("REFRESH_POWER_MEASURE_TIME"), lpSystemInfo->m_fRefreshPowerMeasureTime);

	lpSystemInfo->m_nTempLogInterval = m_cmbSyTempLogInterval.GetCurSel();
	Write_SysIniFile(_T("SYSTEM"), _T("TEMP_LOG_INTERVAL"), lpSystemInfo->m_nTempLogInterval);

	lpSystemInfo->m_nSensingLogInterval = m_cmbSySensingLogInterval.GetCurSel();
	Write_SysIniFile(_T("SYSTEM"), _T("SENSING_LOG_INTERVAL"), lpSystemInfo->m_nSensingLogInterval);
}
