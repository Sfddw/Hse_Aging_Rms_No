// ModelInfo.cpp: 구현 파일
//

#include "pch.h"
#include "HseAging.h"
#include "ModelInfo.h"
#include "afxdialogex.h"
#include "MessageQuestion.h"
#include "Password.h"

// CModelInfo 대화 상자

IMPLEMENT_DYNAMIC(CModelInfo, CDialog)

CModelInfo::CModelInfo(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MODEL_INFO, pParent)
{
	m_pDefaultFont = new CFont();
	m_pDefaultFont->CreateFont(15, 6, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

CModelInfo::~CModelInfo()
{
	if (m_pDefaultFont != NULL)
	{
		delete m_pDefaultFont;
	}
}

void CModelInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_MI_SIGNAL_TYPE, m_cmbMiSignalType);
	DDX_Control(pDX, IDC_CMB_MI_PIXEL_TYPE, m_cmbMiPixelType);
	DDX_Control(pDX, IDC_CMB_MI_ODD_EVEN, m_cmbMiOddEven);
	DDX_Control(pDX, IDC_CMB_MI_SIGNAL_BIT, m_cmbMiSignalBit);
	DDX_Control(pDX, IDC_CMB_MI_BIT_SWAP, m_cmbMiBitSwap);
	DDX_Control(pDX, IDC_CMB_MI_LVDS_RS_SEL, m_cmbMiLvdsRsSel);
	DDX_Control(pDX, IDC_CMB_MI_DIMMING_SEL, m_cmbMiDimmingSel);
	DDX_Control(pDX, IDC_CMB_MI_CABLE_OPEN, m_cmbMiCableOpen);
	DDX_Control(pDX, IDC_CMB_MI_POWER_ON_SEQ1, m_cmbMiPowerOnSeq1);
	DDX_Control(pDX, IDC_CMB_MI_POWER_ON_SEQ2, m_cmbMiPowerOnSeq2);
	DDX_Control(pDX, IDC_CMB_MI_POWER_ON_SEQ3, m_cmbMiPowerOnSeq3);
	DDX_Control(pDX, IDC_CMB_MI_POWER_ON_SEQ4, m_cmbMiPowerOnSeq4);
	DDX_Control(pDX, IDC_CMB_MI_POWER_ON_SEQ5, m_cmbMiPowerOnSeq5);
	DDX_Control(pDX, IDC_CMB_MI_POWER_ON_SEQ6, m_cmbMiPowerOnSeq6);
	DDX_Control(pDX, IDC_CMB_MI_POWER_ON_SEQ7, m_cmbMiPowerOnSeq7);
	DDX_Control(pDX, IDC_CMB_MI_POWER_ON_SEQ8, m_cmbMiPowerOnSeq8);
	DDX_Control(pDX, IDC_CMB_MI_POWER_ON_SEQ9, m_cmbMiPowerOnSeq9);
	DDX_Control(pDX, IDC_CMB_MI_POWER_ON_SEQ10, m_cmbMiPowerOnSeq10);
	DDX_Control(pDX, IDC_CMB_MI_POWER_OFF_SEQ1, m_cmbMiPowerOffSeq1);
	DDX_Control(pDX, IDC_CMB_MI_POWER_OFF_SEQ2, m_cmbMiPowerOffSeq2);
	DDX_Control(pDX, IDC_CMB_MI_POWER_OFF_SEQ3, m_cmbMiPowerOffSeq3);
	DDX_Control(pDX, IDC_CMB_MI_POWER_OFF_SEQ4, m_cmbMiPowerOffSeq4);
	DDX_Control(pDX, IDC_CMB_MI_POWER_OFF_SEQ5, m_cmbMiPowerOffSeq5);
	DDX_Control(pDX, IDC_CMB_MI_POWER_OFF_SEQ6, m_cmbMiPowerOffSeq6);
	DDX_Control(pDX, IDC_CMB_MI_POWER_OFF_SEQ7, m_cmbMiPowerOffSeq7);
	DDX_Control(pDX, IDC_CMB_MI_POWER_OFF_SEQ8, m_cmbMiPowerOffSeq8);
	DDX_Control(pDX, IDC_CMB_MI_POWER_OFF_SEQ9, m_cmbMiPowerOffSeq9);
	DDX_Control(pDX, IDC_CMB_MI_POWER_OFF_SEQ10, m_cmbMiPowerOffSeq10);
	DDX_Control(pDX, IDC_CMB_MI_LOAD_MODEL, m_cmbMiLoadModel);
	DDX_Control(pDX, IDC_MBT_MI_MODEL_LOAD, m_mbtMiModelLoad);
	DDX_Control(pDX, IDC_MBT_MI_MODEL_DELETE, m_mbtMiModelDelete);
	DDX_Control(pDX, IDC_CMB_MI_AGING_END_WAIT_TIME, m_cmbMiAgingEndWaitTime);
	DDX_Control(pDX, IDC_CMB_MI_TEMPERATURE_USE, m_cmbMiTemperatureUse);
	DDX_Control(pDX, IDC_CMB_MI_DOOR_USE, m_cmbMiDoorUse);
	DDX_Control(pDX, IDC_BTN_MI_SAVE, m_btnMiSave);
	DDX_Control(pDX, IDC_BTN_MI_CANCEL, m_btnMiCancel);
}


