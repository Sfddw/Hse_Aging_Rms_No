// AutoFirmware.cpp: 구현 파일
//

#include "pch.h"
#include "HseAging.h"
#include "AutoFirmware.h"
#include "afxdialogex.h"
#include "MessageError.h"
#include "MessageQuestion.h"

#pragma comment(lib, "UxTheme.lib")


// CAutoFirmware 대화 상자

IMPLEMENT_DYNAMIC(CAutoFirmware, CDialog)

CAutoFirmware::CAutoFirmware(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_AUTO_FIRMWARE, pParent)
{
	m_pDefaultFont = new CFont();
	m_pDefaultFont->CreateFont(15, 6, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

CAutoFirmware::~CAutoFirmware()
{
	if (m_pDefaultFont != NULL)
	{
		delete m_pDefaultFont;
	}
}

void CAutoFirmware::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MBC_AF_FILE_LOAD, m_mbcAfFileLoad);
	DDX_Control(pDX, IDC_MBC_AF_START, m_mbcAfStart);
	DDX_Control(pDX, IDC_CTR_AF_PROGRESS, m_ctrAfProgress);
	DDX_Control(pDX, IDC_MBC_AF_VER_READ, m_mbcAfVerRead);
	DDX_Control(pDX, IDC_MBC_AF_CANCEL, m_mbcAfCancel);
	DDX_Control(pDX, IDC_CHK_AF_RACK_ALL, m_chkAfRackAll);
	DDX_Control(pDX, IDC_CHK_AF_RACK_1, m_chkAfRack1);
	DDX_Control(pDX, IDC_CHK_AF_RACK_2, m_chkAfRack2);
	DDX_Control(pDX, IDC_CHK_AF_RACK_3, m_chkAfRack3);
	DDX_Control(pDX, IDC_CHK_AF_RACK_4, m_chkAfRack4);
	DDX_Control(pDX, IDC_CHK_AF_RACK_5, m_chkAfRack5);
	DDX_Control(pDX, IDC_CHK_AF_RACK_6, m_chkAfRack6);
	DDX_Control(pDX, IDC_CHK_AF_DIO_BOARD, m_chkAfDioBoard);
}


BEGIN_MESSAGE_MAP(CAutoFirmware, CDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_MBC_AF_FILE_LOAD, &CAutoFirmware::OnBnClickedMbcAfFileLoad)
	ON_BN_CLICKED(IDC_MBC_AF_START, &CAutoFirmware::OnBnClickedMbcAfStart)
	ON_BN_CLICKED(IDC_MBC_AF_VER_READ, &CAutoFirmware::OnBnClickedMbcAfVerRead)
	ON_BN_CLICKED(IDC_MBC_AF_CANCEL, &CAutoFirmware::OnBnClickedMbcAfCancel)
	ON_BN_CLICKED(IDC_CHK_AF_RACK_ALL, &CAutoFirmware::OnBnClickedChkAfRackAll)
	ON_BN_CLICKED(IDC_CHK_AF_RACK_1, &CAutoFirmware::OnBnClickedChkAfRack1)
	ON_BN_CLICKED(IDC_CHK_AF_RACK_2, &CAutoFirmware::OnBnClickedChkAfRack2)
	ON_BN_CLICKED(IDC_CHK_AF_RACK_3, &CAutoFirmware::OnBnClickedChkAfRack3)
	ON_BN_CLICKED(IDC_CHK_AF_RACK_4, &CAutoFirmware::OnBnClickedChkAfRack4)
	ON_BN_CLICKED(IDC_CHK_AF_RACK_5, &CAutoFirmware::OnBnClickedChkAfRack5)
	ON_BN_CLICKED(IDC_CHK_AF_RACK_6, &CAutoFirmware::OnBnClickedChkAfRack6)
	ON_BN_CLICKED(IDC_CHK_AF_DIO_BOARD, &CAutoFirmware::OnBnClickedChkAfDioBoard)
END_MESSAGE_MAP()


// CAutoFirmware 메시지 처리기


