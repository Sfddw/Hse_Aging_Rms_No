// PidInput.cpp: 구현 파일
//

#include "pch.h"
#include "HseAging.h"
#include "PidInput.h"
#include "afxdialogex.h"
#include "MessageError.h"
#include "MessageQuestion.h"
#include "HseAgingDlg.h"


// CPidInput 대화 상자

IMPLEMENT_DYNAMIC(CPidInput, CDialog)

CPidInput::CPidInput(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_PID_INPUT, pParent)
{
	m_pDefaultFont = new CFont();
	m_pDefaultFont->CreateFont(15, 6, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_nTargetRack = 0;
}

CPidInput::~CPidInput()
{
	if (m_pDefaultFont != NULL)
	{
		delete m_pDefaultFont;
	}
}

void CPidInput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STT_PI_RACK_ID, m_sttPiRackID);
	DDX_Control(pDX, IDC_STT_PI_LAYER1, m_sttPiLayer1);
	DDX_Control(pDX, IDC_STT_PI_LAYER2, m_sttPiLayer2);
	DDX_Control(pDX, IDC_STT_PI_LAYER3, m_sttPiLayer3);
	DDX_Control(pDX, IDC_STT_PI_LAYER4, m_sttPiLayer4);
	DDX_Control(pDX, IDC_STT_PI_LAYER5, m_sttPiLayer5);
	DDX_Control(pDX, IDC_LST_PI_MES_MESSAGE, m_lstPiMesMessage);
	DDX_Control(pDX, IDC_BTN_PI_SAVE_EXIT, m_btnPiSaveExit);
	DDX_Control(pDX, IDC_BTN_PI_CANCEL, m_btnPiCancel);
}


BEGIN_MESSAGE_MAP(CPidInput, CDialog)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_MBC_PI_RACK1, &CPidInput::OnBnClickedMbcPiRack1)
	ON_BN_CLICKED(IDC_MBC_PI_RACK2, &CPidInput::OnBnClickedMbcPiRack2)
	ON_BN_CLICKED(IDC_MBC_PI_RACK3, &CPidInput::OnBnClickedMbcPiRack3)
	ON_BN_CLICKED(IDC_MBC_PI_RACK4, &CPidInput::OnBnClickedMbcPiRack4)
	ON_BN_CLICKED(IDC_MBC_PI_RACK5, &CPidInput::OnBnClickedMbcPiRack5)
	ON_BN_CLICKED(IDC_MBC_PI_RACK6, &CPidInput::OnBnClickedMbcPiRack6)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH1, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch1)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH2, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch2)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH3, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch3)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH4, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch4)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH5, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch5)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH6, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch6)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH7, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch7)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH8, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch8)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH9, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch9)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH10, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch10)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH11, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch11)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH12, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch12)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH13, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch13)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH14, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch14)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH15, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch15)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER1_CH16, &CPidInput::OnEnSetfocusEdtPiPidLayer1Ch16)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH1, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch1)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH2, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch2)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH3, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch3)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH4, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch4)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH5, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch5)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH6, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch6)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH7, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch7)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH8, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch8)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH9, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch9)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH10, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch10)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH11, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch11)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH12, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch12)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH13, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch13)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH14, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch14)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH15, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch15)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER2_CH16, &CPidInput::OnEnSetfocusEdtPiPidLayer2Ch16)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH1, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch1)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH2, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch2)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH3, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch3)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH4, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch4)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH5, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch5)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH6, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch6)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH7, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch7)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH8, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch8)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH9, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch9)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH10, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch10)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH11, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch11)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH12, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch12)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH13, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch13)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH14, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch14)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH15, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch15)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER3_CH16, &CPidInput::OnEnSetfocusEdtPiPidLayer3Ch16)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH1, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch1)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH2, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch2)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH3, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch3)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH4, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch4)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH5, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch5)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH6, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch6)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH7, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch7)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH8, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch8)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH9, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch9)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH10, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch10)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH11, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch11)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH12, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch12)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH13, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch13)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH14, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch14)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH15, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch15)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER4_CH16, &CPidInput::OnEnSetfocusEdtPiPidLayer4Ch16)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH1, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch1)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH2, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch2)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH3, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch3)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH4, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch4)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH5, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch5)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH6, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch6)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH7, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch7)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH8, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch8)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH9, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch9)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH10, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch10)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH11, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch11)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH12, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch12)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH13, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch13)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH14, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch14)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH15, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch15)
	ON_EN_SETFOCUS(IDC_EDT_PI_PID_LAYER5_CH16, &CPidInput::OnEnSetfocusEdtPiPidLayer5Ch16)
	ON_BN_CLICKED(IDC_MBC_PI_PID_CLEAR, &CPidInput::OnBnClickedMbcPiPidClear)
	ON_BN_CLICKED(IDC_BTN_PI_MES_DSPM, &CPidInput::OnBnClickedBtnPiMesAgnin)
	ON_BN_CLICKED(IDC_BTN_PI_MES_DMIN, &CPidInput::OnBnClickedBtnPiMesPchk)
	ON_BN_CLICKED(IDC_BTN_PI_MES_DMOU, &CPidInput::OnBnClickedBtnPiMesAgnout)
	ON_BN_CLICKED(IDC_BTN_PI_SAVE_EXIT, &CPidInput::OnBnClickedBtnPiSaveExit)
	ON_BN_CLICKED(IDC_BTN_PI_CANCEL, &CPidInput::OnBnClickedBtnPiCancel)
	ON_BN_CLICKED(IDC_MBC_PI_CH_ALL_SELECT, &CPidInput::OnBnClickedMbcPiChAllSelect)
	ON_BN_CLICKED(IDC_MBC_PI_CH_ALL_CLEAR, &CPidInput::OnBnClickedMbcPiChAllClear)
END_MESSAGE_MAP()


// CPidInput 메시지 처리기


BOOL CPidInput::OnInitDialog()
{
	CDialog::OnInitDialog();
	lpSystemInfo = m_pApp->GetSystemInfo();
	lpModelInfo = m_pApp->GetModelInfo();
	lpInspWorkInfo = m_pApp->GetInspWorkInfo();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_pApp->Gf_writeMLog(_T("<TEST> 'Panel ID Input' Dialog Open"));

	// Dialog의 기본 FONT 설정.
	SendMessageToDescendants(WM_SETFONT, (WPARAM)m_pDefaultFont->GetSafeHandle(), 1, TRUE, FALSE);

	Lf_InitLocalValue();
	Lf_InitFontset();
	Lf_InitColorBrush();
	Lf_InitDialogDesign();

	Lf_ChangeColorRackButton(m_nTargetRack);
	Lf_updatePanelID();

	if (m_nMesAutoDMOU == MES_AGN_IN_AUTO || lpInspWorkInfo->m_nAgnIn == TRUE) // AGING시작시 AGN_IN
	{
		SetTimer(99, 1000, NULL);
		Lf_setExecuteMesAGNIN();
		lpInspWorkInfo->m_nAgnIn = FALSE;
		EndDialog(IDOK);
	}

	if (m_nMesAutoDMOU == MES_DMOU_MODE_AUTO || lpInspWorkInfo->m_nAgnRack != FALSE) // AGING온료시 AGN_OUT
	{
		//lpInspWorkInfo->m_nAgnOutYn[lpInspWorkInfo->m_nAgnRack] = TRUE;
		SetTimer(99, 1000, NULL);
		lpInspWorkInfo->m_nAgnRack = FALSE;
		Lf_setExecuteMesAGNOUT();
		EndDialog(IDOK);
	}

	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CPidInput::OnDestroy()
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

	CHseAgingDlg* pDlg = (CHseAgingDlg*)AfxGetMainWnd();
	if (pDlg != nullptr && lpInspWorkInfo->m_PidFlag == true)
	{
		pDlg->Lf_setAgingSTOP_PID(_ttoi(lpInspWorkInfo->m_StopRackID));
		lpInspWorkInfo->m_PidFlag = false;
	}
}

