// CableOpen.cpp: 구현 파일
//

#include "pch.h"
#include "HseAging.h"
#include "CableOpen.h"
#include "afxdialogex.h"


// CCableOpen 대화 상자

IMPLEMENT_DYNAMIC(CCableOpen, CDialog)

CCableOpen::CCableOpen(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_CABLE_OPEN, pParent)
{
	m_pDefaultFont = new CFont();
	m_pDefaultFont->CreateFont(15, 6, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

CCableOpen::~CCableOpen()
{
	if (m_pDefaultFont != NULL)
	{
		delete m_pDefaultFont;
	}
}

void CCableOpen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MBC_CO_RETRY, m_mbcCoRetry);
	DDX_Control(pDX, IDC_MBC_CO_CANCEL, m_mbcCoCancel);
}


BEGIN_MESSAGE_MAP(CCableOpen, CDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_MBC_CO_RETRY, &CCableOpen::OnBnClickedMbcCoRetry)
	ON_BN_CLICKED(IDC_MBC_CO_CANCEL, &CCableOpen::OnBnClickedMbcCoCancel)
END_MESSAGE_MAP()


// CCableOpen 메시지 처리기


BOOL CCableOpen::OnInitDialog()
{
	CDialog::OnInitDialog();
	lpModelInfo = m_pApp->GetModelInfo();
	lpSystemInfo = m_pApp->GetSystemInfo();
	lpInspWorkInfo = m_pApp->GetInspWorkInfo();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_pApp->Gf_writeMLog(_T("<TEST> 'Cable Open' Dialog Open"));

	// Dialog의 기본 FONT 설정.
	SendMessageToDescendants(WM_SETFONT, (WPARAM)m_pDefaultFont->GetSafeHandle(), 1, TRUE, FALSE);

	Lf_InitLocalValue();
	Lf_InitFontset();
	Lf_InitColorBrush();
	Lf_InitDialogDesign();

	SetTimer(1, 500, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CCableOpen::OnDestroy()
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


BOOL CCableOpen::PreTranslateMessage(MSG* pMsg)
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


HBRUSH CCableOpen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
			if (pWnd->GetDlgCtrlID() == IDC_STT_CO_TITLE)
			{
				pDC->SetBkColor(COLOR_DARK_NAVY);
				pDC->SetTextColor(COLOR_WHITE);
				return m_Brush[COLOR_IDX_DARK_NAVY];
			}
			if ((pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER1_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER2_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER3_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER4_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_CO_LAYER5_CH16)
				)
			{
				CString strResult;
				GetDlgItem(pWnd->GetDlgCtrlID())->GetWindowText(strResult);
				if (strResult == _T("OK"))
				{
					pDC->SetBkColor(COLOR_GREEN128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_GREEN128];
				}
				else if (strResult == _T("NG"))
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
				else
				{
					pDC->SetBkColor(COLOR_GRAY128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_GRAY128];
				}

			}
			break;
	}
	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.
	return hbr;
}


void CCableOpen::OnPaint()
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


void CCableOpen::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == 1)
	{
		KillTimer(1);

		// Cable Open Check 결과 값을 기준으로 Use/Unuse 채널 정보와 매칭검사 한다.
		if(m_pApp->pCommand->Gf_getCableOpenCheck(m_nRackNo) == TRUE)
		{
			Lf_updateCableOpenResult();
		}
		else
		{
			for (int layer = 0; layer < MAX_LAYER; layer++)
			{
				for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
				{
					m_pSttCableOpen[layer][ch]->SetWindowText(_T("Comm NG"));
				}
			}
			Lf_refreshCableOpen();
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CCableOpen::OnBnClickedMbcCoRetry()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			m_pSttCableOpen[layer][ch]->SetWindowText(_T("-"));
		}
	}
	Lf_refreshCableOpen();
	SetTimer(1, 100, NULL);
}


void CCableOpen::OnBnClickedMbcCoCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialog::OnCancel();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCableOpen::Lf_InitLocalValue()
{
	int layer, i;

	layer = LAYER_1;
	i = 0;
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH1);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH2);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH3);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH4);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH5);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH6);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH7);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH8);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH9);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH10);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH11);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH12);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH13);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH14);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH15);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER1_CH16);

	layer = LAYER_2;
	i = 0;
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH1);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH2);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH3);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH4);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH5);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH6);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH7);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH8);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH9);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH10);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH11);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH12);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH13);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH14);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH15);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER2_CH16);

	layer = LAYER_3;
	i = 0;
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH1);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH2);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH3);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH4);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH5);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH6);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH7);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH8);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH9);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH10);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH11);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH12);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH13);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH14);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH15);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER3_CH16);

	layer = LAYER_4;
	i = 0;
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH1);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH2);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH3);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH4);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH5);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH6);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH7);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH8);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH9);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH10);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH11);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH12);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH13);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH14);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH15);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER4_CH16);

	layer = LAYER_5;
	i = 0;
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH1);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH2);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH3);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH4);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH5);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH6);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH7);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH8);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH9);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH10);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH11);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH12);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH13);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH14);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH15);
	m_pSttCableOpen[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_CO_LAYER5_CH16);
}

void CCableOpen::Lf_InitFontset()
{
	m_Font[0].CreateFont(150, 70, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[1].CreateFont(44, 20, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_CO_TITLE)->SetFont(&m_Font[1]);

	m_Font[2].CreateFont(32, 14, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[3].CreateFont(26, 12, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_MBC_CO_RETRY)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_MBC_CO_CANCEL)->SetFont(&m_Font[3]);

	m_Font[4].CreateFont(19, 8, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[5].CreateFont(16, 7, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

void CCableOpen::Lf_InitColorBrush()
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
	m_Brush[COLOR_IDX_DARK_BG].CreateSolidBrush(COLOR_DARK_BG);
	m_Brush[COLOR_IDX_ITEM_HEAD].CreateSolidBrush(COLOR_ITEM_HEAD);
	m_Brush[COLOR_IDX_ITEM_TITLE].CreateSolidBrush(COLOR_ITEM_TITLE);
}

void CCableOpen::Lf_InitDialogDesign()
{

}

void CCableOpen::Lf_refreshCableOpen()
{
	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			m_pSttCableOpen[layer][ch]->Invalidate(FALSE);
		}
	}
}

void CCableOpen::Lf_updateCableOpenResult()
{
	CString sLog;
	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			if (lpInspWorkInfo->m_nMainEthConnect[m_nRackNo][layer] == 0)
			{
				// Ethernet 연결되지 않은 채널은 N/C로 표시한다.
				m_pSttCableOpen[layer][ch]->SetWindowText(_T("N/C"));
				sLog.Format(_T("<TEST> RACK[%d] LAYER[%d] CH[%d] Cable Check : N/C (Ethernet N/C)\r\n"), m_nRackNo + 1, layer + 1, ch + 1);
			}
			else if (lpInspWorkInfo->m_ast_ChUseUnuse[m_nRackNo][layer][ch] == CHANNEL_UNUSE)
			{
				// Channel Unuse 상태이면 Cable Open Check Skip 한다.
				m_pSttCableOpen[layer][ch]->SetWindowText(_T("Unuse"));
				sLog.Format(_T("<TEST> RACK[%d] LAYER[%d] CH[%d] Cable Check : SKIP (Channel Unuse)\r\n"), m_nRackNo + 1, layer + 1, ch + 1);
			}
			else if (lpInspWorkInfo->m_ast_CableOpenCheck[m_nRackNo][layer][ch] == CABLE_CHECK_OK)
			{
				m_pSttCableOpen[layer][ch]->SetWindowText(_T("OK"));
				sLog.Format(_T("<TEST> RACK[%d] LAYER[%d] CH[%d] Cable Check : OK\r\n"), m_nRackNo + 1, layer + 1, ch + 1);
			}
			else if(lpInspWorkInfo->m_ast_CableOpenCheck[m_nRackNo][layer][ch] == CABLE_CHECK_NG)
			{
				m_pSttCableOpen[layer][ch]->SetWindowText(_T("NG"));
				sLog.Format(_T("<TEST> RACK[%d] LAYER[%d] CH[%d] Cable Check : NG\r\n"), m_nRackNo + 1, layer + 1, ch + 1);
			}
			m_pApp->Gf_writeMLog(sLog);
		}
	}
	Lf_refreshCableOpen();
}