BOOL CAutoFirmware::OnInitDialog()
{
	CDialog::OnInitDialog();
	lpModelInfo = m_pApp->GetModelInfo();
	lpSystemInfo = m_pApp->GetSystemInfo();
	lpInspWorkInfo = m_pApp->GetInspWorkInfo();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_pApp->Gf_writeMLog(_T("<TEST> 'Auto Firmware' Dialog Open"));

	// Dialog의 기본 FONT 설정.
	SendMessageToDescendants(WM_SETFONT, (WPARAM)m_pDefaultFont->GetSafeHandle(), 1, TRUE, FALSE);

	Lf_InitLocalValue();
	Lf_InitFontset();
	Lf_InitColorBrush();
	Lf_InitDialogDesign();
	nTestCnt = 0;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CAutoFirmware::OnDestroy()
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


BOOL CAutoFirmware::PreTranslateMessage(MSG* pMsg)
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


HBRUSH CAutoFirmware::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
			if (pWnd->GetDlgCtrlID() == IDC_STT_AF_TITLE)
			{
				pDC->SetBkColor(COLOR_DARK_NAVY);
				pDC->SetTextColor(COLOR_WHITE);
				return m_Brush[COLOR_IDX_DARK_NAVY];
			}
			if ((pWnd->GetDlgCtrlID() == IDC_GRP_AF_RACK_OPERATION)
				|| (pWnd->GetDlgCtrlID() == IDC_GRP_AF_DIO_OPERATION)
				|| (pWnd->GetDlgCtrlID() == IDC_GRP_AF_FIRMWARE_OPERATION)
				|| (pWnd->GetDlgCtrlID() == IDC_GRP_AF_VERSION_OPERATION)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_AF_PERCENT)
				)
			{
				pDC->SetBkColor(COLOR_GRAY192);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_GRAY192];
			}
			if ((pWnd->GetDlgCtrlID() == IDC_STT_AF_FILE_PATH)
				)
			{
				pDC->SetBkColor(COLOR_ITEM_TITLE);
				pDC->SetTextColor(COLOR_WHITE);
				return m_Brush[COLOR_IDX_ITEM_TITLE];
			}
			if (pWnd->GetDlgCtrlID() == IDC_STT_AF_STATUS)
			{
				pDC->SetBkColor(COLOR_BLACK);
				pDC->SetTextColor(COLOR_CYAN);
				return m_Brush[COLOR_IDX_BLACK];
			}
			if (pWnd->GetDlgCtrlID() == IDC_CHK_AF_RACK_ALL)
			{
				if ((m_nTargetBoard == FW_TARGET_PG) && (m_nSelRack == MAX_RACK))
					pDC->SetTextColor(COLOR_BLUE);
				else
					pDC->SetTextColor(COLOR_BLACK);
				pDC->SetBkColor(COLOR_GRAY192);
				return m_Brush[COLOR_IDX_GRAY192];
			}
			if (pWnd->GetDlgCtrlID() == IDC_CHK_AF_RACK_1)
			{
				if ((m_nTargetBoard == FW_TARGET_PG) && (m_nSelRack == RACK_1))
					pDC->SetTextColor(COLOR_BLUE);
				else
					pDC->SetTextColor(COLOR_BLACK);
				pDC->SetBkColor(COLOR_GRAY192);
				return m_Brush[COLOR_IDX_GRAY192];
			}
			if (pWnd->GetDlgCtrlID() == IDC_CHK_AF_RACK_2)
			{
				if ((m_nTargetBoard == FW_TARGET_PG) && (m_nSelRack == RACK_2))
					pDC->SetTextColor(COLOR_BLUE);
				else
					pDC->SetTextColor(COLOR_BLACK);
				pDC->SetBkColor(COLOR_GRAY192);
				return m_Brush[COLOR_IDX_GRAY192];
			}
			if (pWnd->GetDlgCtrlID() == IDC_CHK_AF_RACK_3)
			{
				if ((m_nTargetBoard == FW_TARGET_PG) && (m_nSelRack == RACK_3))
					pDC->SetTextColor(COLOR_BLUE);
				else
					pDC->SetTextColor(COLOR_BLACK);
				pDC->SetBkColor(COLOR_GRAY192);
				return m_Brush[COLOR_IDX_GRAY192];
			}
			if (pWnd->GetDlgCtrlID() == IDC_CHK_AF_RACK_4)
			{
				if ((m_nTargetBoard == FW_TARGET_PG) && (m_nSelRack == RACK_4))
					pDC->SetTextColor(COLOR_BLUE);
				else
					pDC->SetTextColor(COLOR_BLACK);
				pDC->SetBkColor(COLOR_GRAY192);
				return m_Brush[COLOR_IDX_GRAY192];
			}
			if (pWnd->GetDlgCtrlID() == IDC_CHK_AF_RACK_5)
			{
				if ((m_nTargetBoard == FW_TARGET_PG) && (m_nSelRack == RACK_5))
					pDC->SetTextColor(COLOR_BLUE);
				else
					pDC->SetTextColor(COLOR_BLACK);
				pDC->SetBkColor(COLOR_GRAY192);
				return m_Brush[COLOR_IDX_GRAY192];
			}
			if (pWnd->GetDlgCtrlID() == IDC_CHK_AF_RACK_6)
			{
				if ((m_nTargetBoard == FW_TARGET_PG) && (m_nSelRack == RACK_6))
					pDC->SetTextColor(COLOR_BLUE);
				else
					pDC->SetTextColor(COLOR_BLACK);
				pDC->SetBkColor(COLOR_GRAY192);
				return m_Brush[COLOR_IDX_GRAY192];
			}
			if (pWnd->GetDlgCtrlID() == IDC_CHK_AF_DIO_BOARD)
			{
				if (m_nTargetBoard == FW_TARGET_DIO)		pDC->SetTextColor(COLOR_BLUE);
				else										pDC->SetTextColor(COLOR_BLACK);
				pDC->SetBkColor(COLOR_GRAY192);
				return m_Brush[COLOR_IDX_GRAY192];
			}

			break;
	}
	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.
	return hbr;
}


void CAutoFirmware::OnPaint()
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


void CAutoFirmware::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == 1)
	{
		if (nTestCnt++ > 100)
			nTestCnt = 0;

		m_ctrAfProgress.SetPos(nTestCnt);
	}
	CDialog::OnTimer(nIDEvent);
}


void CAutoFirmware::OnBnClickedChkAfRackAll()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nTargetBoard = FW_TARGET_PG;
	m_nSelRack = MAX_RACK;
	Lf_setCheckFwRack(m_nSelRack);
}


void CAutoFirmware::OnBnClickedChkAfRack1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nTargetBoard = FW_TARGET_PG;
	m_nSelRack = RACK_1;
	Lf_setCheckFwRack(m_nSelRack);
}