BEGIN_MESSAGE_MAP(CModelInfo, CDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_MI_SAVE, &CModelInfo::OnBnClickedBtnMiSave)
	ON_BN_CLICKED(IDC_BTN_MI_CANCEL, &CModelInfo::OnBnClickedBtnMiCancel)
	ON_EN_CHANGE(IDC_EDT_MI_AGING_TIME_HH, &CModelInfo::OnEnChangeEdtMiAgingTimeHh)
	ON_EN_CHANGE(IDC_EDT_MI_AGING_TIME_MM, &CModelInfo::OnEnChangeEdtMiAgingTimeMm)
	ON_EN_CHANGE(IDC_EDT_MI_HOR_ACTIVE, &CModelInfo::OnEnChangeEdtMiHorActive)
	ON_EN_CHANGE(IDC_EDT_MI_HOR_WIDTH, &CModelInfo::OnEnChangeEdtMiHorWidth)
	ON_EN_CHANGE(IDC_EDT_MI_HOR_BP, &CModelInfo::OnEnChangeEdtMiHorBp)
	ON_EN_CHANGE(IDC_EDT_MI_HOR_FP, &CModelInfo::OnEnChangeEdtMiHorFp)
	ON_EN_CHANGE(IDC_EDT_MI_VER_ACTIVE, &CModelInfo::OnEnChangeEdtMiVerActive)
	ON_EN_CHANGE(IDC_EDT_MI_VER_WIDTH, &CModelInfo::OnEnChangeEdtMiVerWidth)
	ON_EN_CHANGE(IDC_EDT_MI_VER_BP, &CModelInfo::OnEnChangeEdtMiVerBp)
	ON_EN_CHANGE(IDC_EDT_MI_VER_FP, &CModelInfo::OnEnChangeEdtMiVerFp)
	ON_BN_CLICKED(IDC_MBT_MI_MODEL_LOAD, &CModelInfo::OnBnClickedMbtMiModelLoad)
	ON_BN_CLICKED(IDC_MBT_MI_MODEL_DELETE, &CModelInfo::OnBnClickedMbtMiModelDelete)
END_MESSAGE_MAP()


// CModelInfo 메시지 처리기


BOOL CModelInfo::OnInitDialog()
{
	CDialog::OnInitDialog();
	lpSystemInfo = m_pApp->GetSystemInfo();
	lpModelInfo = m_pApp->GetModelInfo();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_pApp->Gf_writeMLog(_T("<TEST> 'MODEL Information' Dialog Open"));

	// Dialog의 기본 FONT 설정.
	SendMessageToDescendants(WM_SETFONT, (WPARAM)m_pDefaultFont->GetSafeHandle(), 1, TRUE, FALSE);

	Lf_InitLocalValue();
	Lf_InitFontset();
	Lf_InitColorBrush();
	Lf_InitDialogDesign();

	Lf_InitComboModelList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CModelInfo::OnDestroy()
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


BOOL CModelInfo::PreTranslateMessage(MSG* pMsg)
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


HBRUSH CModelInfo::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
			if (pWnd->GetDlgCtrlID() == IDC_STT_MI_TITLE)
			{
				pDC->SetBkColor(COLOR_DARK_NAVY);
				pDC->SetTextColor(COLOR_WHITE);
				return m_Brush[COLOR_IDX_DARK_NAVY];
			}
			if ((pWnd->GetDlgCtrlID() == IDC_STT_MI_LOAD_MODEL)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MI_SAVE_MODEL)
				)
			{
				pDC->SetBkColor(COLOR_DARK_NAVY);
				pDC->SetTextColor(COLOR_WHITE);
				return m_Brush[COLOR_IDX_DARK_NAVY];
			}
			if ((pWnd->GetDlgCtrlID() == IDC_STT_MI_TIMING_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MI_LCM_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MI_PWM_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MI_FUNCTION_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MI_POWER_SEQ_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MI_POWER_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MI_AGING_SET_TITLE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MI_OPERATION_TITLE)
				)
			{
				pDC->SetBkColor(COLOR_DARK_NAVY);
				pDC->SetTextColor(COLOR_WHITE);
				return m_Brush[COLOR_IDX_DARK_NAVY];
			}
			// EDID Box가 Disable 상태가되면 Static Color로 제어된다.
			if ((pWnd->GetDlgCtrlID() == IDC_EDT_MI_HOR_TOTAL)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_MI_VER_TOTAL)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_MI_VSYNC)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_MI_AGING_TIME_MINUTE)
				)
			{
				pDC->SetBkColor(COLOR_JADEGREEN);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_JADEGREEN];
			}
			break;
	}

	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.
	return hbr;
}


void CModelInfo::OnPaint()
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

void CModelInfo::OnBnClickedMbtMiModelLoad()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_cmbMiLoadModel.GetCurSel() == 0)
	{
		m_pApp->Gf_ShowMessageBox(_T("No model selected. Please select a model."));
		return;
	}
	Lf_loadModelData();
}


void CModelInfo::OnBnClickedMbtMiModelDelete()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_deleteModelData();
}

void CModelInfo::OnBnClickedBtnMiSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CMessageQuestion msg_dlg;

	if (Lf_checkAgingTempInfoChange() == FALSE)
		return;

	msg_dlg.m_strQMessage.Format(_T("Model data Save ?"));
	if (msg_dlg.DoModal() == IDOK)
	{
		Lf_saveModelData();
	}

	// Model 삭제 후 Cursor 이동할 위치를 계산한다.
	int selectedIndex;
	if (curLoadingModel.GetLength() != 0)
		selectedIndex = m_cmbMiLoadModel.FindStringExact(0, curLoadingModel);
	else
		selectedIndex = 0;

	Lf_InitComboModelList(selectedIndex);

}


void CModelInfo::OnBnClickedBtnMiCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialog::OnOK();
}


void CModelInfo::OnEnChangeEdtMiAgingTimeHh()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_calcAgingTimeMinute();
}


void CModelInfo::OnEnChangeEdtMiAgingTimeMm()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_calcAgingTimeMinute();
}

void CModelInfo::OnEnChangeEdtMiHorActive()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_calcHorResolution();
}


void CModelInfo::OnEnChangeEdtMiHorWidth()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_calcHorResolution();
}


void CModelInfo::OnEnChangeEdtMiHorBp()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_calcHorResolution();
}


void CModelInfo::OnEnChangeEdtMiHorFp()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_calcHorResolution();
}


void CModelInfo::OnEnChangeEdtMiVerActive()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_calcVerResolution();
}


void CModelInfo::OnEnChangeEdtMiVerWidth()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_calcVerResolution();
}


void CModelInfo::OnEnChangeEdtMiVerBp()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_calcVerResolution();
}


void CModelInfo::OnEnChangeEdtMiVerFp()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_calcVerResolution();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CModelInfo::Lf_InitLocalValue()
{

}