//BOOL CPidInput::PreTranslateMessage(MSG* pMsg)
//{
//    // TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
//    if (pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_F4)
//    {
//        if (::GetKeyState(VK_MENU) < 0) return TRUE;
//    }
//
//    // 일반 Key 동작에 대한 Event
//    if (pMsg->message == WM_KEYDOWN)          
//    {
//        switch (pMsg->wParam)
//        {
//        case VK_ESCAPE:
//            return TRUE;
//        case VK_RETURN:
//            if (GetDlgItem(IDC_EDT_PI_RACK_ID) == GetFocus())
//            {
//                m_pedtPannelID[LAYER_1][CH_1]->SetFocus();
//            }
//            else
//            {
//                Lf_setFocus();
//            }
//            return TRUE;
//        case VK_SPACE:
//            return TRUE;
//        }
//
//        // ASCII 범위 내의 키를 입력받으면
//        if ((pMsg->wParam >= 33) && (pMsg->wParam <= 'z'))
//        {
//            CString sdata;
//            sdata.Format(_T("%c"), pMsg->wParam);
//			if (sdata.CompareNoCase(_T("R")) == 0)
//			{
//				m_nSubKeyInData.Append(sdata);
//			}
//            m_nMainKeyInData.Append(sdata);
//            CString rackID;
//            CString channelID;
//			if (sdata.CompareNoCase(_T("R")) == 0)
//			{
//
//			}
//
//            // RACK과 CH를 분리
//            if (m_nMainKeyInData.GetLength() >= 10) // 10글자 이상일 때
//            {
//                int rackLength = 6; // "RACK01"의 길이 (6자)
//                rackID = m_nMainKeyInData.Left(rackLength); // RACK 부분 저장
//                channelID = m_nMainKeyInData.Right(m_nMainKeyInData.GetLength() - rackLength); // CH 부분 저장
//
//                if (m_nMainKeyInData.Left(4).CompareNoCase(_T("RACK")) == 0)
//                {
//                    Lf_checkBcrRackChIDInput(rackID, channelID);
//                }
//            }
//
//            // 모든 입력값을 Edit Control에 설정
//            CWnd* pFocusedWnd = GetFocus();
//            if (pFocusedWnd != nullptr)
//            {
//                CEdit* pEditControl = reinterpret_cast<CEdit*>(pFocusedWnd);
//                if (pEditControl != nullptr)
//                {
//                    // Edit Control에 현재 입력받은 값을 설정
//                    pEditControl->SetWindowText(m_nMainKeyInData); // 업데이트된 텍스트 설정
//                }
//            }
//
//            return TRUE; // 처리 완료
//        }
//    }
//
//    return CDialog::PreTranslateMessage(pMsg);
//}
BOOL CPidInput::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_F4)
	{
		if (::GetKeyState(VK_MENU) < 0)	return TRUE;
	}

	// 일반 Key 동작에 대한 Event
	if (pMsg->message == WM_KEYDOWN)
	{
		bool ScanIndex = false;
		CString sdata;
		
		switch (pMsg->wParam)
		{
		case VK_ESCAPE:
			return 1;
		case VK_RETURN:
			
			if (GetDlgItem(IDC_EDT_PI_RACK_ID) == GetFocus())
			{
				m_pedtPannelID[LAYER_1][CH_1]->SetFocus();
			}
			else if (m_nMainKeyInData.GetLength() == 14) // PID SCAN
			{
				bool P_Chk = false;
				if (m_pApp->m_bIsGmesConnect == FALSE)
				{
					Lf_addMessage(_T("MES not connected"));
					m_nMainKeyInData.Empty();
					CHseAgingDlg* pDlg = (CHseAgingDlg*)AfxGetMainWnd();

					int RackID = _ttoi(lpInspWorkInfo->m_RackID);
					int ChID = _ttoi(lpInspWorkInfo->m_ChID);
					RackID -= 1;
					pDlg->Lf_setAgingSTOP_PID(_ttoi(lpInspWorkInfo->m_StopRackID));

					pDlg->Lf_setAgingSTART_PID(RackID, ChID);
					return TRUE;
				}
				else
				{
					P_Chk = m_pApp->Gf_gmesSendHost_PCHK(HOST_PCHK, m_nMainKeyInData); // PCHK 값 전송
				}

				if (P_Chk == TRUE)
				{
					OnBnClickedBtnPiSaveExit_B(); // 저장
					if (m_nMainKeyInData != "")
					{
						sdata.Format(_T("PCHK OK [%s]"), m_nMainKeyInData);
						Lf_addMessage(sdata);

						CHseAgingDlg* pDlg = (CHseAgingDlg*)AfxGetMainWnd();
						pDlg->Lf_setAgingSTOP_PID(_ttoi(lpInspWorkInfo->m_RackID));

						pDlg->Lf_setAgingSTART_PID(_ttoi(lpInspWorkInfo->m_RackID), _ttoi(lpInspWorkInfo->m_ChID));


					}
				}
				else
				{
					if (m_nMainKeyInData != "")
					{
						m_pApp->pCommand->Gf_dio_setDIOWriteOutput(9, 1); // 부져 on

						sdata.Format(_T("PCHK ERROR [%s]"), m_nMainKeyInData);
						m_pApp->Gf_ShowMessageBox(sdata);

						lpInspWorkInfo->m_nDioOutputData = lpInspWorkInfo->m_nDioOutputData & ~DIO_OUT_BUZZER;

						m_pApp->pCommand->Gf_dio_setDIOWriteOutput(lpInspWorkInfo->m_nDioOutputData, lpInspWorkInfo->m_nDioOutputMode); // 부져 off

						sdata.Format(_T("PCHK ERROR [%s]"), m_nMainKeyInData);
						Lf_addMessage(sdata);

						CWnd* pFocusedWnd = GetFocus();
						if (pFocusedWnd != nullptr)
						{
							// Edit Control의 포인터로 캐스팅
							CEdit* pEditControl = reinterpret_cast<CEdit*>(pFocusedWnd);
							if (pEditControl != nullptr)
							{
								pEditControl->SetWindowText(_T(""));
							}
						}

					}
				}


				m_nMainKeyInData.Empty(); // 입력값 초기화

				return 1;
			}
			else if (m_nMainKeyInData.GetLength() == 10) // RACK ID SCAN
			{
				ScanIndex = true;

				CString sdata;
				CString rackId = m_nMainKeyInData.Left(6);       // "RACK01"
				CString chTag = m_nMainKeyInData.Mid(6, 2);       // "CH"
				CString chId = m_nMainKeyInData.Right(2);         // "YY"

				if (rackId.Left(4).CompareNoCase(_T("RACK")) == 0 && chTag.CompareNoCase(_T("CH")) == 0)
				{
					Lf_checkBcrRackChIDInput(rackId, chId);

					lpInspWorkInfo->m_RackID = rackId.Right(2);
					lpInspWorkInfo->m_ChID = chId.Right(2);

					/*lpInspWorkInfo->m_StopRackID = rackId.Right(2);*/

					/*(lpInspWorkInfo->m_RackID).Right(2);
					(lpInspWorkInfo->m_ChID).Right(2);*/
				}
				else
				{
					sdata.Format(_T("RACK BARCODE RESCAN"));
					m_pApp->Gf_ShowMessageBox(sdata);
				}

				m_nMainKeyInData.Empty();
				return 1;
			}
			
			else
			{
				if (ScanIndex == true)
				{
					ScanIndex = false;
					return 1;
				}
				else
				{
					CWnd* pFocusedWnd = GetFocus();
					if (pFocusedWnd != nullptr)
					{
						// Edit Control의 포인터로 캐스팅
						CEdit* pEditControl = reinterpret_cast<CEdit*>(pFocusedWnd);
						if (pEditControl != nullptr)
						{
							pEditControl->SetWindowText(_T(""));
						}
					}
					m_nMainKeyInData.Empty(); // 입력값 초기화
				}
			}
			return 1;
		case VK_SPACE:
			return 1;
		}
		if ((pMsg->wParam >= 33) && (pMsg->wParam <= 'z'))
		{
			sdata.Format(_T("%c"), pMsg->wParam);
			m_nMainKeyInData.Append(sdata); // 바코드 스캔값 저장
			if (m_nMainKeyInData.GetLength() >= 14)
			{
				CWnd* pFocusedWnd = GetFocus();
				if (pFocusedWnd != nullptr)
				{
					// Edit Control의 포인터로 캐스팅
					CEdit* pEditControl = reinterpret_cast<CEdit*>(pFocusedWnd);
					if (pEditControl != nullptr)
					{
						pEditControl->SetWindowText(m_nMainKeyInData);
					}
				}
			}
			return 1;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CPidInput::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  여기서 DC의 특성을 변경합니다.
	switch (nCtlColor)
	{
		case CTLCOLOR_MSGBOX:
			break;
		case CTLCOLOR_EDIT:
			if ((pWnd->GetDlgCtrlID() == IDC_EDT_PI_RACK_ID)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER1_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER2_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER3_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER4_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_EDT_PI_PID_LAYER5_CH16)
				)
			{
				if (Lf_checkPIDMesInfo(pWnd->GetDlgCtrlID()) != 0)
				{
					// MES 완료 색상
					pDC->SetBkColor(COLOR_SKYBLUE);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_SKYBLUE];
				}
				if (Lf_checkPIDFocusInfo(pWnd->GetDlgCtrlID()) != 0)
				{
					// Focus 위치 색상
					pDC->SetBkColor(COLOR_JADEBLUE);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_JADEBLUE];
				}
				if (Lf_checkPIDErrorInfo(pWnd->GetDlgCtrlID()) != 0)
				{
					// PID ERROR 색상
					pDC->SetBkColor(COLOR_RED);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED];
				}
				if (Lf_checkPIDValidInfo(pWnd->GetDlgCtrlID()) != 0)
				{
					// PID 입력 완료 색상
					pDC->SetBkColor(COLOR_LIGHT_GREEN);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_LIGHT_GREEN];
				}

				// PID 입력 대기 색상
				pDC->SetBkColor(COLOR_WHITE);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_WHITE];
			}

			break;
		case CTLCOLOR_LISTBOX:
			if (pWnd->GetDlgCtrlID() == IDC_LST_PI_MES_MESSAGE)
			{
				pDC->SetBkColor(COLOR_WHITE);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_WHITE];
			}
			break;
		case CTLCOLOR_SCROLLBAR:
			break;
		case CTLCOLOR_BTN:
			break;
		case CTLCOLOR_STATIC:		// Static, CheckBox control
			if ((pWnd->GetDlgCtrlID() == IDC_STATIC)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_RIGHT_SIDE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LEFT_SIDE)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LAYER1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LAYER2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LAYER3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LAYER4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LAYER5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LEFT_LAYER1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LEFT_LAYER2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LEFT_LAYER3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LEFT_LAYER4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_LEFT_LAYER5)
				)
			{
				pDC->SetBkColor(COLOR_SKYBLUE);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_SKYBLUE];
			}
			if (pWnd->GetDlgCtrlID() == IDC_STT_PI_TITLE)
			{
				pDC->SetBkColor(COLOR_DARK_NAVY);
				pDC->SetTextColor(COLOR_WHITE);
				return m_Brush[COLOR_IDX_DARK_NAVY];
			}
			if ((pWnd->GetDlgCtrlID() == IDC_GRP_PI_PID_OPERATION)
				|| (pWnd->GetDlgCtrlID() == IDC_GRP_PI_MES_OPERATION)
				)
			{
				pDC->SetBkColor(COLOR_GRAY192);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_GRAY192];
			}
			if ((pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH1) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH3) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH5) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH7) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH9) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH11) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH13) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH15) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER1_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH1) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH3) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH5) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH7) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH9) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH11) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH13) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH15) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER2_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH1) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH3) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH5) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH7) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH9) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH11) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH13) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH15) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER3_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH1) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH3) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH5) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH7) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH9) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH11) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH13) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH15) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER4_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH1) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH3) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH5) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH7) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH9) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH11) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH13) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH15) || (pWnd->GetDlgCtrlID() == IDC_CHK_PI_LAYER5_CH16)
				)
			{
				pDC->SetBkColor(COLOR_GRAY192);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_GRAY192];
			}
			if ((pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER1_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER2_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER3_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER4_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH7)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH9)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH11)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH13)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH15)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_PI_MSG_LAYER5_CH16)
				)
			{
				pDC->SetBkColor(COLOR_GRAY159);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_GRAY159];

			}
			break;
	}
	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.
	return hbr;
}