void CAutoFirmware::OnBnClickedChkAfRack2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nTargetBoard = FW_TARGET_PG;
	m_nSelRack = RACK_2;
	Lf_setCheckFwRack(m_nSelRack);
}


void CAutoFirmware::OnBnClickedChkAfRack3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nTargetBoard = FW_TARGET_PG;
	m_nSelRack = RACK_3;
	Lf_setCheckFwRack(m_nSelRack);
}


void CAutoFirmware::OnBnClickedChkAfRack4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nTargetBoard = FW_TARGET_PG;
	m_nSelRack = RACK_4;
	Lf_setCheckFwRack(m_nSelRack);
}


void CAutoFirmware::OnBnClickedChkAfRack5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nTargetBoard = FW_TARGET_PG;
	m_nSelRack = RACK_5;
	Lf_setCheckFwRack(m_nSelRack);
}


void CAutoFirmware::OnBnClickedChkAfRack6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nTargetBoard = FW_TARGET_PG;
	m_nSelRack = RACK_6;
	Lf_setCheckFwRack(m_nSelRack);
}

void CAutoFirmware::OnBnClickedChkAfDioBoard()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nTargetBoard = FW_TARGET_DIO;
	m_chkAfDioBoard.SetCheck(TRUE);		// DIO Board 선택 시 항상 Check 표시되도록 한다.
	Lf_setCheckFwRack(MAX_RACK + 1);	// 모든 RACK check 버턴 해제, +1을하면 해당 RACK이 없어 모두 해제된다.
}


void CAutoFirmware::OnBnClickedMbcAfFileLoad()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_loadFirmwareFile();
}


void CAutoFirmware::OnBnClickedMbcAfStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sLog;

	if (Lf_checkRackSelect() == FALSE)
		return;

	if (GetDlgItem(IDC_STT_AF_FILE_PATH)->GetWindowTextLength() == 0)
	{
		AfxMessageBox(_T("Firmware file not found."), MB_ICONERROR);
		return;
	}

	// Disable Button
	Lf_disableButton();

	// Firmware Download START
	if (m_nTargetBoard == FW_TARGET_PG)
	{
		if (m_nSelRack == MAX_RACK)
		{
			for (int rack = RACK_1; rack < MAX_RACK; rack++)
			{
				sLog.Format(_T("<FIRMWARE] 'RACK-%d' Firmware Download Start"), (rack + 1));
				m_pApp->Gf_writeMLog(sLog);

				if (Lf_isRackEthConnect(rack) == FALSE)
					continue;

				if (Lf_firmwareDownloadStartPG(rack) == FALSE)
				{
					CMessageQuestion msg_dlg;
					msg_dlg.m_strQMessage = _T("Next Rack Firmware Download Continue ?");
					msg_dlg.m_strLButton = _T("Continue");
					msg_dlg.m_strRButton = _T("Stop");
					if (msg_dlg.DoModal() == IDCANCEL)
					{
						break;
					}
				}
			}
		}
		else
		{
			sLog.Format(_T("<FIRMWARE> 'RACK-%d' Firmware Download Start"), (m_nSelRack + 1));
			m_pApp->Gf_writeMLog(sLog);

			Lf_firmwareDownloadStartPG(m_nSelRack);
		}
	}
	else if (m_nTargetBoard == FW_TARGET_DIO)
	{
 		sLog.Format(_T("<FIRMWARE> 'DIO Board' Firmware Download Start"));
 		m_pApp->Gf_writeMLog(sLog);
 
 		Lf_firmwareDownloadStartDIO();
	}

	// Firmware Download 완료 후 Board 안정화시간 1Sec Delay.
	delayMs(1000);

	// Enable Button
	Lf_enableButton();

	CString sdata;
	sdata.Format(_T("Firmware Download Complete"));
	GetDlgItem(IDC_STT_AF_STATUS)->SetWindowText(sdata);
}


void CAutoFirmware::OnBnClickedMbcAfVerRead()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (Lf_checkRackSelect() == FALSE)
		return;

	Lf_readFirmwareVersion();
}

void CAutoFirmware::OnBnClickedMbcAfCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialog::OnCancel();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAutoFirmware::Lf_InitLocalValue()
{
	m_pApp->m_nDownloadReadyAckCount = 0;					//Firmware Download ACK Receive Count 초기화.	

	m_nFirmwareDataLen = 0;									//Firmware File Length 초기화
	m_pFirmwareData = new BYTE[MAX_FILE_SIZE];				//Firmware File Buff Memory 1M 할당.
	memset(m_pFirmwareData, NULL, MAX_FILE_SIZE);			//Buff Memory 초기화

	m_nSelRack = MAX_RACK;
	m_chkAfRackAll.SetCheck(TRUE);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	int rack, i;

	rack = RACK_1;
	i = 0;
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK1_LAYER1);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK1_LAYER2);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK1_LAYER3);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK1_LAYER4);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK1_LAYER5);

	rack = RACK_2;
	i = 0;
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK2_LAYER1);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK2_LAYER2);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK2_LAYER3);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK2_LAYER4);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK2_LAYER5);

	rack = RACK_3;
	i = 0;
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK3_LAYER1);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK3_LAYER2);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK3_LAYER3);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK3_LAYER4);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK3_LAYER5);

	rack = RACK_4;
	i = 0;
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK4_LAYER1);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK4_LAYER2);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK4_LAYER3);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK4_LAYER4);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK4_LAYER5);

	rack = RACK_5;
	i = 0;
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK5_LAYER1);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK5_LAYER2);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK5_LAYER3);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK5_LAYER4);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK5_LAYER5);

	rack = RACK_6;
	i = 0;
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK6_LAYER1);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK6_LAYER2);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK6_LAYER3);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK6_LAYER4);
	m_pStaticVer[rack][i++] = (CStatic*)GetDlgItem(IDC_STT_AF_VERSION_RACK6_LAYER5);
}