void CModelInfo::Lf_InitFontset()
{
	m_Font[0].CreateFont(150, 70, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[1].CreateFont(44, 20, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_MI_TITLE)->SetFont(&m_Font[1]);

	m_Font[2].CreateFont(32, 14, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_CMB_MI_LOAD_MODEL)->SetFont(&m_Font[2]);

	m_Font[3].CreateFont(26, 12, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_MI_LOAD_MODEL)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_STT_MI_SAVE_MODEL)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_EDT_MI_SAVE_MODEL)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_MBT_MI_MODEL_LOAD)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_MBT_MI_MODEL_DELETE)->SetFont(&m_Font[3]);

	GetDlgItem(IDC_BTN_MI_SAVE)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_BTN_MI_CANCEL)->SetFont(&m_Font[3]);

	m_Font[4].CreateFont(19, 8, 0, 0, FW_BOLD, TRUE, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[5].CreateFont(16, 7, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

void CModelInfo::Lf_InitColorBrush()
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
}

void CModelInfo::Lf_InitDialogDesign()
{
	m_btnMiSave.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_OX_O));
	m_btnMiCancel.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_OX_X));

	m_mbtMiModelLoad.EnableWindowsTheming(FALSE);
	m_mbtMiModelLoad.SetFaceColor(COLOR_VERDANT2);
	m_mbtMiModelLoad.SetTextColor(COLOR_BLACK);
	
	m_mbtMiModelDelete.EnableWindowsTheming(FALSE);
	m_mbtMiModelDelete.SetFaceColor(COLOR_RED128);
	m_mbtMiModelDelete.SetTextColor(COLOR_WHITE);
}