void CPidInput::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.
	CRect rect, rectOri;
	GetClientRect(&rect);
	rectOri = rect;

	rect.bottom = 70;
	dc.FillSolidRect(rect, COLOR_DARK_NAVY);

	rect.top = rect.bottom;
	rect.bottom = rectOri.bottom;
	dc.FillSolidRect(rect, COLOR_GRAY192);
}


void CPidInput::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == 99)
	{
		KillTimer(99);

		/*if (Lf_setExecuteMesAGNOUT() == TRUE)
		{
			CDialog::OnOK();
		}*/
		CDialog::OnOK();
	}
	CDialog::OnTimer(nIDEvent);
}

void CPidInput::OnBnClickedMbcPiRack1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_ChangeColorRackButton(RACK_1);
	Lf_updatePanelID();
}


void CPidInput::OnBnClickedMbcPiRack2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_ChangeColorRackButton(RACK_2);
	Lf_updatePanelID();
}


void CPidInput::OnBnClickedMbcPiRack3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_ChangeColorRackButton(RACK_3);
	Lf_updatePanelID();
}


void CPidInput::OnBnClickedMbcPiRack4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_ChangeColorRackButton(RACK_4);
	Lf_updatePanelID();
}


void CPidInput::OnBnClickedMbcPiRack5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_ChangeColorRackButton(RACK_5);
	Lf_updatePanelID();
}