void CAutoFirmware::Lf_InitFontset()
{
	m_Font[0].CreateFont(150, 70, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[1].CreateFont(44, 20, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_AF_TITLE)->SetFont(&m_Font[1]);

	m_Font[2].CreateFont(32, 14, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[3].CreateFont(26, 12, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[4].CreateFont(19, 8, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_GRP_AF_RACK_OPERATION)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_GRP_AF_DIO_OPERATION)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_GRP_AF_FIRMWARE_OPERATION)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_GRP_AF_VERSION_OPERATION)->SetFont(&m_Font[4]);

	GetDlgItem(IDC_CHK_AF_RACK_ALL)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_CHK_AF_RACK_1)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_CHK_AF_RACK_2)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_CHK_AF_RACK_3)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_CHK_AF_RACK_4)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_CHK_AF_RACK_5)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_CHK_AF_RACK_6)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_CHK_AF_DIO_BOARD)->SetFont(&m_Font[4]);

	GetDlgItem(IDC_MBC_AF_FILE_LOAD)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_MBC_AF_START)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_MBC_AF_VER_READ)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_MBC_AF_CANCEL)->SetFont(&m_Font[4]);

	GetDlgItem(IDC_STT_AF_FILE_PATH)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_STT_AF_STATUS)->SetFont(&m_Font[4]);

	m_Font[5].CreateFont(16, 7, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

void CAutoFirmware::Lf_InitColorBrush()
{
	// 각 Control의 COLOR 설정을 위한 Brush를 Setting 한다.
	m_Brush[COLOR_IDX_BLACK].CreateSolidBrush(COLOR_BLACK);
	m_Brush[COLOR_IDX_WHITE].CreateSolidBrush(COLOR_WHITE);
	m_Brush[COLOR_IDX_RED].CreateSolidBrush(COLOR_RED);
	m_Brush[COLOR_IDX_GREEN].CreateSolidBrush(COLOR_GREEN);
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
	m_Brush[COLOR_IDX_DARK_BG].CreateSolidBrush(COLOR_DARK_BG);
	m_Brush[COLOR_IDX_ITEM_HEAD].CreateSolidBrush(COLOR_ITEM_HEAD);
	m_Brush[COLOR_IDX_ITEM_TITLE].CreateSolidBrush(COLOR_ITEM_TITLE);
}

void CAutoFirmware::Lf_InitDialogDesign()
{
	SetWindowTheme(GetDlgItem(IDC_CHK_AF_RACK_ALL)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_AF_RACK_1)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_AF_RACK_2)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_AF_RACK_3)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_AF_RACK_4)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_AF_RACK_5)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_AF_RACK_6)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_AF_DIO_BOARD)->GetSafeHwnd(), L"", L"");

	SetWindowTheme(GetDlgItem(IDC_GRP_AF_RACK_OPERATION)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_GRP_AF_DIO_OPERATION)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_GRP_AF_FIRMWARE_OPERATION)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_GRP_AF_VERSION_OPERATION)->GetSafeHwnd(), L"", L"");
}


void CAutoFirmware::Lf_disableButton()
{
	GetDlgItem(IDC_MBC_AF_FILE_LOAD)->EnableWindow(FALSE);
	GetDlgItem(IDC_MBC_AF_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_MBC_AF_VER_READ)->EnableWindow(FALSE);

	GetDlgItem(IDC_CHK_AF_RACK_ALL)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHK_AF_RACK_1)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHK_AF_RACK_2)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHK_AF_RACK_3)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHK_AF_RACK_4)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHK_AF_RACK_5)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHK_AF_RACK_6)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHK_AF_DIO_BOARD)->EnableWindow(FALSE);
}

void CAutoFirmware::Lf_enableButton()
{
	GetDlgItem(IDC_MBC_AF_FILE_LOAD)->EnableWindow(TRUE);
	GetDlgItem(IDC_MBC_AF_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_MBC_AF_VER_READ)->EnableWindow(TRUE);

	GetDlgItem(IDC_CHK_AF_RACK_ALL)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHK_AF_RACK_1)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHK_AF_RACK_2)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHK_AF_RACK_3)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHK_AF_RACK_4)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHK_AF_RACK_5)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHK_AF_RACK_6)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHK_AF_DIO_BOARD)->EnableWindow(TRUE);
}

