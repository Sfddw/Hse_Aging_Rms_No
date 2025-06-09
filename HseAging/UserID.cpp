// UserID.cpp: 구현 파일
//

#include "pch.h"
#include "HseAging.h"
#include "UserID.h"
#include "afxdialogex.h"


// CUserID 대화 상자

IMPLEMENT_DYNAMIC(CUserID, CDialog)

CUserID::CUserID(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_USER_ID, pParent)
{

}

CUserID::~CUserID()
{
}

void CUserID::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STT_UI_IMAGE, m_picUiImage);
	DDX_Control(pDX, IDC_BTN_UI_LOGIN, m_btnUiLogin);
}


BEGIN_MESSAGE_MAP(CUserID, CDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_UI_LOGIN, &CUserID::OnBnClickedBtnUiLogin)
END_MESSAGE_MAP()


// CUserID 메시지 처리기

BOOL CUserID::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	lpSystemInfo = m_pApp->GetSystemInfo();
	lpInspWorkInfo = m_pApp->GetInspWorkInfo();

	m_pApp->Gf_writeMLog(_T("<TEST> 'USER ID' Dialog Open"));

	Lf_InitLocalValue();
	Lf_InitFontset();
	Lf_InitColorBrush();
	Lf_InitDialogDesign();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CUserID::OnDestroy()
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

BOOL CUserID::PreTranslateMessage(MSG* pMsg)
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
			case VK_RETURN:
				if(Lf_confirmUserID() == FALSE)
				{
					GetDlgItem(IDC_EDT_UI_USERID)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_UI_LOGIN)->EnableWindow(TRUE);
				}
				return 1;
			case VK_ESCAPE:
				return 1;
			case VK_SPACE:
				return 1;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CUserID::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
			if (pWnd->GetDlgCtrlID() == IDC_STT_UI_USERID_TITLE)
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


void CUserID::OnPaint()
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


void CUserID::OnBnClickedBtnUiLogin()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (Lf_confirmUserID() == FALSE)
	{
		GetDlgItem(IDC_EDT_UI_USERID)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_UI_LOGIN)->EnableWindow(TRUE);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUserID::Lf_InitLocalValue()
{

}