void CPidInput::OnBnClickedMbcPiRack6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_ChangeColorRackButton(RACK_6);
	Lf_updatePanelID();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_1;
	Lf_InvalidateWindow();

	if (lpInspWorkInfo->m_SendRackID == true)
	{
		//Lf_Send_checkBcrRackChIDInput(lpInspWorkInfo->m_RackID, lpInspWorkInfo->m_ChID);
		Lf_checkBcrRackChIDInput(lpInspWorkInfo->m_RackID, lpInspWorkInfo->m_ChID);
		lpInspWorkInfo->m_SendRackID = false;
	}
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_2;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_3;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_4;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_5;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_6;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch7()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_7;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch8()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_8;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch9()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_9;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch10()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_10;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch11()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_11;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch12()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_12;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch13()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_13;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch14()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_14;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch15()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_15;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer1Ch16()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_1;
	m_nSelChannel = CH_16;
	Lf_InvalidateWindow();
}

void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_1;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_2;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_3;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_4;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_5;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_6;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch7()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_7;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch8()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_8;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch9()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_9;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch10()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_10;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch11()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_11;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch12()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_12;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch13()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_13;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch14()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_14;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch15()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_15;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer2Ch16()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_2;
	m_nSelChannel = CH_16;
	Lf_InvalidateWindow();
}

void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_1;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_2;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_3;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_4;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_5;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_6;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch7()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_7;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch8()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_8;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch9()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_9;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch10()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_10;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch11()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_11;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch12()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_12;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch13()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_13;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch14()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_14;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch15()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_15;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer3Ch16()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_3;
	m_nSelChannel = CH_16;
	Lf_InvalidateWindow();
}

void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_1;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_2;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_3;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_4;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_5;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_6;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch7()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_7;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch8()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_8;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch9()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_9;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch10()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_10;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch11()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_11;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch12()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_12;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch13()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_13;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch14()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_14;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch15()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_15;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer4Ch16()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_4;
	m_nSelChannel = CH_16;
	Lf_InvalidateWindow();
}

void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_1;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_2;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_3;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_4;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_5;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_6;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch7()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_7;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch8()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_8;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch9()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_9;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch10()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_10;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch11()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_11;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch12()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_12;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch13()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_13;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch14()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_14;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch15()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_15;
	Lf_InvalidateWindow();
}


void CPidInput::OnEnSetfocusEdtPiPidLayer5Ch16()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nSelLayer = LAYER_5;
	m_nSelChannel = CH_16;
	Lf_InvalidateWindow();
}


void CPidInput::OnBnClickedBtnPiMesAgnin()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setExecuteMesAGNIN();
}


void CPidInput::OnBnClickedBtnPiMesPchk() // PCHK
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setExecuteMesPCHK();
}


void CPidInput::OnBnClickedBtnPiMesAgnout()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setExecuteMesAGNOUT();
}

void CPidInput::OnBnClickedMbcPiPidClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sLog, sdata = _T("");

	m_pApp->Gf_writeMLog(_T("<PID> 'PID ALL Clear' Button Click."));

	CMessageQuestion msg_dlg;
	msg_dlg.m_strQMessage = _T("Do you want clear all ID ?");
	msg_dlg.m_strLButton = _T("YES");
	msg_dlg.m_strRButton = _T("NO");

	if (msg_dlg.DoModal() == IDOK)
	{
		CString sSection, sKey, sValue;
		sSection.Format(_T("MES_PID_RACK%d"), m_nSelRack + 1);

		for(int layer=0; layer<MAX_LAYER; layer++)
		{
			for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
			{
				lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch].Empty();
				m_pedtPannelID[layer][ch]->SetWindowText(_T(""));

				// PID Save
				sKey.Format(_T("RACK%d_LAYER%d_CH%d"), m_nSelRack + 1, layer + 1, ch + 1);
				Write_MesPIDInfo(sSection, sKey, _T(""));
			}
		}
		m_pedtPannelID[0][0]->SetFocus();
	}
}

void CPidInput::OnBnClickedMbcPiChAllSelect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_AllChannelSelect(TRUE);

	// 2025-01-17 PDH. 채널 전체 선택 시 PID 입력 포커스 CH1로 이동
	m_pedtPannelID[LAYER_1][CH_1]->SetFocus();
}


void CPidInput::OnBnClickedMbcPiChAllClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_AllChannelSelect(FALSE);
}

void CPidInput::OnBnClickedBtnPiSaveExit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CMessageQuestion msg_dlg;
	msg_dlg.m_strQMessage = _T("Do you want save ?");
	msg_dlg.m_strLButton = _T("YES");
	msg_dlg.m_strRButton = _T("NO");

	if (msg_dlg.DoModal() == IDOK)
	{
		CString sSection, sKey, sValue;

		sSection.Format(_T("MES_PID_RACK%d"), m_nSelRack + 1);

		// RACK ID 저장
		GetDlgItem(IDC_EDT_PI_RACK_ID)->GetWindowText(sValue);

		lpInspWorkInfo->m_sRackID[m_nSelRack] = sValue;
		sKey.Format(_T("RACK%d_BCR_ID"), m_nSelRack + 1);
		Write_SysIniFile(_T("SYSTEM"), sKey, sValue);

		// CHANNEL PID 저장
		for (int layer = 0; layer < MAX_LAYER; layer++)
		{
			for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
			{
				// Channel Use Save
				sKey.Format(_T("RACK%d_LAYER%d_CH%d_USE"), m_nSelRack + 1, layer + 1, ch + 1);
				lpInspWorkInfo->m_bMesChannelUse[m_nSelRack][layer][ch] = m_pChkChannelUse[layer][ch]->GetCheck();
				sValue.Format(_T("%d"), m_pChkChannelUse[layer][ch]->GetCheck());
				Write_MesPIDInfo(sSection, sKey, sValue);

				// Channel PID Save
				sKey.Format(_T("RACK%d_LAYER%d_CH%d"), m_nSelRack + 1, layer + 1, ch + 1);
				m_pedtPannelID[layer][ch]->GetWindowText(sValue);
				lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch] = sValue;
				Write_MesPIDInfo(sSection, sKey, sValue);
			}
		}

		//CDialog::OnOK();
	}
}