void CAutoFirmware::Lf_clearVersionStatus(int rack)
{
	if (m_nTargetBoard == FW_TARGET_PG)
	{
		if (rack == MAX_RACK)
		{
			for (int m_ra = 0; m_ra < MAX_RACK; m_ra++)
			{
				for (int layer = 0; layer < MAX_LAYER; layer++)
				{
					if (m_pStaticVer[m_ra][layer] != NULL)
					{
						m_pStaticVer[m_ra][layer]->SetWindowText(_T(""));
						lpInspWorkInfo->m_sMainFWVersion[m_ra][layer].Empty();
					}
				}
			}
		}
		else
		{
			for (int layer = 0; layer < MAX_LAYER; layer++)
			{
				if (m_pStaticVer[rack][layer] != NULL)
				{
					m_pStaticVer[rack][layer]->SetWindowText(_T(""));
					lpInspWorkInfo->m_sMainFWVersion[rack][layer].Empty();
				}
			}
		}
	}
	else if (m_nTargetBoard == FW_TARGET_DIO)
	{
		GetDlgItem(IDC_STT_AF_VERSION_DIO_BOARD)->SetWindowText(_T(""));
		lpInspWorkInfo->m_sDioFWVersion.Empty();
	}
}

BOOL CAutoFirmware::Lf_checkRackSelect()
{
	if ((m_chkAfRackAll.GetCheck() == 0)
		&& (m_chkAfRack1.GetCheck() == 0) && (m_chkAfRack2.GetCheck() == 0) && (m_chkAfRack3.GetCheck() == 0)
		&& (m_chkAfRack4.GetCheck() == 0) && (m_chkAfRack5.GetCheck() == 0) && (m_chkAfRack6.GetCheck() == 0)
		&& (m_chkAfDioBoard.GetCheck() == 0)
		)
	{
		CMessageError err_dlg;
		err_dlg.m_strEMessage = _T("Have not selected target. Select a target.");
		err_dlg.DoModal();
		return FALSE;
	}

	return TRUE;
}

void CAutoFirmware::Lf_setCheckFwRack(int rack)
{
	m_chkAfRackAll.SetCheck(FALSE);
	m_chkAfRack1.SetCheck(FALSE);
	m_chkAfRack2.SetCheck(FALSE);
	m_chkAfRack3.SetCheck(FALSE);
	m_chkAfRack4.SetCheck(FALSE);
	m_chkAfRack5.SetCheck(FALSE);
	m_chkAfRack6.SetCheck(FALSE);
	if (m_nTargetBoard == FW_TARGET_PG)
		m_chkAfDioBoard.SetCheck(FALSE);

	if (rack == MAX_RACK)	m_chkAfRackAll.SetCheck(TRUE);
	if (rack == RACK_1)		m_chkAfRack1.SetCheck(TRUE);
	if (rack == RACK_2)		m_chkAfRack2.SetCheck(TRUE);
	if (rack == RACK_3)		m_chkAfRack3.SetCheck(TRUE);
	if (rack == RACK_4)		m_chkAfRack4.SetCheck(TRUE);
	if (rack == RACK_5)		m_chkAfRack5.SetCheck(TRUE);
	if (rack == RACK_6)		m_chkAfRack6.SetCheck(TRUE);
}

void CAutoFirmware::Lf_loadFirmwareFile()
{
	CString m_sFirmwarePath;
	TCHAR szFilePath[1024] = { 0, };

	GetCurrentDirectory(sizeof(szFilePath), szFilePath);
	CFileDialog m_ldFile(TRUE, _T("hex|*"), NULL, OFN_READONLY, _T("Intel Hex File(*.Hex;*.a90)|*.HEX;*.a9|All File(*.*)|*.*|"));
	if (m_ldFile.DoModal() == IDOK)
	{
		SetCurrentDirectory(szFilePath);

		m_sFirmwarePath = m_ldFile.GetPathName();

		GetDlgItem(IDC_STT_AF_FILE_PATH)->SetWindowText(m_sFirmwarePath);
		Lf_readFirmwareFile(m_sFirmwarePath);

		UpdateData(FALSE);

		CString sLog;
		sLog.Format(_T("<FIRMWARE> Load File - %s"), m_sFirmwarePath);
		m_pApp->Gf_writeMLog(sLog);
	}
}

void CAutoFirmware::Lf_readFirmwareFile(CString strFilePath)
{
	CStdioFile* pFile;
	pFile = new CStdioFile();

	// File을 Open한다.
	if (pFile->Open(strFilePath, CStdioFile::modeRead | CStdioFile::typeText) == NULL)
	{
		AfxMessageBox(_T("File Open Fail !!!"), MB_OK | MB_ICONERROR);
		delete pFile;
		return;
	}

	// Firmware Data를 저장할 Buff를 초기화 한다.
	m_nFirmwareDataLen = 0;								//Firmware File Length 초기화
	memset(m_pFirmwareData, 0xFF, MAX_FILE_SIZE);			//Firmware Buff Memory 초기화

	// Line이 Null일때까지 읽는다.
	CString lineString;
	CString recStart, recData;
	int recLength, recOffset, recType, recChksum;
	while (pFile->ReadString(lineString) == TRUE)
	{
		recStart = lineString.Left(1);
		recChksum = _tcstol(lineString.Right(2), NULL, 16);

		// 레코드의 시작문자(:)를 확인한다.
		if (recStart == ":")
		{
			recLength = _tcstol(lineString.Mid(1, 2), NULL, 16);
			recOffset = _tcstol(lineString.Mid(3, 4), NULL, 16);
			recType = _tcstol(lineString.Mid(7, 2), NULL, 16);
			recData = lineString.Mid(9, (recLength * 2));

			//00 - Data record 
			//01 - End of file record 
			//02 - Extended segment address record 
			//03 - Start segment address record 
			//04 - Extended linear address record 
			//05 - Start linear address record 
			if (recType == 0x01)
			{	// 파일의 끝이므로 종료.
				break;
			}
			else if ((recType == 0x02) || (recType == 0x03) || (recType == 0x04) || (recType == 0x05))
			{	// Address Record는 Data에 포함하지 않는다.
			}
			else
			{	// Data Record를 Parsing한다.
#if 1			// 2025-01-06 PDH. Address 기준으로 Data Parsing 하도록 알고리즘 변경
				Lf_parseDataRecord(recData, &m_pFirmwareData[recOffset]);
				m_nFirmwareDataLen = recOffset + recLength;
#else
				Lf_parseDataRecord(recData, &m_pFirmwareData[m_nFirmwareDataLen]);
				m_nFirmwareDataLen += recLength;
#endif
			}
		}
		else
		{
			AfxMessageBox(_T("Not the Intel hex file type."), MB_OK | MB_ICONERROR);
			// Data Buff Memory를 초기화 한다.
			ZeroMemory(m_pFirmwareData, MAX_FILE_SIZE);
			// Data Length를 초기화 한다.
			m_nFirmwareDataLen = 0;
		}
	}

	// File을 닫는다.
	pFile->Close();
	delete pFile;
}