void CModelInfo::Lf_InitComboModelList(int setSel)
{
	CString strfilename = _T("");
	CString strfilepath = _T("");
	WIN32_FIND_DATA wfd;
	HANDLE hSearch;

	strfilepath.Format(_T("./Model/*.ini"));
	hSearch = FindFirstFile(strfilepath, &wfd);

	m_cmbMiLoadModel.ResetContent();
	m_cmbMiLoadModel.AddString(_T("- MODEL LIST -"));
	if (hSearch != INVALID_HANDLE_VALUE)
	{
		if (wfd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			strfilename.Format(_T("%s"), wfd.cFileName);
			strfilename = strfilename.Mid(0, strfilename.GetLength() - 4);
			strfilename.MakeUpper();
			m_cmbMiLoadModel.AddString(strfilename);
		}
		while (FindNextFile(hSearch, &wfd))
		{
			if (wfd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
			{
				strfilename.Format(_T("%s"), wfd.cFileName);
				strfilename = strfilename.Mid(0, strfilename.GetLength() - 4);
				strfilename.MakeUpper();
				m_cmbMiLoadModel.AddString(strfilename);
			}
		}
		FindClose(hSearch);
	}

	m_cmbMiLoadModel.SetCurSel(setSel);
}

void CModelInfo::Lf_loadModelData()
{
	CString modelName;

	m_cmbMiLoadModel.GetWindowText(modelName);

	CMessageQuestion msg_dlg;
	msg_dlg.m_strQMessage.Format(_T("Would you like to load model information?\r\nThe data you are writing will not be saved."));
	if (msg_dlg.DoModal() == IDOK)
	{
		m_pApp->Gf_loadModelData(modelName);
		Lf_reloadControlData();

		GetDlgItem(IDC_EDT_MI_SAVE_MODEL)->SetWindowText(modelName);
		curLoadingModel.Format(_T("%s"), modelName);
	}
}

void CModelInfo::Lf_deleteModelData()
{
	int selectedIndex;
	CString modelName;

	// Model 삭제 후 Cursor 이동할 위치를 계산한다.
	if (curLoadingModel.GetLength() != 0)
		selectedIndex = m_cmbMiLoadModel.FindStringExact(0, curLoadingModel);
	else
		selectedIndex = 0;

	// 수정중인 모델은 삭제할 수 없더록 인터락 추가
	m_cmbMiLoadModel.GetWindowText(modelName);
	if (curLoadingModel == modelName)
	{
		m_pApp->Gf_ShowMessageBox(_T("A model that is being modified cannot be deleted."));
		return;
	}

	// 삭제 여부 확인 메세지 출력
	CMessageQuestion msg_dlg;
	msg_dlg.m_strQMessage.Format(_T("Are you sure you want to delete the selected model ?"));
	if (msg_dlg.DoModal() == IDOK)
	{
		CString filePath;
		filePath.Format(_T("./Model/%s.ini"), modelName);
		DeleteFile(filePath);

		Lf_InitComboModelList(selectedIndex);
	}
}

void CModelInfo::Lf_reloadControlData()
{
	CString sdata;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Timing Set
	sdata.Format(_T("%.2f"), lpModelInfo->m_fTimingMainClock);
	GetDlgItem(IDC_EDT_MI_MCLOCK)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nTimingHorTotal);
	GetDlgItem(IDC_EDT_MI_HOR_TOTAL)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nTimingHorActive);
	GetDlgItem(IDC_EDT_MI_HOR_ACTIVE)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nTimingHorWidth);
	GetDlgItem(IDC_EDT_MI_HOR_WIDTH)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nTimingHorBP);
	GetDlgItem(IDC_EDT_MI_HOR_BP)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nTimingHorFP);
	GetDlgItem(IDC_EDT_MI_HOR_FP)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nTimingVerTotal);
	GetDlgItem(IDC_EDT_MI_VER_TOTAL)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nTimingVerActive);
	GetDlgItem(IDC_EDT_MI_VER_ACTIVE)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nTimingVerWidth);
	GetDlgItem(IDC_EDT_MI_VER_WIDTH)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nTimingVerBP);
	GetDlgItem(IDC_EDT_MI_VER_BP)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nTimingVerFP);
	GetDlgItem(IDC_EDT_MI_VER_FP)->SetWindowText(sdata);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// LCM Set
	m_cmbMiSignalType.SetCurSel(lpModelInfo->m_nLcmSignalType);
	m_cmbMiPixelType.SetCurSel(lpModelInfo->m_nLcmPixelType);
	m_cmbMiOddEven.SetCurSel(lpModelInfo->m_nLcmOddEven);
	m_cmbMiSignalBit.SetCurSel(lpModelInfo->m_nLcmSignalBit);
	m_cmbMiBitSwap.SetCurSel(lpModelInfo->m_nLcmBitSwap);
	m_cmbMiLvdsRsSel.SetCurSel(lpModelInfo->m_nLcmLvdsRsSel);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Inverter & PWM Set
	m_cmbMiDimmingSel.SetCurSel(lpModelInfo->m_nDimmingSel);

	sdata.Format(_T("%d"), lpModelInfo->m_nPwmFreq);
	GetDlgItem(IDC_EDT_MI_PWM_FREQ)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nPwmDuty);
	GetDlgItem(IDC_EDT_MI_PWM_DUTY)->SetWindowText(sdata);

	sdata.Format(_T("%.1f"), lpModelInfo->m_fVbrVolt);
	GetDlgItem(IDC_EDT_MI_VBR_VOLT)->SetWindowText(sdata);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Function Set
	m_cmbMiCableOpen.SetCurSel(lpModelInfo->m_nFuncCableOpen);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Power Sequence Set
	m_cmbMiPowerOnSeq1.SetCurSel(lpModelInfo->m_nPowerOnSeq1);
	m_cmbMiPowerOnSeq2.SetCurSel(lpModelInfo->m_nPowerOnSeq2);
	m_cmbMiPowerOnSeq3.SetCurSel(lpModelInfo->m_nPowerOnSeq3);
	m_cmbMiPowerOnSeq4.SetCurSel(lpModelInfo->m_nPowerOnSeq4);
	m_cmbMiPowerOnSeq5.SetCurSel(lpModelInfo->m_nPowerOnSeq5);
	m_cmbMiPowerOnSeq6.SetCurSel(lpModelInfo->m_nPowerOnSeq6);
	m_cmbMiPowerOnSeq7.SetCurSel(lpModelInfo->m_nPowerOnSeq7);
	m_cmbMiPowerOnSeq8.SetCurSel(lpModelInfo->m_nPowerOnSeq8);
	m_cmbMiPowerOnSeq9.SetCurSel(lpModelInfo->m_nPowerOnSeq9);
	m_cmbMiPowerOnSeq10.SetCurSel(lpModelInfo->m_nPowerOnSeq10);
	//m_cmbMiPowerOnSeq11.SetCurSel(lpModelInfo->m_nPowerOnSeq11);

	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOnDelay1);
	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY1)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOnDelay2);
	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY2)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOnDelay3);
	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY3)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOnDelay4);
	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY4)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOnDelay5);
	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY5)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOnDelay6);
	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY6)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOnDelay7);
	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY7)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOnDelay8);
	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY8)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOnDelay9);
	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY9)->SetWindowText(sdata);
	//sdata.Format(_T("%d"), lpModelInfo->m_nPowerOnDelay10);
	//GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY10)->SetWindowText(sdata);

	m_cmbMiPowerOffSeq1.SetCurSel(lpModelInfo->m_nPowerOffSeq1);
	m_cmbMiPowerOffSeq2.SetCurSel(lpModelInfo->m_nPowerOffSeq2);
	m_cmbMiPowerOffSeq3.SetCurSel(lpModelInfo->m_nPowerOffSeq3);
	m_cmbMiPowerOffSeq4.SetCurSel(lpModelInfo->m_nPowerOffSeq4);
	m_cmbMiPowerOffSeq5.SetCurSel(lpModelInfo->m_nPowerOffSeq5);
	m_cmbMiPowerOffSeq6.SetCurSel(lpModelInfo->m_nPowerOffSeq6);
	m_cmbMiPowerOffSeq7.SetCurSel(lpModelInfo->m_nPowerOffSeq7);
	m_cmbMiPowerOffSeq8.SetCurSel(lpModelInfo->m_nPowerOffSeq8);
	m_cmbMiPowerOffSeq9.SetCurSel(lpModelInfo->m_nPowerOffSeq9);
	m_cmbMiPowerOffSeq10.SetCurSel(lpModelInfo->m_nPowerOffSeq10);
	//m_cmbMiPowerOffSeq11.SetCurSel(lpModelInfo->m_nPowerOffSeq11);

	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay1);
	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY1)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay2);
	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY2)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay3);
	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY3)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay4);
	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY4)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay5);
	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY5)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay6);
	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY6)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay7);
	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY7)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay8);
	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY8)->SetWindowText(sdata);
	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay9);
	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY9)->SetWindowText(sdata);
	//sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay10);
	//GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY10)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nPowerOffDelay);
	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY)->SetWindowText(sdata);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Power Set
	sdata.Format(_T("%.2f"), lpModelInfo->m_fVccVolt);
	GetDlgItem(IDC_EDT_MI_VCC_VOLTAGE)->SetWindowText(sdata);

	sdata.Format(_T("%.2f"), lpModelInfo->m_fVccVoltOffset);
	GetDlgItem(IDC_EDT_MI_VCC_OFFSET)->SetWindowText(sdata);

	sdata.Format(_T("%.2f"), lpModelInfo->m_fVccLimitVoltLow);
	GetDlgItem(IDC_EDT_MI_VCC_LIMIT_VOL_LOW)->SetWindowText(sdata);

	sdata.Format(_T("%.2f"), lpModelInfo->m_fVccLimitVoltHigh);
	GetDlgItem(IDC_EDT_MI_VCC_LIMIT_VOL_HIGH)->SetWindowText(sdata);

	sdata.Format(_T("%.2f"), lpModelInfo->m_fVccLimitCurrLow);
	GetDlgItem(IDC_EDT_MI_VCC_LIMIT_CUR_LOW)->SetWindowText(sdata);

	sdata.Format(_T("%.2f"), lpModelInfo->m_fVccLimitCurrHigh);
	GetDlgItem(IDC_EDT_MI_VCC_LIMIT_CUR_HIGH)->SetWindowText(sdata);


	sdata.Format(_T("%.2f"), lpModelInfo->m_fVblVolt);
	GetDlgItem(IDC_EDT_MI_VBL_VOLTAGE)->SetWindowText(sdata);

	sdata.Format(_T("%.2f"), lpModelInfo->m_fVblVoltOffset);
	GetDlgItem(IDC_EDT_MI_VBL_OFFSET)->SetWindowText(sdata);

	sdata.Format(_T("%.2f"), lpModelInfo->m_fVblLimitVoltLow);
	GetDlgItem(IDC_EDT_MI_VBL_LIMIT_VOL_LOW)->SetWindowText(sdata);

	sdata.Format(_T("%.2f"), lpModelInfo->m_fVblLimitVoltHigh);
	GetDlgItem(IDC_EDT_MI_VBL_LIMIT_VOL_HIGH)->SetWindowText(sdata);

	sdata.Format(_T("%.2f"), lpModelInfo->m_fVblLimitCurrLow);
	GetDlgItem(IDC_EDT_MI_VBL_LIMIT_CUR_LOW)->SetWindowText(sdata);

	sdata.Format(_T("%.2f"), lpModelInfo->m_fVblLimitCurrHigh);
	GetDlgItem(IDC_EDT_MI_VBL_LIMIT_CUR_HIGH)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nAgingTimeHH);
	GetDlgItem(IDC_EDT_MI_AGING_TIME_HH)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nAgingTimeMM);
	GetDlgItem(IDC_EDT_MI_AGING_TIME_MM)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nAgingTimeMinute);
	GetDlgItem(IDC_EDT_MI_AGING_TIME_MINUTE)->SetWindowText(sdata);

	m_cmbMiAgingEndWaitTime.SetCurSel(lpModelInfo->m_nAgingEndWaitTime);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Operation Set
	m_cmbMiTemperatureUse.SetCurSel(lpModelInfo->m_nOpeTemperatureUse);

	sdata.Format(_T("%d"), lpModelInfo->m_nOpeTemperatureMin);
	GetDlgItem(IDC_EDT_MI_TEMPERATURE_MIN)->SetWindowText(sdata);

	sdata.Format(_T("%d"), lpModelInfo->m_nOpeTemperatureMax);
	GetDlgItem(IDC_EDT_MI_TEMPERATURE_MAX)->SetWindowText(sdata);

	m_cmbMiDoorUse.SetCurSel(lpModelInfo->m_nOpeDoorUse);
}