void CPidInput::OnBnClickedBtnPiSaveExit_B()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 
	// CString sSection, sKey, sValue;

	CString sSection, sKey, sValue;

	sSection.Format(_T("MES_PID_RACK%d"), m_nSelRack + 1);

	// RACK ID 저장
	GetDlgItem(IDC_EDT_PI_RACK_ID)->GetWindowText(sValue);

	lpInspWorkInfo->m_sRackID[m_nSelRack] = sValue;
	sKey.Format(_T("RACK%d_BCR_ID"), m_nSelRack + 1);
	Write_SysIniFile(_T("SYSTEM"), sKey, sValue);

	// CHANNEL PID 저장
	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			// Channel Use Save
			sKey.Format(_T("RACK%d_LAYER%d_CH%d_USE"), m_nSelRack + 1, layer + 1, ch + 1);
			lpInspWorkInfo->m_bMesChannelUse[m_nSelRack][layer][ch] = m_pChkChannelUse[layer][ch]->GetCheck();
			sValue.Format(_T("%d"), m_pChkChannelUse[layer][ch]->GetCheck());
			Write_MesPIDInfo(sSection, sKey, sValue);

			// Channel PID Save
			sKey.Format(_T("RACK%d_LAYER%d_CH%d"), m_nSelRack + 1, layer + 1, ch + 1);
			m_pedtPannelID[layer][ch]->GetWindowText(sValue);
			lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch] = sValue;
			Write_MesPIDInfo(sSection, sKey, sValue);
		}
	}
}


void CPidInput::OnBnClickedBtnPiCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialog::OnCancel();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPidInput::Lf_InitLocalValue()
{
	memset(m_bPIDRuleError, FALSE, sizeof(m_bPIDRuleError));
	m_nSelRack = RACK_1;

	////////////////////////////////////////////////////////////////////////////////////////////
	int layer;
	int i = 0;
	m_pBtnPidRack[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_PI_RACK1);
	m_pBtnPidRack[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_PI_RACK2);
	m_pBtnPidRack[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_PI_RACK3);
	m_pBtnPidRack[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_PI_RACK4);
	m_pBtnPidRack[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_PI_RACK5);
	m_pBtnPidRack[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_PI_RACK6);

	layer = LAYER_1;
	i = 0;
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH1);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH2);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH3);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH4);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH5);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH6);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH7);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH8);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH9);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH10);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH11);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH12);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH13);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH14);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH15);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER1_CH16);
	i = 0;
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH1);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH2);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH3);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH4);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH5);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH6);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH7);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH8);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH9);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH10);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH11);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH12);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH13);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH14);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH15);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH16);
	i = 0;
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH1);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH2);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH3);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH4);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH5);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH6);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH7);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH8);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH9);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH10);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH11);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH12);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH13);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH14);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH15);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER1_CH16);

	i = 0;
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH1);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH2);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH3);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH4);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH5);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH6);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH7);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH8);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH9);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH10);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH11);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH12);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH13);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH14);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH15);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER1_CH16);

	i = 0;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH1;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH2;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH3;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH4;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH5;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH6;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH7;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH8;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH9;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH10;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH11;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH12;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH13;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH14;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH15;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER1_CH16;


	layer = LAYER_2;
	i = 0;
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH1);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH2);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH3);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH4);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH5);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH6);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH7);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH8);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH9);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH10);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH11);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH12);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH13);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH14);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH15);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER2_CH16);
	i = 0;
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH1);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH2);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH3);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH4);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH5);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH6);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH7);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH8);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH9);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH10);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH11);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH12);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH13);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH14);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH15);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH16);
	i = 0;
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH1);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH2);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH3);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH4);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH5);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH6);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH7);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH8);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH9);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH10);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH11);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH12);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH13);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH14);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH15);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER2_CH16);

	i = 0;
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH1);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH2);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH3);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH4);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH5);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH6);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH7);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH8);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH9);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH10);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH11);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH12);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH13);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH14);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH15);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER2_CH16);

	i = 0;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH1;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH2;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH3;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH4;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH5;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH6;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH7;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH8;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH9;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH10;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH11;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH12;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH13;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH14;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH15;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER2_CH16;

	layer = LAYER_3;
	i = 0;
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH1);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH2);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH3);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH4);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH5);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH6);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH7);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH8);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH9);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH10);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH11);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH12);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH13);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH14);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH15);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER3_CH16);
	i = 0;
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH1);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH2);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH3);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH4);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH5);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH6);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH7);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH8);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH9);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH10);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH11);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH12);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH13);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH14);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH15);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH16);
	i = 0;
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH1);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH2);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH3);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH4);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH5);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH6);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH7);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH8);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH9);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH10);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH11);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH12);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH13);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH14);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH15);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER3_CH16);

	i = 0;
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH1);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH2);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH3);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH4);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH5);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH6);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH7);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH8);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH9);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH10);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH11);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH12);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH13);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH14);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH15);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER3_CH16);

	i = 0;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH1;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH2;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH3;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH4;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH5;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH6;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH7;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH8;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH9;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH10;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH11;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH12;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH13;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH14;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH15;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER3_CH16;

	layer = LAYER_4;
	i = 0;
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH1);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH2);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH3);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH4);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH5);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH6);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH7);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH8);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH9);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH10);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH11);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH12);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH13);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH14);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH15);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER4_CH16);
	i = 0;
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH1);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH2);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH3);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH4);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH5);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH6);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH7);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH8);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH9);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH10);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH11);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH12);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH13);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH14);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH15);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH16);
	i = 0;
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH1);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH2);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH3);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH4);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH5);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH6);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH7);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH8);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH9);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH10);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH11);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH12);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH13);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH14);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH15);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER4_CH16);

	i = 0;
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH1);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH2);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH3);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH4);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH5);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH6);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH7);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH8);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH9);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH10);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH11);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH12);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH13);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH14);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH15);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER4_CH16);

	i = 0;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH1;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH2;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH3;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH4;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH5;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH6;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH7;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH8;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH9;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH10;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH11;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH12;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH13;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH14;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH15;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER4_CH16;

	layer = LAYER_5;
	i = 0;
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH1);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH2);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH3);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH4);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH5);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH6);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH7);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH8);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH9);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH10);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH11);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH12);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH13);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH14);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH15);
	m_pChkChannelUse[layer][i++] = (CButton*)GetDlgItem(IDC_CHK_PI_LAYER5_CH16);
	i = 0;
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH1);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH2);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH3);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH4);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH5);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH6);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH7);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH8);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH9);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH10);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH11);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH12);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH13);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH14);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH15);
	m_pSttChInfo[layer][i++] = (CStatic*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH16);
	i = 0;
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH1);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH2);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH3);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH4);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH5);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH6);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH7);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH8);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH9);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH10);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH11);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH12);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH13);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH14);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH15);
	m_pedtPannelID[layer][i++] = (CEdit*)GetDlgItem(IDC_EDT_PI_PID_LAYER5_CH16);

	i = 0;
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH1);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH2);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH3);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH4);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH5);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH6);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH7);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH8);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH9);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH10);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH11);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH12);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH13);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH14);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH15);
	m_pedtBcrChID[layer][i++] = (CEdit*)GetDlgItem(IDC_STT_PI_MSG_LAYER5_CH16);

	i = 0;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH1;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH2;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH3;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH4;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH5;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH6;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH7;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH8;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH9;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH10;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH11;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH12;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH13;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH14;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH15;
	m_nedtDlgCtrlID[layer][i++] = IDC_EDT_PI_PID_LAYER5_CH16;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Channel 별 Check 상태 초기화
	////////////////////////////////////////////////////////////////////////////////////////////
	Lf_loadMesStatusInfo();

}