void CAutoFirmware::Lf_parseDataRecord(CString strRecord, BYTE* pData)
{
	int nLen = 0;
	int point = 0;

	nLen = strRecord.GetLength() / 2;
	for (int i = 0; i < nLen; i++)
	{
		*pData = (BYTE)_tcstol(strRecord.Mid(point, 2), NULL, 16);

		point += 2;
		pData++;
	}
}

void CAutoFirmware::Lf_readyInitialize()
{
	m_pApp->m_nDownloadReadyAckCount = 0;				//Firmware Download ACK Receive Count 초기화.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CAutoFirmware::Lf_firmwareDownloadStartPG(int rack)
{
	CMessageError err_dlg;
	CString sdata;

	sdata.Format(_T("RACK-%d  Firmware Downloading ..."), rack + 1);
	GetDlgItem(IDC_STT_AF_STATUS)->SetWindowText(sdata);

	// Progress 초기화
	m_ctrAfProgress.SetPos(0);
	GetDlgItem(IDC_STT_AF_PERCENT)->SetWindowText(_T("(0%)"));

	// Channel Version Display 초기화
	Lf_clearVersionStatus(rack);

	// Step1. Download Ready Check
	if (Lf_checkDownloadReady1PG(rack) == FALSE)
	{
		err_dlg.m_strEMessage.Format(_T("xxx  [RACK-%d] Firmware Download Fail - Ready Check  xxx"), (rack + 1));
		err_dlg.DoModal();
		goto ERR_EXCEPT;
	}

	// Step2. Download Ready Delay
	delayMs(2000);

	// Step3. Download Sequence Set
	m_pApp->m_nDownloadReadyAckCount = 0;
	Lf_checkDownloadReady2PG(rack);

	// Step4. Download Start - Send Raw Data
	delayMs(300);
	if (Lf_sendFirmwareFilePG(rack) == FALSE)
	{
		err_dlg.m_strEMessage.Format(_T("xxx  [RACK-%d] Firmware Download Fail - Data Download  xxx"), (rack + 1));
		err_dlg.DoModal();
		goto ERR_EXCEPT;
	}

	// Step5. Download Complete Check
	delayMs(100);
	if (Lf_sendDownloadCompletePG(rack) == FALSE)
	{
		err_dlg.m_strEMessage.Format(_T("xxx  [RACK-d] Firmware Download Fail - Complete Check  xxx"), (rack + 1));
		err_dlg.DoModal();
		goto ERR_EXCEPT;
	}

	// Step6. Download Initialize & Ready
	Lf_readyInitialize();

	m_ctrAfProgress.SetPos(100);
	GetDlgItem(IDC_STT_AF_PERCENT)->SetWindowText(_T("(100%)"));
	return TRUE;

ERR_EXCEPT:
	// Error Exception. Initialize.
	Lf_readyInitialize();
	return FALSE;
}

BOOL CAutoFirmware::Lf_checkDownloadReady1PG(int rack)
{
	// App 영역에서 Boot 영역으로 진입하기 위한 Ready Command 이다.
	DWORD sTick, eTick;

	sTick = ::GetTickCount();
	while (1)
	{
		m_pApp->m_nDownloadCountUp = TRUE;
		m_pApp->pCommand->Gf_setGoToBootSection(rack);
		delayMs(30);

		if (m_pApp->m_nDownloadReadyAckCount > 10)
		{
			return TRUE;
		}

		eTick = ::GetTickCount();
		if ((eTick - sTick) > 10000)
			break;

		ProcessMessage();
	}

	return FALSE;
}

BOOL CAutoFirmware::Lf_checkDownloadReady2PG(int rack)
{
	// Boot 영역에서 Download Sequence로 진입하기 위한 Ready Command 이다.
	DWORD sTick, eTick;

	sTick = ::GetTickCount();
	while (1)
	{
		m_pApp->m_nDownloadCountUp = TRUE;

		m_pApp->pCommand->Gf_setGoToBootSection(rack);

		delayMs(30);

		if (m_pApp->m_nDownloadReadyAckCount > 5)
		{
			return TRUE;
		}

		eTick = ::GetTickCount();
		if ((eTick - sTick) > 3000)
			break;

		ProcessMessage();
	}

	return FALSE;
}

BOOL CAutoFirmware::Lf_sendFirmwareFilePG(int rack)
{
	BOOL bRet = FALSE;
	int startAddr = 0;
	int packetLen = 0;
	char szpacket[4096] = { 0, };

	BOOL bFirstTime = TRUE;
	while (1)
	{
		sprintf_s(szpacket, "%05X", startAddr);

		// 2048 Byte 단위로 끊어서 Packet을 전송한다.
		// 남은 Data가 2048보다 작을 경우 남은 갯수 만큼만 전송한다.
		if ((startAddr + 2048) <= m_nFirmwareDataLen)
		{
			packetLen = 5 + 2048;
			memcpy(&szpacket[5], (char*)&m_pFirmwareData[startAddr], 2048);
			bRet = m_pApp->pCommand->Gf_setDownloadData(rack, packetLen, szpacket);
			if (bRet == FALSE)
			{
				return FALSE;
			}
		}
		else
		{
			packetLen = 5 + (m_nFirmwareDataLen - startAddr);
			memcpy(&szpacket[5], (char*)&m_pFirmwareData[startAddr], (m_nFirmwareDataLen - startAddr));
			bRet = m_pApp->pCommand->Gf_setDownloadData(rack, packetLen, szpacket);
			break;
		}

		ZeroMemory(szpacket, sizeof(szpacket));
		startAddr += 2048;

		int nPos;
		nPos = (startAddr * 100) / m_nFirmwareDataLen;
		m_ctrAfProgress.SetPos(nPos);

		CString sPer;
		sPer.Format(_T("(%d%%)"), nPos);
		GetDlgItem(IDC_STT_AF_PERCENT)->SetWindowText(sPer);

		ProcessMessage();

		if (bFirstTime == TRUE)
		{
			bFirstTime = FALSE;
			delayMs(2000);
		}
		else
		{
			delayMs(100);
		}
	}

	return TRUE;
}

BOOL CAutoFirmware::Lf_sendDownloadCompletePG(int rack)
{
	return m_pApp->pCommand->Gf_setDownloadComplete(rack);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CAutoFirmware::Lf_firmwareDownloadStartDIO()
{
	CMessageError err_dlg;
	CString sdata;

	sdata.Format(_T("DIO Board Firmware Downloading ..."));
	GetDlgItem(IDC_STT_AF_STATUS)->SetWindowText(sdata);

	// Progress 초기화
	m_ctrAfProgress.SetPos(0);
	GetDlgItem(IDC_STT_AF_PERCENT)->SetWindowText(_T("(0%)"));

	// Channel Version Display 초기화
	Lf_clearVersionStatus(NULL);

	// Step1. Download Ready Check
	if (Lf_checkDownloadReady1DIO() == FALSE)
	{
		err_dlg.m_strEMessage.Format(_T("xxx  [DIO Board] Firmware Download Fail - Ready Check  xxx"));
		err_dlg.DoModal();
		goto ERR_EXCEPT;
	}

	// Step2. Download Ready Delay
	delayMs(2000);

	// Step3. Download Sequence Set
	m_pApp->m_nDownloadReadyAckCount = 0;
	Lf_checkDownloadReady2DIO();

	// Step4. Download Start - Send Raw Data
	delayMs(300);
	if (Lf_sendFirmwareFileDIO() == FALSE)
	{
		err_dlg.m_strEMessage.Format(_T("xxx  [DIO Board] Firmware Download Fail - Data Download  xxx"));
		err_dlg.DoModal();
		goto ERR_EXCEPT;
	}

	// Step5. Download Complete Check
	delayMs(100);
	if (Lf_sendDownloadCompleteDIO() == FALSE)
	{
		err_dlg.m_strEMessage.Format(_T("xxx  [DIO Board] Firmware Download Fail - Complete Check  xxx"));
		err_dlg.DoModal();
		goto ERR_EXCEPT;
	}

	// Step6. Download Initialize & Ready
	Lf_readyInitialize();

	m_ctrAfProgress.SetPos(100);
	GetDlgItem(IDC_STT_AF_PERCENT)->SetWindowText(_T("(100%)"));
	return TRUE;

ERR_EXCEPT:
	// Error Exception. Initialize.
	Lf_readyInitialize();
	return FALSE;
}

BOOL CAutoFirmware::Lf_checkDownloadReady1DIO()
{
	// App 영역에서 Boot 영역으로 진입하기 위한 Ready Command 이다.
	DWORD sTick, eTick;

	sTick = ::GetTickCount();
	while (1)
	{
		m_pApp->m_nDownloadCountUp = TRUE;
		m_pApp->pCommand->Gf_dio_setGoToBootSection();
		delayMs(30);

		if (m_pApp->m_nDownloadReadyAckCount > 10)
		{
			return TRUE;
		}

		eTick = ::GetTickCount();
		if ((eTick - sTick) > 10000)
			break;

		ProcessMessage();
	}

	return FALSE;
}

BOOL CAutoFirmware::Lf_checkDownloadReady2DIO()
{
	// Boot 영역에서 Download Sequence로 진입하기 위한 Ready Command 이다.
	DWORD sTick, eTick;

	sTick = ::GetTickCount();
	while (1)
	{
		m_pApp->m_nDownloadCountUp = TRUE;

		m_pApp->pCommand->Gf_dio_setGoToBootSection();

		delayMs(30);

		if (m_pApp->m_nDownloadReadyAckCount > 5)
		{
			return TRUE;
		}

		eTick = ::GetTickCount();
		if ((eTick - sTick) > 3000)
			break;

		ProcessMessage();
	}

	return FALSE;
}

BOOL CAutoFirmware::Lf_sendFirmwareFileDIO()
{
	BOOL bRet = FALSE;
	int startAddr = 0;
	int packetLen = 0;
	char szpacket[4096] = { 0, };

	BOOL bFirstTime = TRUE;
	while (1)
	{
		sprintf_s(szpacket, "%05X", startAddr);

		// 2048 Byte 단위로 끊어서 Packet을 전송한다.
		// 남은 Data가 2048보다 작을 경우 남은 갯수 만큼만 전송한다.
		if ((startAddr + 2048) <= m_nFirmwareDataLen)
		{
			packetLen = 5 + 2048;
			memcpy(&szpacket[5], (char*)&m_pFirmwareData[startAddr], 2048);
			bRet = m_pApp->pCommand->Gf_dio_setDownloadData(packetLen, szpacket);
			if (bRet == FALSE)
			{
				return FALSE;
			}
		}
		else
		{
			packetLen = 5 + (m_nFirmwareDataLen - startAddr);
			memcpy(&szpacket[5], (char*)&m_pFirmwareData[startAddr], (m_nFirmwareDataLen - startAddr));
			bRet = m_pApp->pCommand->Gf_dio_setDownloadData(packetLen, szpacket);
			break;
		}

		ZeroMemory(szpacket, sizeof(szpacket));
		startAddr += 2048;

		int nPos;
		nPos = (startAddr * 100) / m_nFirmwareDataLen;
		m_ctrAfProgress.SetPos(nPos);

		CString sPer;
		sPer.Format(_T("(%d%%)"), nPos);
		GetDlgItem(IDC_STT_AF_PERCENT)->SetWindowText(sPer);

		ProcessMessage();

		if (bFirstTime == TRUE)
		{
			bFirstTime = FALSE;
			delayMs(1000);
		}
		else
		{
			delayMs(100);
		}
	}

	return TRUE;
}

BOOL CAutoFirmware::Lf_sendDownloadCompleteDIO()
{
	return m_pApp->pCommand->Gf_dio_setDownloadComplete();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAutoFirmware::Lf_readFirmwareVersion()
{
	CString sLog;

	if (Lf_checkRackSelect() == FALSE)
		return;

	// Firmware Version 초기화
	Lf_clearVersionStatus(m_nSelRack);

	// Firmware Version Read
	if (m_nTargetBoard == FW_TARGET_PG)
	{
		if (m_nSelRack == MAX_RACK)
		{
			for (int rack = 0; rack < MAX_RACK; rack++)
			{
				if (Lf_isRackEthConnect(rack) == FALSE)
					continue;

				for (int layer = 0; layer < MAX_LAYER; layer++)
				{
					lpInspWorkInfo->m_sMainFWVersion[rack][layer].Empty();
				}

				// Firmware Version을 Read한다.
				m_pApp->pCommand->Gf_getMainBoardFwVersion(rack);
			}
		}
		else
		{
			for (int layer = 0; layer < MAX_LAYER; layer++)
			{
				lpInspWorkInfo->m_sMainFWVersion[m_nSelRack][layer].Empty();
			}

			// Firmware Version을 Read한다.
			m_pApp->pCommand->Gf_getMainBoardFwVersion(m_nSelRack);
		}

		delayMs(200);

		// Firmware Version을 UI에 Update한다.
		if (m_nSelRack == MAX_RACK)
		{
			for (int rack = 0; rack < MAX_RACK; rack++)
			{
				for (int ma = 0; ma < MAX_LAYER; ma++)
				{
					m_pStaticVer[rack][ma]->SetWindowText(lpInspWorkInfo->m_sMainFWVersion[rack][ma]);

					sLog.Format(_T("<FIRMWARE> Read Version,  CH%d - %s"), (ma + 1), lpInspWorkInfo->m_sMainFWVersion[rack][ma]);
					m_pApp->Gf_writeMLog(sLog);
				}
			}
		}
		else
		{
			for (int ma = 0; ma < MAX_LAYER; ma++)
			{
				m_pStaticVer[m_nSelRack][ma]->SetWindowText(lpInspWorkInfo->m_sMainFWVersion[m_nSelRack][ma]);

				sLog.Format(_T("<FIRMWARE> Read Version,  CH%d - %s"), (ma + 1), lpInspWorkInfo->m_sMainFWVersion[m_nSelRack][ma]);
				m_pApp->Gf_writeMLog(sLog);
			}
		}
	}
	else if (m_nTargetBoard == FW_TARGET_DIO)
	{
		// Firmware Version을 Read한다.
		if (m_pApp->pCommand->Gf_dio_getDIOBoardFwVersion() == TRUE)
		{
			GetDlgItem(IDC_STT_AF_VERSION_DIO_BOARD)->SetWindowText(lpInspWorkInfo->m_sDioFWVersion);
		}
	}

	delayMs(200);

}

BOOL CAutoFirmware::Lf_isRackEthConnect(int rack)
{
	BOOL bConnect = FALSE;

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		if (lpInspWorkInfo->m_nMainEthConnect[rack][layer] != 0)
			bConnect = TRUE;
	}

	return bConnect;
}