void CModelInfo::Lf_saveModelData()
{
	CString modelName, sdata;

	GetDlgItem(IDC_EDT_MI_SAVE_MODEL)->GetWindowText(modelName);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Timing Set
	GetDlgItem(IDC_EDT_MI_MCLOCK)->GetWindowText(sdata);
 	lpModelInfo->m_fTimingMainClock = (float)_tstof(sdata);
 	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_MAIN_CLOCK"), lpModelInfo->m_fTimingMainClock);

	GetDlgItem(IDC_EDT_MI_HOR_TOTAL)->GetWindowText(sdata);
	lpModelInfo->m_nTimingHorTotal = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_HOR_TOTAL"), lpModelInfo->m_nTimingHorTotal);

	GetDlgItem(IDC_EDT_MI_HOR_ACTIVE)->GetWindowText(sdata);
	lpModelInfo->m_nTimingHorActive = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_HOR_ACTIVE"), lpModelInfo->m_nTimingHorActive);

	GetDlgItem(IDC_EDT_MI_HOR_WIDTH)->GetWindowText(sdata);
	lpModelInfo->m_nTimingHorWidth = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_HOR_WIDTH"), lpModelInfo->m_nTimingHorWidth);

	GetDlgItem(IDC_EDT_MI_HOR_BP)->GetWindowText(sdata);
	lpModelInfo->m_nTimingHorBP = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_HOR_BACKPORCH"), lpModelInfo->m_nTimingHorBP);

	GetDlgItem(IDC_EDT_MI_HOR_FP)->GetWindowText(sdata);
	lpModelInfo->m_nTimingHorFP = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_HOR_FRONTPORCH"), lpModelInfo->m_nTimingHorFP);

	GetDlgItem(IDC_EDT_MI_VER_TOTAL)->GetWindowText(sdata);
	lpModelInfo->m_nTimingVerTotal = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_VER_TOTAL"), lpModelInfo->m_nTimingVerTotal);

	GetDlgItem(IDC_EDT_MI_VER_ACTIVE)->GetWindowText(sdata);
	lpModelInfo->m_nTimingVerActive = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_VER_ACTIVE"), lpModelInfo->m_nTimingVerActive);

	GetDlgItem(IDC_EDT_MI_VER_WIDTH)->GetWindowText(sdata);
	lpModelInfo->m_nTimingVerWidth = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_VER_WIDTH"), lpModelInfo->m_nTimingVerWidth);

	GetDlgItem(IDC_EDT_MI_VER_BP)->GetWindowText(sdata);
	lpModelInfo->m_nTimingVerBP = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_VER_BACKPORCH"), lpModelInfo->m_nTimingVerBP);

	GetDlgItem(IDC_EDT_MI_VER_FP)->GetWindowText(sdata);
	lpModelInfo->m_nTimingVerFP = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_VER_FRONTPORCH"), lpModelInfo->m_nTimingVerFP);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// LCM Set
	lpModelInfo->m_nLcmSignalType = m_cmbMiSignalType.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_SIGNAL_TYPE"), lpModelInfo->m_nLcmSignalType);

	lpModelInfo->m_nLcmPixelType = m_cmbMiPixelType.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_PIXEL_TYPE"), lpModelInfo->m_nLcmPixelType);

	lpModelInfo->m_nLcmOddEven = m_cmbMiOddEven.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_ODD_EVEN"), lpModelInfo->m_nLcmOddEven);

	lpModelInfo->m_nLcmSignalBit = m_cmbMiSignalBit.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_SIGNAL_BIT"), lpModelInfo->m_nLcmSignalBit);

	lpModelInfo->m_nLcmBitSwap = m_cmbMiBitSwap.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_BIT_SWAP"), lpModelInfo->m_nLcmBitSwap);

	lpModelInfo->m_nLcmLvdsRsSel = m_cmbMiLvdsRsSel.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_LVDS_RS_SEL"), lpModelInfo->m_nLcmLvdsRsSel);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Inverter & PWM Set
	lpModelInfo->m_nDimmingSel = m_cmbMiDimmingSel.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("DIMMING_SEL"), lpModelInfo->m_nDimmingSel);

	GetDlgItem(IDC_EDT_MI_PWM_FREQ)->GetWindowText(sdata);
	lpModelInfo->m_nPwmFreq = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("PWM_FREQ"), lpModelInfo->m_nPwmFreq);

	GetDlgItem(IDC_EDT_MI_PWM_DUTY)->GetWindowText(sdata);
	lpModelInfo->m_nPwmDuty = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("PWM_DUTY"), lpModelInfo->m_nPwmDuty);

	GetDlgItem(IDC_EDT_MI_VBR_VOLT)->GetWindowText(sdata);
	lpModelInfo->m_fVbrVolt = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VBR_VOLT"), lpModelInfo->m_fVbrVolt);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Function Set
	lpModelInfo->m_nFuncCableOpen = m_cmbMiCableOpen.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("CABLE_OPEN"), lpModelInfo->m_nFuncCableOpen);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Power Sequence Set
	lpModelInfo->m_nPowerOnSeq1 = m_cmbMiPowerOnSeq1.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ1"), lpModelInfo->m_nPowerOnSeq1);

	lpModelInfo->m_nPowerOnSeq2 = m_cmbMiPowerOnSeq2.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ2"), lpModelInfo->m_nPowerOnSeq2);

	lpModelInfo->m_nPowerOnSeq3 = m_cmbMiPowerOnSeq3.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ3"), lpModelInfo->m_nPowerOnSeq3);

	lpModelInfo->m_nPowerOnSeq4 = m_cmbMiPowerOnSeq4.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ4"), lpModelInfo->m_nPowerOnSeq4);

	lpModelInfo->m_nPowerOnSeq5 = m_cmbMiPowerOnSeq5.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ5"), lpModelInfo->m_nPowerOnSeq5);

	lpModelInfo->m_nPowerOnSeq6 = m_cmbMiPowerOnSeq6.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ6"), lpModelInfo->m_nPowerOnSeq6);

	lpModelInfo->m_nPowerOnSeq7 = m_cmbMiPowerOnSeq7.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ7"), lpModelInfo->m_nPowerOnSeq7);

	lpModelInfo->m_nPowerOnSeq8 = m_cmbMiPowerOnSeq8.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ8"), lpModelInfo->m_nPowerOnSeq8);

	lpModelInfo->m_nPowerOnSeq9 = m_cmbMiPowerOnSeq9.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ9"), lpModelInfo->m_nPowerOnSeq9);

	lpModelInfo->m_nPowerOnSeq10 = m_cmbMiPowerOnSeq10.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ10"), lpModelInfo->m_nPowerOnSeq10);

	//lpModelInfo->m_nPowerOnSeq11 = m_cmbMiPowerOnSeq11.GetCurSel();
	//Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ11"), lpModelInfo->m_nPowerOnSeq11);

	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY1)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOnDelay1 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY1"), lpModelInfo->m_nPowerOnDelay1);

	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY2)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOnDelay2 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY2"), lpModelInfo->m_nPowerOnDelay2);

	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY3)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOnDelay3 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY3"), lpModelInfo->m_nPowerOnDelay3);

	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY4)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOnDelay4 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY4"), lpModelInfo->m_nPowerOnDelay4);

	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY5)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOnDelay5 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY5"), lpModelInfo->m_nPowerOnDelay5);

	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY6)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOnDelay6 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY6"), lpModelInfo->m_nPowerOnDelay6);

	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY7)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOnDelay7 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY7"), lpModelInfo->m_nPowerOnDelay7);

	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY8)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOnDelay8 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY8"), lpModelInfo->m_nPowerOnDelay8);

	GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY9)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOnDelay9 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY9"), lpModelInfo->m_nPowerOnDelay9);

	//GetDlgItem(IDC_EDT_MI_POWER_ON_DELAY10)->GetWindowText(sdata);
	//lpModelInfo->m_nPowerOnDelay10 = _ttoi(sdata);
	//Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY10"), lpModelInfo->m_nPowerOnDelay10);

	lpModelInfo->m_nPowerOffSeq1 = m_cmbMiPowerOffSeq1.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ1"), lpModelInfo->m_nPowerOffSeq1);

	lpModelInfo->m_nPowerOffSeq2 = m_cmbMiPowerOffSeq2.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ2"), lpModelInfo->m_nPowerOffSeq2);

	lpModelInfo->m_nPowerOffSeq3 = m_cmbMiPowerOffSeq3.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ3"), lpModelInfo->m_nPowerOffSeq3);

	lpModelInfo->m_nPowerOffSeq4 = m_cmbMiPowerOffSeq4.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ4"), lpModelInfo->m_nPowerOffSeq4);

	lpModelInfo->m_nPowerOffSeq5 = m_cmbMiPowerOffSeq5.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ5"), lpModelInfo->m_nPowerOffSeq5);

	lpModelInfo->m_nPowerOffSeq6 = m_cmbMiPowerOffSeq6.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ6"), lpModelInfo->m_nPowerOffSeq6);

	lpModelInfo->m_nPowerOffSeq7 = m_cmbMiPowerOffSeq7.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ7"), lpModelInfo->m_nPowerOffSeq7);

	lpModelInfo->m_nPowerOffSeq8 = m_cmbMiPowerOffSeq8.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ8"), lpModelInfo->m_nPowerOffSeq8);

	lpModelInfo->m_nPowerOffSeq9 = m_cmbMiPowerOffSeq9.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ9"), lpModelInfo->m_nPowerOffSeq9);

	lpModelInfo->m_nPowerOffSeq10 = m_cmbMiPowerOffSeq10.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ10"), lpModelInfo->m_nPowerOffSeq10);

	//lpModelInfo->m_nPowerOffSeq11 = m_cmbMiPowerOffSeq10.GetCurSel();
	//Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ11"), lpModelInfo->m_nPowerOffSeq11;

	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY1)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOffDelay1 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY1"), lpModelInfo->m_nPowerOffDelay1);

	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY2)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOffDelay2 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY2"), lpModelInfo->m_nPowerOffDelay2);

	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY3)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOffDelay3 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY3"), lpModelInfo->m_nPowerOffDelay3);

	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY4)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOffDelay4 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY4"), lpModelInfo->m_nPowerOffDelay4);

	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY5)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOffDelay5 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY5"), lpModelInfo->m_nPowerOffDelay5);

	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY6)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOffDelay6 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY6"), lpModelInfo->m_nPowerOffDelay6);

	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY7)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOffDelay7 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY7"), lpModelInfo->m_nPowerOffDelay7);

	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY8)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOffDelay8 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY8"), lpModelInfo->m_nPowerOffDelay8);

	GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY9)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOffDelay9 = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY9"), lpModelInfo->m_nPowerOffDelay9);

	//GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY10)->GetWindowText(sdata);
	lpModelInfo->m_nPowerOffDelay10 = 0;
	//Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY10"), lpModelInfo->m_nPowerOffDelay10);

	//GetDlgItem(IDC_EDT_MI_POWER_OFF_DELAY)->GetWindowText(sdata);
	//lpModelInfo->m_nPowerOffDelay = _ttoi(sdata);
	//Write_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY"), lpModelInfo->m_nPowerOffDelay);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Power Set
	GetDlgItem(IDC_EDT_MI_VCC_VOLTAGE)->GetWindowText(sdata);
	lpModelInfo->m_fVccVolt = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_VOLT"), lpModelInfo->m_fVccVolt);

	GetDlgItem(IDC_EDT_MI_VCC_OFFSET)->GetWindowText(sdata);
	lpModelInfo->m_fVccVoltOffset = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_VOLT_OFFSET"), lpModelInfo->m_fVccVoltOffset);

	GetDlgItem(IDC_EDT_MI_VCC_LIMIT_VOL_LOW)->GetWindowText(sdata);
	lpModelInfo->m_fVccLimitVoltLow= (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_LIMIT_VOLT_LOW"), lpModelInfo->m_fVccLimitVoltLow);

	GetDlgItem(IDC_EDT_MI_VCC_LIMIT_VOL_HIGH)->GetWindowText(sdata);
	lpModelInfo->m_fVccLimitVoltHigh = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_LIMIT_VOLT_HIGH"), lpModelInfo->m_fVccLimitVoltHigh);

	GetDlgItem(IDC_EDT_MI_VCC_LIMIT_CUR_LOW)->GetWindowText(sdata);
	lpModelInfo->m_fVccLimitCurrLow = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_LIMIT_CURR_LOW"), lpModelInfo->m_fVccLimitCurrLow);

	GetDlgItem(IDC_EDT_MI_VCC_LIMIT_CUR_HIGH)->GetWindowText(sdata);
	lpModelInfo->m_fVccLimitCurrHigh = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_LIMIT_CURR_HIGH"), lpModelInfo->m_fVccLimitCurrHigh);


	GetDlgItem(IDC_EDT_MI_VBL_VOLTAGE)->GetWindowText(sdata);
	lpModelInfo->m_fVblVolt = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_VOLT"), lpModelInfo->m_fVblVolt);

	GetDlgItem(IDC_EDT_MI_VBL_OFFSET)->GetWindowText(sdata);
	lpModelInfo->m_fVblVoltOffset = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_VOLT_OFFSET"), lpModelInfo->m_fVblVoltOffset);

	GetDlgItem(IDC_EDT_MI_VBL_LIMIT_VOL_LOW)->GetWindowText(sdata);
	lpModelInfo->m_fVblLimitVoltLow = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_LIMIT_VOLT_LOW"), lpModelInfo->m_fVblLimitVoltLow);

	GetDlgItem(IDC_EDT_MI_VBL_LIMIT_VOL_HIGH)->GetWindowText(sdata);
	lpModelInfo->m_fVblLimitVoltHigh = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_LIMIT_VOLT_HIGH"), lpModelInfo->m_fVblLimitVoltHigh);

	GetDlgItem(IDC_EDT_MI_VBL_LIMIT_CUR_LOW)->GetWindowText(sdata);
	lpModelInfo->m_fVblLimitCurrLow = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_LIMIT_CURR_LOW"), lpModelInfo->m_fVblLimitCurrLow);

	GetDlgItem(IDC_EDT_MI_VBL_LIMIT_CUR_HIGH)->GetWindowText(sdata);
	lpModelInfo->m_fVblLimitCurrHigh = (float)_tstof(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_LIMIT_CURR_HIGH"), lpModelInfo->m_fVblLimitCurrHigh);

	GetDlgItem(IDC_EDT_MI_AGING_TIME_HH)->GetWindowText(sdata);
	lpModelInfo->m_nAgingTimeHH = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("AGING_TIME_HH"), lpModelInfo->m_nAgingTimeHH);

	GetDlgItem(IDC_EDT_MI_AGING_TIME_MM)->GetWindowText(sdata);
	lpModelInfo->m_nAgingTimeMM = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("AGING_TIME_MM"), lpModelInfo->m_nAgingTimeMM);

	GetDlgItem(IDC_EDT_MI_AGING_TIME_MINUTE)->GetWindowText(sdata);
	lpModelInfo->m_nAgingTimeMinute = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("AGING_TIME_MINUTE"), lpModelInfo->m_nAgingTimeMinute);

	lpModelInfo->m_nAgingEndWaitTime = m_cmbMiAgingEndWaitTime.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("AGING_END_WAIT_TIME"), lpModelInfo->m_nAgingEndWaitTime);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Operation Set
	lpModelInfo->m_nOpeTemperatureUse = m_cmbMiTemperatureUse.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TEMPERATURE_USE"), lpModelInfo->m_nOpeTemperatureUse);

	GetDlgItem(IDC_EDT_MI_TEMPERATURE_MIN)->GetWindowText(sdata);
	lpModelInfo->m_nOpeTemperatureMin = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TEMPERATURE_MIN"), lpModelInfo->m_nOpeTemperatureMin);

	GetDlgItem(IDC_EDT_MI_TEMPERATURE_MAX)->GetWindowText(sdata);
	lpModelInfo->m_nOpeTemperatureMax = _ttoi(sdata);
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("TEMPERATURE_MAX"), lpModelInfo->m_nOpeTemperatureMax);

	lpModelInfo->m_nOpeDoorUse = m_cmbMiDoorUse.GetCurSel();
	Write_ModelFile(modelName, _T("MODEL_INFO"), _T("DOOR_USE"), lpModelInfo->m_nOpeDoorUse);
}