void CPidInput::Lf_InitFontset()
{
	m_Font[0].CreateFont(150, 70, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[1].CreateFont(44, 20, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_PI_TITLE)->SetFont(&m_Font[1]);

	m_Font[2].CreateFont(32, 14, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_EDT_PI_RACK_ID)->SetFont(&m_Font[2]);

	m_Font[3].CreateFont(24, 11, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_MBC_PI_RACK1)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_MBC_PI_RACK2)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_MBC_PI_RACK3)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_MBC_PI_RACK4)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_MBC_PI_RACK5)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_MBC_PI_RACK6)->SetFont(&m_Font[3]);

	m_Font[4].CreateFont(19, 8, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_PI_LAYER1)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_STT_PI_LAYER2)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_STT_PI_LAYER3)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_STT_PI_LAYER4)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_STT_PI_LAYER5)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_STT_PI_LEFT_LAYER1)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_STT_PI_LEFT_LAYER2)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_STT_PI_LEFT_LAYER3)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_STT_PI_LEFT_LAYER4)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_STT_PI_LEFT_LAYER5)->SetFont(&m_Font[4]);

	GetDlgItem(IDC_GRP_PI_PID_OPERATION)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_GRP_PI_MES_OPERATION)->SetFont(&m_Font[4]);

	GetDlgItem(IDC_MBC_PI_PID_CLEAR)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_MBC_PI_CH_ALL_SELECT)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_MBC_PI_CH_ALL_CLEAR)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_BTN_PI_MES_DSPM)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_BTN_PI_MES_DMIN)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_BTN_PI_MES_DMOU)->SetFont(&m_Font[4]);

	GetDlgItem(IDC_BTN_PI_SAVE_EXIT)->SetFont(&m_Font[4]);
	GetDlgItem(IDC_BTN_PI_CANCEL)->SetFont(&m_Font[4]);

	m_Font[5].CreateFont(16, 7, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

void CPidInput::Lf_InitColorBrush()
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
	m_Brush[COLOR_IDX_YELLOW].CreateSolidBrush(COLOR_YELLOW);
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

void CPidInput::Lf_InitDialogDesign()
{
	SetWindowTheme(GetDlgItem(IDC_MBC_PI_RACK1)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_MBC_PI_RACK2)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_MBC_PI_RACK3)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_MBC_PI_RACK4)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_MBC_PI_RACK5)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_MBC_PI_RACK6)->GetSafeHwnd(), L"", L"");

	SetWindowTheme(GetDlgItem(IDC_GRP_PI_PID_OPERATION)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_GRP_PI_MES_OPERATION)->GetSafeHwnd(), L"", L"");

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			SetWindowTheme(m_pChkChannelUse[layer][ch]->GetSafeHwnd(), L"", L"");
		}
	}

	m_pApp->Gf_setGradientStatic02(&m_sttPiRackID, &m_Font[2], FALSE);

	m_btnPiSaveExit.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_OX_O));
	m_btnPiCancel.SetIcon(AfxGetApp()->LoadIcon(IDI_ICON_OX_X));
}

void CPidInput::Lf_ChangeColorRackButton(int selectRack)
{
	m_nSelRack = selectRack;

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		if (rack == selectRack)
		{
			m_pBtnPidRack[rack]->EnableWindowsTheming(FALSE);
			m_pBtnPidRack[rack]->SetFaceColor(COLOR_BUTTON_SEL);
			m_pBtnPidRack[rack]->SetTextColor(COLOR_GRAY224);
		}
		else
		{
			m_pBtnPidRack[rack]->EnableWindowsTheming(FALSE);
			m_pBtnPidRack[rack]->SetFaceColor(COLOR_BUTTON_DARK);
			m_pBtnPidRack[rack]->SetTextColor(COLOR_GRAY224);
		}
	}
}

void CPidInput::Lf_updatePanelID()
{
	GetDlgItem(IDC_EDT_PI_RACK_ID)->SetWindowText(lpInspWorkInfo->m_sRackID[m_nSelRack]);

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			m_pChkChannelUse[layer][ch]->SetCheck(lpInspWorkInfo->m_bMesChannelUse[m_nSelRack][layer][ch]);
			//m_pedtPannelID[layer][ch]->SetWindowText(lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch]);
			m_pedtPannelID[layer][ch]->SetWindowText(lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch]);
			if (m_pedtBcrChID[layer][ch] != nullptr) {
				m_pedtBcrChID[layer][ch]->SetWindowText(lpInspWorkInfo->m_sMesBcrChID[m_nSelRack][layer][ch]);
			}
			else {
				// 적절한 에러 처리
				//AfxMessageBox(_T("Edit control is not initialized."));
			}
			//m_pedtBcrChID[layer][ch]->SetWindowText(lpInspWorkInfo->m_sMesBcrChID[m_nSelRack][layer][ch]);
		}
	}
}

void CPidInput::Lf_addMessage(CString msg)
{
	m_lstPiMesMessage.SetCurSel(m_lstPiMesMessage.AddString(msg));
}


void CPidInput::Lf_setFocus()
{
	CString sdata;
	int pidInputMode;

	Read_SysIniFile(_T("SYSTEM"), _T("PID_INPUT_MODE"), &pidInputMode);

	if (pidInputMode == 0)
	{
		while (1)
		{
			m_nSelChannel++;
			if ((m_nSelLayer == LAYER_1) && (m_nSelChannel >= MAX_LAYER_CHANNEL))
				return;

			if (m_nSelChannel <= 8)
			{
				if ((m_nSelChannel % 2) == 0)
				{
					if (m_nSelLayer == LAYER_5)
					{
						if (m_nSelChannel == CH_9)
						{
							m_nSelLayer = LAYER_5;
						}
						else
						{
							m_nSelLayer = 0;
						}
					}
					else
					{
						m_nSelLayer++;
						m_nSelChannel = m_nSelChannel - 2;
					}
				}
			}
			else
			{
				if ((m_nSelChannel % 2) == 0)
				{
					if (m_nSelLayer == LAYER_1)
					{
						m_nSelLayer = LAYER_5;
					}
					else
					{
						m_nSelLayer--;
						m_nSelChannel = m_nSelChannel - 2;
					}
				}
			}

			if (m_pChkChannelUse[m_nSelLayer][m_nSelChannel]->GetCheck() == TRUE)
				break;
		}
	}
	else
	{
		while (1)
		{
			m_nSelChannel++;
			if ((m_nSelLayer == LAYER_1) && (m_nSelChannel >= MAX_LAYER_CHANNEL))
				return;

			if (m_nSelChannel == 8)
			{
				if (m_nSelLayer == LAYER_5)
				{
				}
				else
				{
					m_nSelLayer++;
					m_nSelChannel = CH_1;
				}
			}
			else
			{
				if (m_nSelChannel == 16)
				{
					m_nSelLayer--;
					m_nSelChannel = CH_9;
				}
			}

			if (m_pChkChannelUse[m_nSelLayer][m_nSelChannel]->GetCheck() == TRUE)
				break;
		}
	}


	m_pedtPannelID[m_nSelLayer][m_nSelChannel]->SetFocus();
	m_pedtPannelID[m_nSelLayer][m_nSelChannel]->SetSel(0, m_pedtPannelID[m_nSelLayer][m_nSelChannel]->GetWindowTextLength());
}

void CPidInput::Lf_loadMesStatusInfo()
{
	CString section, key;

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			section.Format(_T("RACK-%d"), m_nSelRack + 1);
			key.Format(_T("LAYER%d-CH%d"), layer + 1, ch + 1);
			Read_MesStatusInfo(section, key, &lpInspWorkInfo->m_nMesDspmOK[m_nSelRack][layer][ch]);
		}
	}
}

