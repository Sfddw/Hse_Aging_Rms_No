// MessageQuestion.cpp: 구현 파일
//

#include "pch.h"
#include "HseAging.h"
#include "MessageQuestion.h"
#include "afxdialogex.h"


// CMessageQuestion 대화 상자

IMPLEMENT_DYNAMIC(CMessageQuestion, CDialog)

CMessageQuestion::CMessageQuestion(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MESSAGE_QUESTION, pParent)
{

}

CMessageQuestion::~CMessageQuestion()
{
}

void CMessageQuestion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_MQ_CONFIRM, m_btnMqConfirm);
	DDX_Control(pDX, IDC_BTN_MQ_CANCEL, m_btnMqCancel);
	DDX_Control(pDX, IDC_PIC_MQ_QUESTION, m_picMqQuestion);
}


BEGIN_MESSAGE_MAP(CMessageQuestion, CDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_MQ_CONFIRM, &CMessageQuestion::OnBnClickedBtnMqConfirm)
	ON_BN_CLICKED(IDC_BTN_MQ_CANCEL, &CMessageQuestion::OnBnClickedBtnMqCancel)
END_MESSAGE_MAP()


// CMessageQuestion 메시지 처리기


BOOL CMessageQuestion::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_pApp->Gf_writeMLog(_T("<TEST> 'Message Question' Dialog Open"));

	Lf_InitLocalValue();
	Lf_InitFontset();
	Lf_InitColorBrush();
	Lf_InitDialogDesign();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CMessageQuestion::OnDestroy()
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


BOOL CMessageQuestion::PreTranslateMessage(MSG* pMsg)
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


HBRUSH CMessageQuestion::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
		if (pWnd->GetDlgCtrlID() == IDC_STT_MQ_QUESTION_MSG)
		{
			pDC->SetBkColor(COLOR_GRAY192);
			pDC->SetTextColor(COLOR_BLACK);
			return m_Brush[COLOR_IDX_GRAY192];
		}

		break;
	}
	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.
	return hbr;
}


void CMessageQuestion::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.

	CRect rect, rectOri;
	GetClientRect(&rect);
	rectOri = rect;

	//rect.bottom = 80;
	dc.FillSolidRect(rect, COLOR_GRAY192);
}


void CMessageQuestion::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CDialog::OnTimer(nIDEvent);
}


void CMessageQuestion::OnBnClickedBtnMqConfirm()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pApp->Gf_writeMLog(_T("<MESSAGE> 'Confirm' Button Click"));
	CDialog::OnOK();
}


void CMessageQuestion::OnBnClickedBtnMqCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pApp->Gf_writeMLog(_T("<MESSAGE> 'Cancel' Button Click"));
	CDialog::OnCancel();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMessageQuestion::Lf_InitLocalValue()
{
	GetDlgItem(IDC_STT_MQ_QUESTION_MSG)->SetWindowText(m_strQMessage);
	if (m_strLButton.GetLength() != 0)
	{
		GetDlgItem(IDC_BTN_MQ_CONFIRM)->SetWindowText(m_strLButton);
	}
	if (m_strRButton.GetLength() != 0)
	{
		GetDlgItem(IDC_BTN_MQ_CANCEL)->SetWindowText(m_strRButton);
	}

	CString sLog;
	sLog.Format(_T("<MESSAGE> %s"), m_strQMessage);
	m_pApp->Gf_writeMLog(sLog);
}

void CMessageQuestion::Lf_InitFontset()
{
	m_Font[0].CreateFont(150, 70, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[1].CreateFont(44, 20, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[2].CreateFont(32, 14, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[3].CreateFont(26, 12, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_MQ_QUESTION_MSG)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_BTN_MQ_CONFIRM)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_BTN_MQ_CANCEL)->SetFont(&m_Font[3]);

	m_Font[4].CreateFont(19, 8, 0, 0, FW_BOLD, TRUE, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[5].CreateFont(16, 7, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

void CMessageQuestion::Lf_InitColorBrush()
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
	m_Brush[COLOR_IDX_GRAY64].CreateSolidBrush(COLOR_GRAY64);
	m_Brush[COLOR_IDX_GRAY128].CreateSolidBrush(COLOR_GRAY128);
	m_Brush[COLOR_IDX_GRAY159].CreateSolidBrush(COLOR_GRAY159);
	m_Brush[COLOR_IDX_GRAY192].CreateSolidBrush(COLOR_GRAY192);
	m_Brush[COLOR_IDX_GRAY224].CreateSolidBrush(COLOR_GRAY224);
	m_Brush[COLOR_IDX_GRAY240].CreateSolidBrush(COLOR_GRAY240);
	m_Brush[COLOR_IDX_BLUISH].CreateSolidBrush(COLOR_BLUISH);
	m_Brush[COLOR_IDX_DARK_BLUE].CreateSolidBrush(COLOR_DARK_BLUE);
	m_Brush[COLOR_IDX_DARK_NAVY].CreateSolidBrush(COLOR_DARK_NAVY);
}

void CMessageQuestion::Lf_InitDialogDesign()
{
	CBitmap m_Bit;
	m_Bit.LoadBitmap(IDB_BMP_QUESTION);
	m_picMqQuestion.SetBitmap(m_Bit);
	m_Bit.Detach();

	m_btnMqConfirm.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_OX_O));
	m_btnMqCancel.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_OX_X));
}