void CUserID::Lf_InitFontset()
{
	m_Font[0].CreateFont(150, 70, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[1].CreateFont(44, 20, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_UI_USERID_TITLE)->SetFont(&m_Font[1]);

	m_Font[2].CreateFont(32, 14, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_EDT_UI_USERID)->SetFont(&m_Font[2]);

	m_Font[3].CreateFont(26, 12, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_BTN_UI_LOGIN)->SetFont(&m_Font[3]);

	m_Font[4].CreateFont(19, 8, 0, 0, FW_BOLD, TRUE, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[5].CreateFont(16, 7, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

void CUserID::Lf_InitColorBrush()
{
	// 각 Control의 COLOR 설정을 위한 Brush를 Setting 한다.
	m_Brush[COLOR_IDX_BLACK].CreateSolidBrush(COLOR_BLACK);
	m_Brush[COLOR_IDX_WHITE].CreateSolidBrush(COLOR_WHITE);
	m_Brush[COLOR_IDX_RED].CreateSolidBrush(COLOR_RED);
	m_Brush[COLOR_IDX_GREEN].CreateSolidBrush(COLOR_GREEN);
	m_Brush[COLOR_IDX_BLUE].CreateSolidBrush(COLOR_BLUE);
	m_Brush[COLOR_IDX_SEABLUE].CreateSolidBrush(COLOR_SEABLUE);
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
	m_Brush[COLOR_IDX_GRAY128].CreateSolidBrush(COLOR_GRAY128);
	m_Brush[COLOR_IDX_GRAY224].CreateSolidBrush(COLOR_GRAY224);
	m_Brush[COLOR_IDX_BLUISH].CreateSolidBrush(COLOR_BLUISH);
	m_Brush[COLOR_IDX_DARK_BLUE].CreateSolidBrush(COLOR_DARK_BLUE);
	m_Brush[COLOR_IDX_DARK_NAVY].CreateSolidBrush(COLOR_DARK_NAVY);
}

void CUserID::Lf_InitDialogDesign()
{
	CBitmap m_Bit;

	m_Bit.LoadBitmap(IDB_BMP_USER_IMG);
	m_picUiImage.SetBitmap(m_Bit);
	m_Bit.Detach();

	m_btnUiLogin.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_USER_BTN));
}

BOOL CUserID::Lf_confirmUserID()
{
	CString sLog, m_sUserId;

	GetDlgItem(IDC_EDT_UI_USERID)->GetWindowText(m_sUserId);
	sLog.Format(_T("<USER> USER ID INPUT : %s"), m_sUserId);
	m_pApp->Gf_writeMLog(sLog);

	GetDlgItem(IDC_BTN_UI_LOGIN)->EnableWindow(FALSE);

	m_pApp->m_bUserIdAdmin = FALSE;
	if (!m_sUserId.CompareNoCase(_T("PM")))
	{
		m_sUserId.MakeUpper();
		m_pApp->m_sUserID = m_sUserId;
		m_pApp->m_sUserName.Empty();
		m_pApp->m_bUserIdAdmin = TRUE;
		lpInspWorkInfo->m_nConnectInfo[CONNECT_MES] = FALSE;
	}
	else
	{
		if (m_sUserId.GetLength() < 5)
		{
			m_pApp->Gf_ShowMessageBox(_T("USER ID WRONG. Please input again."));
			return FALSE;
		}

		//MES Connect
		if (m_pApp->m_bIsGmesConnect == FALSE) {

			if (m_pApp->Gf_gmesConnect(SERVER_MES) == FALSE)
			{
				m_pApp->Gf_ShowMessageBox(_T("MES Server Connect Fail"));

				CString sLog;
				sLog.Format(_T("<GMES> MES Server Connection Fail"));
				m_pApp->Gf_writeMLog(sLog);

				return FALSE;
			}
		}
		//EAS Connect
		if (m_pApp->m_bIsEasConnect == FALSE)
		{
			if (m_pApp->Gf_gmesConnect(SERVER_EAS) == FALSE)
			{
				m_pApp->Gf_ShowMessageBox(_T("EAS Server Connect Fail"));

				CString sLog;
				sLog.Format(_T("<EAS> EAS Server Connection Fail"));
				m_pApp->Gf_writeMLog(sLog);

				//return FALSE;
			}
			lpInspWorkInfo->m_nConnectInfo[CONNECT_EAS] = TRUE;
		}

		m_pApp->m_bUserIdAdmin = FALSE;

		if (m_pApp->m_bIsSendEAYT == FALSE)
		{
			if (m_pApp->Gf_gmesSendHost(HOST_EAYT, NULL, NULL, NULL) == FALSE)
			{
				return FALSE;
			}
			m_pApp->m_bIsSendEAYT = TRUE;
		}

		m_pApp->pCimNet->SetUserId(m_sUserId);
		if (m_pApp->Gf_gmesSendHost(HOST_UCHK, NULL, NULL, NULL) == FALSE)
		{
			return FALSE;
		}

		if (m_pApp->Gf_gmesSendHost(HOST_EDTI, NULL, NULL, NULL) == FALSE)
		{
			return FALSE;
		}

		m_pApp->m_sUserID.Format(_T("%s"), m_sUserId);
		m_pApp->pCimNet->SetUserId(m_sUserId);

		lpInspWorkInfo->m_nConnectInfo[CONNECT_MES] = TRUE;

		CString sLog;
		sLog.Format(_T("<USER> Login User ID : %s"), m_sUserId);
		m_pApp->Gf_writeMLog(sLog);
	}

	CDialog::OnOK();
	return TRUE;
}