void CPidInput::Lf_saveMesStatusInfo()
{
	CString section, key;

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			section.Format(_T("RACK-%d"), m_nSelRack + 1);
			key.Format(_T("LAYER%d-CH%d"), layer + 1, ch + 1);

			Write_MesStatusInfo(section, key, lpInspWorkInfo->m_nMesDspmOK[m_nSelRack][layer][ch]);
		}
	}
}

void CPidInput::Lf_clearMesStatusInfo()
{
	CString section, key;

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			// AGN_IN
			section.Format(_T("RACK-%d"), m_nSelRack + 1);
			key.Format(_T("LAYER%d-CH%d"), layer + 1, ch + 1);
			lpInspWorkInfo->m_nMesDspmOK[m_nSelRack][layer][ch] = 0;
			Write_MesStatusInfo(section, key, 0);
		}
	}
}

int CPidInput::Lf_checkPIDMesInfo(int ctrl_id)
{
	CString bcr_info;

	GetDlgItem(ctrl_id)->GetWindowText(bcr_info);


	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			if (ctrl_id == m_nedtDlgCtrlID[layer][ch])
			{
				if (lpInspWorkInfo->m_nMesDspmOK[m_nSelRack][layer][ch] == 1)
					return TRUE;
				else
					return FALSE;
			}
		}
	}

	return FALSE;
}

int CPidInput::Lf_checkPIDErrorInfo(int ctrl_id)
{
	int isMatch=FALSE, match_layer, match_ch;
	CString bcr_info;

	GetDlgItem(ctrl_id)->GetWindowText(bcr_info);

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			if (ctrl_id == m_nedtDlgCtrlID[layer][ch])
			{
				isMatch = TRUE;
				match_layer = layer;
				match_ch = ch;
			}
		}

		if (isMatch == TRUE)
			break;
	}
	if (isMatch == FALSE)
		return 0;

	if (bcr_info.IsEmpty() == FALSE)
	{
		/////////////////////////////////////////////////////////////////////
		// BCR에 PID CHECk CODE가 없으면 Error로 처리한다.
		/////////////////////////////////////////////////////////////////////
		if (bcr_info.Find(lpModelInfo->m_sPanelIDCode) == -1)
		{
			m_bPIDRuleError[match_layer][match_ch] = TRUE;
			return -1;
		}

		/////////////////////////////////////////////////////////////////////
		// 중복으로 입력된 Code가 있으면 Error로 처리한다.
		/////////////////////////////////////////////////////////////////////
		CString sBuff;
		for (int layer = 0; layer < MAX_LAYER; layer++)
		{
			for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
			{
				if ((layer==match_layer) && (ch == match_ch))	// 자신의 ID 위치는 Skip 한다.
					continue;

				if (GetDlgItem(m_nedtDlgCtrlID[layer][ch])->IsWindowVisible() == TRUE)
				{
					GetDlgItem(m_nedtDlgCtrlID[layer][ch])->GetWindowText(sBuff);

					if (bcr_info == sBuff)
					{
						m_bPIDRuleError[match_layer][match_ch] = TRUE;
						return -1;
					}
				}
			}
		}
		/////////////////////////////////////////////////////////////////////
	}

	m_bPIDRuleError[match_layer][match_ch] = FALSE;
	return 0;
}

int CPidInput::Lf_checkPIDValidInfo(int ctrl_id)
{
	CString bcr_info;

	if (GetDlgItem(ctrl_id)->GetWindowTextLength() != 0)
		return 1;

	return 0;
}

int CPidInput::Lf_checkPIDFocusInfo(int ctrl_id)
{
	if (GetDlgItem(ctrl_id) == GetFocus())
		return 1;

	return 0;
}

void CPidInput::Lf_InvalidateWindow()
{
	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			GetDlgItem(m_nedtDlgCtrlID[layer][ch])->Invalidate(FALSE);
		}
	}
}

BOOL CPidInput::Lf_isExistErrorChannel()
{
	CMessageError err_dlg;

	////////////////////////////////////////////////////////////////
	// PID Rule Error 조건이 있는지 확인한다.
	////////////////////////////////////////////////////////////////
	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			if (m_bPIDRuleError[layer][ch] == TRUE)
			{
				err_dlg.m_strEMessage.Format(_T("'LAYER%d-CH%d' PID was entered incorrectly."), layer + 1, ch + 1);
				err_dlg.DoModal();

				return TRUE;
			}
		}
	}
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	// ICC 값이 있으나 PID 입력되지 않은 조건이 있는지 확인한다.
	// ICC 값이 없지만 PID가 입력된 조건이 있는지 확인한다.
	////////////////////////////////////////////////////////////////
	CString pid_buf;
	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			GetDlgItem(m_nedtDlgCtrlID[layer][ch])->GetWindowText(pid_buf);

			if (lpInspWorkInfo->m_nMeasICC[m_nSelRack][layer][ch] < 50)
			{
				// ICC 측정되지 않으나 PID 입력된 경우.
				if (pid_buf.GetLength() != 0)
				{
					err_dlg.m_strEMessage.Format(_T("'LAYER%d-CH%d' PID Input Error.  ICC-%dmA  PID-%s"), layer + 1, ch + 1, lpInspWorkInfo->m_nMeasICC[m_nSelRack][ch], pid_buf);
					err_dlg.DoModal();

					return TRUE;
				}
			}
			else
			{
				// ICC 측정되었지만 PID 입력되지 않은 경우.
				if (pid_buf.GetLength() == 0)
				{
					err_dlg.m_strEMessage.Format(_T("'LAYER%d-CH%d' PID Input Error.  ICC-%dmA  PID-NULL"), layer + 1, ch + 1, lpInspWorkInfo->m_nMeasICC[m_nSelRack][ch]);
					err_dlg.DoModal();

					return TRUE;
				}
			}
		}
	}
	////////////////////////////////////////////////////////////////


	return FALSE;
}


void CPidInput::Lf_AllChannelSelect(int onoff)
{
	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			m_pChkChannelUse[layer][ch]->SetCheck(onoff);
		}
	}
}

BOOL CPidInput::Lf_setExecuteMesAGNIN() // AGN_IN
{
	CString sdata = _T(""), sLog;

	if (m_pApp->m_bIsGmesConnect == FALSE)
	{
		Lf_addMessage(_T("MES not connected"));
		return TRUE;
	}

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			if (m_pChkChannelUse[layer][ch]->GetCheck() == FALSE)
			{
				sdata.Format(_T("<MES> AGN_IN Send SKIP - R%d-L%d-C%d"), m_nSelRack + 1, layer + 1, ch + 1);
				Lf_addMessage(sdata);
				continue;
			}

			if (lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch].IsEmpty() == TRUE)
				continue;



			sdata.Format(_T("Rack%d-Layer%d-Ch%d  PID:%s"), m_nSelRack + 1, layer + 1, ch + 1, lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch]);
			if (m_pApp->Gf_gmesSendHost(HOST_AGN_IN, m_nSelRack, layer, ch) == TRUE)
			//if (m_pApp->Gf_gmesSendHost(HOST_PCHK, m_nSelRack, layer, ch) == TRUE)
			{
				sLog.Format(_T("<MES> AGN_IN Send OK - %s"), sdata);
				m_pApp->Gf_writeMLog(sLog);

				sdata.Format(_T("<MES> AGN_IN Send OK - R%d-L%d-C%d"), m_nSelRack + 1, layer + 1, ch + 1);
				Lf_addMessage(sdata);
			}
			else
			{
				sLog.Format(_T("<MES> AGN_IN Send NG - %s"), sdata);
				m_pApp->Gf_writeMLog(sLog);

				sdata.Format(_T("<MES> AGN_IN Send NG - R%d-L%d-C%d"), m_nSelRack + 1, layer + 1, ch + 1);
				Lf_addMessage(sdata);
			}
		}
	}

	return TRUE;
}