void CModelInfo::Lf_calcAgingTimeMinute()
{
	CString sdata;
	int timeHH, timeMM, timeMinute;

	GetDlgItem(IDC_EDT_MI_AGING_TIME_HH)->GetWindowText(sdata);
	timeHH = _ttoi(sdata);
	GetDlgItem(IDC_EDT_MI_AGING_TIME_MM)->GetWindowText(sdata);
	timeMM = _ttoi(sdata);

	timeMinute = (timeHH * 60) + timeMM;
	sdata.Format(_T("%d"), timeMinute);
	GetDlgItem(IDC_EDT_MI_AGING_TIME_MINUTE)->SetWindowText(sdata);
}


void CModelInfo::Lf_calcHorResolution()
{
	CString sdata;
	int fp, active, bp, width;

	GetDlgItem(IDC_EDT_MI_HOR_ACTIVE)->GetWindowText(sdata);
	active = _ttoi(sdata);
	GetDlgItem(IDC_EDT_MI_HOR_WIDTH)->GetWindowText(sdata);
	width = _ttoi(sdata);
	GetDlgItem(IDC_EDT_MI_HOR_BP)->GetWindowText(sdata);
	bp = _ttoi(sdata);
	GetDlgItem(IDC_EDT_MI_HOR_FP)->GetWindowText(sdata);
	fp = _ttoi(sdata);

	sdata.Format(_T("%d"), active + width + bp + fp);
	GetDlgItem(IDC_EDT_MI_HOR_TOTAL)->SetWindowText(sdata);

	Lf_calcVSync();
}

void CModelInfo::Lf_calcVerResolution()
{
	CString sdata;
	int fp, active, bp, width;

	GetDlgItem(IDC_EDT_MI_VER_ACTIVE)->GetWindowText(sdata);
	active = _ttoi(sdata);
	GetDlgItem(IDC_EDT_MI_VER_WIDTH)->GetWindowText(sdata);
	width = _ttoi(sdata);
	GetDlgItem(IDC_EDT_MI_VER_BP)->GetWindowText(sdata);
	bp = _ttoi(sdata);
	GetDlgItem(IDC_EDT_MI_VER_FP)->GetWindowText(sdata);
	fp = _ttoi(sdata);

	sdata.Format(_T("%d"), active + width + bp + fp);
	GetDlgItem(IDC_EDT_MI_VER_TOTAL)->SetWindowText(sdata);

	Lf_calcVSync();
}

void CModelInfo::Lf_calcVSync()
{
	CString sdata;
	double mclk, vsync;
	int htotal, vtotal;

	GetDlgItem(IDC_EDT_MI_MCLOCK)->GetWindowText(sdata);
	mclk = _tstof(sdata);
	GetDlgItem(IDC_EDT_MI_HOR_TOTAL)->GetWindowText(sdata);
	htotal = _ttoi(sdata);
	GetDlgItem(IDC_EDT_MI_VER_TOTAL)->GetWindowText(sdata);
	vtotal = _ttoi(sdata);

	mclk = mclk * 1000 * 1000;
	vsync = mclk / (double)(htotal * vtotal);

	sdata.Format(_T("%.1f"), vsync);
	GetDlgItem(IDC_EDT_MI_VSYNC)->SetWindowText(sdata);
}