BOOL CPidInput::Lf_setExecuteMesPCHK() // PCHK
{
	CString sdata = _T(""), sLog;

	if (m_pApp->m_bIsGmesConnect == FALSE)
	{
		Lf_addMessage(_T("MES not connected"));
		return TRUE;
	}

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			if (m_pChkChannelUse[layer][ch]->GetCheck() == FALSE)
			{
				sdata.Format(_T("<MES> PCHK Send SKIP - R%d-L%d-C%d"), m_nSelRack + 1, layer + 1, ch + 1);
				Lf_addMessage(sdata);
				continue;
			}
			if (lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch].IsEmpty() == TRUE)
				continue;

			sdata.Format(_T("Rack%d-Layer%d-Ch%d PID=%s"), m_nSelRack + 1, layer + 1, ch + 1, lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch]);
			if (m_pApp->Gf_gmesSendHost(HOST_PCHK, m_nSelRack, layer, ch) == TRUE)
			{
				sLog.Format(_T("<MES> PCHK Send OK - %s"), sdata);
				m_pApp->Gf_writeMLog(sLog);

				sdata.Format(_T("<MES> PCHK Send OK - R%d-L%d-C%d"), m_nSelRack + 1, layer + 1, ch + 1);
				Lf_addMessage(sdata);
			}
			else
			{
				sLog.Format(_T("<MES> PCHK Send NG - %s"), sdata);
				m_pApp->Gf_writeMLog(sLog);

				sdata.Format(_T("<MES> PCHK Send NG - R%d-L%d-C%d"), m_nSelRack + 1, layer + 1, ch + 1);
				Lf_addMessage(sdata);
			}
		}
	}

	return TRUE;
}

BOOL CPidInput::Lf_setExecuteMesAGNOUT() // AGN_OUT
{
	CString sdata = _T(""), sLog;

	if (m_pApp->m_bIsGmesConnect == FALSE)
	{
		Lf_addMessage(_T("MES not connected"));
		return TRUE;
	}

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			if (m_pChkChannelUse[layer][ch]->GetCheck() == FALSE)
			{
				sdata.Format(_T("<MES> AGN_OUT Send SKIP - R%d-L%d-C%d"), m_nSelRack + 1, layer + 1, ch + 1);
				Lf_addMessage(sdata);
				continue;
			}
			if (lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch].IsEmpty() == TRUE)
				continue;

			sdata.Format(_T("Rack%d-Layer%d-Ch%d PID=%s"), m_nSelRack + 1, layer + 1, ch + 1, lpInspWorkInfo->m_sMesPanelID[m_nSelRack][layer][ch]);
			if (m_pApp->Gf_gmesSendHost(HOST_AGN_OUT, m_nSelRack, layer, ch) == TRUE)
			{
				sLog.Format(_T("<MES> AGN_OUT Send OK - %s"), sdata);
				m_pApp->Gf_writeMLog(sLog);

				sdata.Format(_T("<MES> AGN_OUT Send OK - R%d-L%d-C%d"), m_nSelRack + 1, layer + 1, ch + 1);
				Lf_addMessage(sdata);
			}
			else
			{
				sLog.Format(_T("<MES> AGN_OUT Send NG - %s"), sdata);
				m_pApp->Gf_writeMLog(sLog);

				sdata.Format(_T("<MES> AGN_OUT SEnd NG - R%d-L%d-C%d"), m_nSelRack + 1, layer + 1, ch + 1);
				Lf_addMessage(sdata);
			}
		}
	}

	return TRUE;
}

void CPidInput::Lf_checkBcrRackChIDInput(CString RackID, CString ChID)
{
	// [1] RACK ID 매핑
	CString rackKeys[6] = {
		_T("RACK1_BCR_ID"), _T("RACK2_BCR_ID"), _T("RACK3_BCR_ID"),
		_T("RACK4_BCR_ID"), _T("RACK5_BCR_ID"), _T("RACK6_BCR_ID")
	};

	for (int i = 0; i < 6; ++i)
	{
		CString rackVal;
		Read_SysIniFile(_T("SYSTEM"), rackKeys[i], &rackVal);

		if (!rackVal.IsEmpty() && RackID.Find(rackVal) != -1)
		{
			switch (i)
			{
			case 0: OnBnClickedMbcPiRack1(); break;
			case 1: OnBnClickedMbcPiRack2(); break;
			case 2: OnBnClickedMbcPiRack3(); break;
			case 3: OnBnClickedMbcPiRack4(); break;
			case 4: OnBnClickedMbcPiRack5(); break;
			case 5: OnBnClickedMbcPiRack6(); break;
			}
			break;
		}
	}

	// [2] CH 번호 파싱
	int chNumber = _ttoi(ChID); // "CH10" → 10

	int row = -1, col = -1;

	if (chNumber >= 1 && chNumber <= 40)
	{
		row = (chNumber - 1) / 8;
		col = (chNumber - 1) % 8;
	}
	else if (chNumber >= 41 && chNumber <= 80)
	{
		row = (chNumber - 41) / 8;
		col = (chNumber - 41) % 8 + 8; // 오른쪽 col(8~15)
	}

	if (row >= 0 && row < MAX_LAYER && col >= 0 && col < MAX_LAYER_CHANNEL && m_pedtPannelID[row][col] != nullptr)
	{
		m_pedtPannelID[row][col]->SetFocus();
	}
	else
	{
		TRACE(_T("⚠️ 잘못된 포커싱: ChID=%s → chNumber=%d → row=%d, col=%d\n"), ChID, chNumber, row, col);
	}
}


void CPidInput::Lf_Send_checkBcrRackChIDInput(CString RackID, CString ChID)
{
	int chNumber = _ttoi(ChID.Mid(2)); // "CH10" → 10

	int row = -1, col = -1;

	if (chNumber >= 1 && chNumber <= 40)
	{
		row = (chNumber - 1) / 8;
		col = (chNumber - 1) % 8;
	}
	else if (chNumber >= 41 && chNumber <= 80)
	{
		row = (chNumber - 41) / 8;
		col = (chNumber - 41) % 8 + 8; // 오른쪽 col(8~15)
	}

	if (row >= 0 && row < MAX_LAYER && col >= 0 && col < MAX_LAYER_CHANNEL && m_pedtPannelID[row][col] != nullptr)
	{
		m_pedtPannelID[row][col]->SetFocus();
	}
	else
	{
		TRACE(_T("⚠️ 잘못된 포커싱: ChID=%s → chNumber=%d → row=%d, col=%d\n"), ChID, chNumber, row, col);
	}
}