BOOL CModelInfo::Lf_checkAgingTempInfoChange()
{
	CString sdata;
	CString strMsg;

	int new_agingTimeHH, new_agingTimeMM;
	int new_tempUse, new_tempMin, new_tempMax;

	GetDlgItem(IDC_EDT_MI_AGING_TIME_HH)->GetWindowText(sdata);
	new_agingTimeHH = _ttoi(sdata);
	if (new_agingTimeHH != lpModelInfo->m_nAgingTimeHH)
	{
		sdata.Format(_T("'Aging Time(HH)' Changed : %d -> %d\r\n"), lpModelInfo->m_nAgingTimeHH, new_agingTimeHH);
		strMsg.Append(sdata);
	}

	GetDlgItem(IDC_EDT_MI_AGING_TIME_MM)->GetWindowText(sdata);
	new_agingTimeMM = _ttoi(sdata);
	if (new_agingTimeMM != lpModelInfo->m_nAgingTimeMM)
	{
		sdata.Format(_T("'Aging Time(MM)' Changed : %d -> %d\r\n"), lpModelInfo->m_nAgingTimeMM, new_agingTimeMM);
		strMsg.Append(sdata);
	}

	new_tempUse = m_cmbMiTemperatureUse.GetCurSel();
	if (new_tempUse != lpModelInfo->m_nOpeTemperatureUse)
	{
		CString strOldUse, strNewUse;
		if (lpModelInfo->m_nOpeTemperatureUse == FALSE)		strOldUse = _T("Disable");
		else												strOldUse = _T("Enable");
		if (new_tempUse == FALSE)							strNewUse = _T("Disable");
		else												strNewUse = _T("Enable");
		sdata.Format(_T("'Temperature Use' Changed : %s -> %s\r\n"), strOldUse, strNewUse);
		strMsg.Append(sdata);
	}

	GetDlgItem(IDC_EDT_MI_TEMPERATURE_MIN)->GetWindowText(sdata);
	new_tempMin = _ttoi(sdata);
	if (new_tempMin != lpModelInfo->m_nOpeTemperatureMin)
	{
		sdata.Format(_T("'Temperature Min' Changed : %d -> %d\r\n"), lpModelInfo->m_nOpeTemperatureMin, new_tempMin);
		strMsg.Append(sdata);
	}

	GetDlgItem(IDC_EDT_MI_TEMPERATURE_MAX)->GetWindowText(sdata);
	new_tempMax = _ttoi(sdata);
	if (new_tempMax != lpModelInfo->m_nOpeTemperatureMax)
	{
		sdata.Format(_T("'Temperature Max' Changed : %d -> %d\r\n"), lpModelInfo->m_nOpeTemperatureMax, new_tempMax);
		strMsg.Append(sdata);
	}

	// 변경사항이 없으면 TRUE Return 한다.
	if (strMsg.GetLength() == 0)
		return TRUE;


	CMessageQuestion msg_dlg;
	strMsg.Append(_T("Do you want change ?"));
	/*msg_dlg.m_strQMessage.Format(_T("%s"), strMsg);
	if (msg_dlg.DoModal() == IDOK)
	{
		return TRUE;
	}*/
	CPassword pw_dlg;
	if (pw_dlg.DoModal() == IDCANCEL)
	{
		return FALSE;
	}
	else
	{
		msg_dlg.m_strQMessage.Format(_T("%s"), strMsg);
	if (msg_dlg.DoModal() == IDOK)
	{
		return TRUE;
	}
	}


	return FALSE;
}


