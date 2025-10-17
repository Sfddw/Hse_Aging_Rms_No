
// HseAging.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "HseAging.h"
#include "HseAgingDlg.h"
#include "MessageError.h"
#include "MessageQuestion.h"
#include "UserID.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHseAgingApp

BEGIN_MESSAGE_MAP(CHseAgingApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CHseAgingApp 생성

CHseAgingApp::CHseAgingApp()
{
	// 다시 시작 관리자 지원
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
	m_pApp = (CHseAgingApp*)AfxGetApp();

	lpModelInfo		= new MODELINFO;
	lpSystemInfo	= new SYSTEMINFO;
	lpInspWorkInfo	= new INSPWORKINFO;
	pCommand		= new CCommand();
	pCimNet			= new CCimNetCommApi;
	m_pTemp2xxx		= new CTemp2xxx;


	m_pUDPSocket	= new CUDPSocket();
}


// 유일한 CHseAgingApp 개체입니다.

CHseAgingApp theApp;
CHseAgingApp* m_pApp;


// CHseAgingApp 초기화

BOOL CHseAgingApp::InitInstance()
{
	// 애플리케이션 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(_T("Windows socket initial fail"));
		return FALSE;
	}


	AfxEnableControlContainer();

	// 대화 상자에 셸 트리 뷰 또는
	// 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
	CShellManager *pShellManager = new CShellManager;

	// MFC 컨트롤의 테마를 사용하기 위해 "Windows 원형" 비주얼 관리자 활성화
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 애플리케이션 마법사에서 생성된 애플리케이션"));

	//////////////////////////////////////////////////////////////////
	CreateMutex(NULL, TRUE, _T("HseAging"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		AfxMessageBox(_T("'HSE Aging'  Already Running!!"), MB_ICONSTOP);
		PostQuitMessage(0);
		return FALSE;
	}

	//////////////////////////////////////////////////////////////////
	Gf_LoadSystemData();
	Gf_SoftwareStartLog();					// 프로그램 시작 LOG 기록

	//////////////////////////////////////////////////////////////////
	Lf_InitGlobalVariable();
	Lf_initCreateUdpSocket();
	Gf_initTempRecorder();
	Lf_initCreateFolder();

	// GMES DLL Initialize
	if (Gf_gmesInitServer(SERVER_MES) == FALSE)
	{
		AfxMessageBox(_T("TIB Driver Init Fail.\r\nPlease check whether you have installed the TibDriver and registered the MES DLL."), MB_ICONERROR);
	}
	if (Gf_gmesInitServer(SERVER_EAS) == FALSE)
	{
		AfxMessageBox(_T("TIB Driver Init Fail.\r\nPlease check whether you have installed the TibDriver and registered the EAS DLL."), MB_ICONERROR);
	}
	if (Gf_gmesInitServer(SERVER_RMS) == FALSE)
	{
		AfxMessageBox(_T("TIB Driver Init Fail.\r\nPlease check whether you have installed the TibDriver and registered the RMS DLL."), MB_ICONERROR);
	}

	CUserID user_dlg;
	if (user_dlg.DoModal() == IDCANCEL)
	{
		// exit software
		PostQuitMessage(0);
		return FALSE;
	}

	CHseAgingDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}

	// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고 응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LPMODELINFO CHseAgingApp::GetModelInfo()
{
	ASSERT(NULL != lpModelInfo);
	VERIFY(NULL != lpModelInfo);

	return lpModelInfo;
}

LPSYSTEMINFO CHseAgingApp::GetSystemInfo()
{
	ASSERT(NULL != lpSystemInfo);
	VERIFY(NULL != lpSystemInfo);

	return lpSystemInfo;
}

LPINSPWORKINFO CHseAgingApp::GetInspWorkInfo()
{
	ASSERT(NULL != lpInspWorkInfo);
	VERIFY(NULL != lpInspWorkInfo);

	return lpInspWorkInfo;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHseAgingApp::Lf_InitGlobalVariable()
{
	m_bUserIdAdmin = FALSE;
	m_bIsGmesConnect = FALSE;
	m_bIsEasConnect = FALSE;

	memset(m_nAckCmdPG, 0, sizeof(m_nAckCmdPG));

	memset(lpInspWorkInfo->m_nConnectInfo, 0, sizeof(lpInspWorkInfo->m_nConnectInfo));
	memset(lpInspWorkInfo->m_nMainEthConnect, 0, sizeof(lpInspWorkInfo->m_nMainEthConnect));
	memset(lpInspWorkInfo->m_nAgingSetTime, 0, sizeof(lpInspWorkInfo->m_nAgingSetTime));
	memset(lpInspWorkInfo->m_nAgingRunTime, 0, sizeof(lpInspWorkInfo->m_nAgingRunTime));
	memset(lpInspWorkInfo->m_nAgingDoorOpenTime, 0, sizeof(lpInspWorkInfo->m_nAgingDoorOpenTime));

	memset(lpInspWorkInfo->m_nAgingTempMatchTime, 0, sizeof(lpInspWorkInfo->m_nAgingTempMatchTime));

	memset(lpInspWorkInfo->m_nMeasVCC, 0, sizeof(lpInspWorkInfo->m_nMeasVCC));
	memset(lpInspWorkInfo->m_nMeasICC, 0, sizeof(lpInspWorkInfo->m_nMeasICC));
	memset(lpInspWorkInfo->m_nMeasVBL, 0, sizeof(lpInspWorkInfo->m_nMeasVBL));
	memset(lpInspWorkInfo->m_nMeasIBL, 0, sizeof(lpInspWorkInfo->m_nMeasIBL));

	memset(lpInspWorkInfo->m_ast_AgingLayerError, 0, sizeof(lpInspWorkInfo->m_ast_AgingLayerError));
	memset(lpInspWorkInfo->m_ast_AgingStartStop, 0, sizeof(lpInspWorkInfo->m_ast_AgingStartStop));
	memset(lpInspWorkInfo->m_ast_AgingChOnOff, 0, sizeof(lpInspWorkInfo->m_ast_AgingChOnOff));
	memset(lpInspWorkInfo->m_ast_ChUseUnuse, 0, sizeof(lpInspWorkInfo->m_ast_ChUseUnuse));
	memset(lpInspWorkInfo->m_ast_CableOpenCheck, 0, sizeof(lpInspWorkInfo->m_ast_CableOpenCheck));
	memset(lpInspWorkInfo->m_ast_AgingChErrorResult, 0, sizeof(lpInspWorkInfo->m_ast_AgingChErrorResult));
	memset(lpInspWorkInfo->m_ast_AgingChErrorType, 0, sizeof(lpInspWorkInfo->m_ast_AgingChErrorType));
	memset(lpInspWorkInfo->m_ast_AgingChErrorValue, 0, sizeof(lpInspWorkInfo->m_ast_AgingChErrorValue));

	memset(lpInspWorkInfo->m_nDioInputData, 0, sizeof(lpInspWorkInfo->m_nDioInputData));
	memset(lpInspWorkInfo->m_nDoorOpenClose, 1, sizeof(lpInspWorkInfo->m_nDoorOpenClose));
	memset(lpInspWorkInfo->m_fTempReadVal, 0, sizeof(lpInspWorkInfo->m_fTempReadVal));

	memset(lpInspWorkInfo->m_fOpeAgingTempMin, 60, sizeof(lpInspWorkInfo->m_fOpeAgingTempMin));
	memset(lpInspWorkInfo->m_fOpeAgingTempMax, 0, sizeof(lpInspWorkInfo->m_fOpeAgingTempMax));
	memset(lpInspWorkInfo->m_fOpeAgingTempAvg, 0, sizeof(lpInspWorkInfo->m_fOpeAgingTempAvg));
	memset(lpInspWorkInfo->m_fOpeAgingVccMin, 0, sizeof(lpInspWorkInfo->m_fOpeAgingVccMin));
	memset(lpInspWorkInfo->m_fOpeAgingVccMax, 0, sizeof(lpInspWorkInfo->m_fOpeAgingVccMax));
	memset(lpInspWorkInfo->m_fOpeAgingVccAvg, 0, sizeof(lpInspWorkInfo->m_fOpeAgingVccAvg));
	memset(lpInspWorkInfo->m_fOpeAgingIccMin, 0, sizeof(lpInspWorkInfo->m_fOpeAgingIccMin));
	memset(lpInspWorkInfo->m_fOpeAgingIccMax, 0, sizeof(lpInspWorkInfo->m_fOpeAgingIccMax));
	memset(lpInspWorkInfo->m_fOpeAgingIccAvg, 0, sizeof(lpInspWorkInfo->m_fOpeAgingIccAvg));
	memset(lpInspWorkInfo->m_fOpeAgingVblMin, 0, sizeof(lpInspWorkInfo->m_fOpeAgingVblMin));
	memset(lpInspWorkInfo->m_fOpeAgingVblMax, 0, sizeof(lpInspWorkInfo->m_fOpeAgingVblMax));
	memset(lpInspWorkInfo->m_fOpeAgingVblAvg, 0, sizeof(lpInspWorkInfo->m_fOpeAgingVblAvg));
	memset(lpInspWorkInfo->m_fOpeAgingIblMin, 0, sizeof(lpInspWorkInfo->m_fOpeAgingIblMin));
	memset(lpInspWorkInfo->m_fOpeAgingIblMax, 0, sizeof(lpInspWorkInfo->m_fOpeAgingIblMax));
	memset(lpInspWorkInfo->m_fOpeAgingIblAvg, 0, sizeof(lpInspWorkInfo->m_fOpeAgingIblAvg));

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		lpInspWorkInfo->m_nAgingOperatingMode[rack] = AGING_IDLE;
		lpInspWorkInfo->m_nFwVerifyResult[rack] = TRUE;
	}
}

void CHseAgingApp::Lf_initCreateFolder()
{
	if (PathFileExists(_T("./Model")) != TRUE)	CreateDirectory(_T("./Model"), NULL);
	if (PathFileExists(_T("./Config")) != TRUE)	CreateDirectory(_T("./Config"), NULL);
	if (PathFileExists(_T("./Logs")) != TRUE)	CreateDirectory(_T("./Logs"), NULL);
	if (PathFileExists(_T("./Logs/MLog")) != TRUE)	CreateDirectory(_T("./Logs/MLog"), NULL);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHseAgingApp::Gf_SoftwareStartLog()
{
	// Main Form Title Set
	CString strPGMTitle;
	char D_String[15] = { 0, };
	char Date_String[15] = { 0, };
	char* Date[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	sprintf_s(D_String, "%s", __DATE__);
	for (int i = 12; i; i--)
	{
		for (int j = 3; j; j--)
		{
			if (D_String[j - 1] != *(Date[i - 1] + (j - 1)))
				break;
			if (j == 1)
			{
				if (D_String[4] == 0x20)	D_String[4] = 0x30;
				sprintf_s(Date_String, "%c%c%c%c-%02d-%c%c", D_String[7], D_String[8], D_String[9], D_String[10], i, D_String[4], D_String[5]);
				i = 1; j = 1;
				break;
			}
		}
	}

	m_sSoftwareVersion.Format(_T("%S - %s"), Date_String, PGM_VERSION);
	strPGMTitle.Format(_T("Hse Aging ( %s )"), m_sSoftwareVersion);

	CString sLog;
	sLog.Format(_T("***************************** %s *****************************"), strPGMTitle);
	Gf_writeMLog(sLog);
}

void CHseAgingApp::Gf_writeMLog(CString sLogData)
{
	CFile cfp;
	USHORT nShort = 0xfeff;
	CString strLog, fileName, filePath, strDate;

	// 엔터 Key 값이 있으면 문자를 변경 시키낟.
	sLogData.Replace(_T("\r\n"), _T(" | "));

	SYSTEMTIME sysTime;
	::GetSystemTime(&sysTime);
	CTime time = CTime::GetCurrentTime();
	strLog.Format(_T("[%02d:%02d:%02d %03d] %06d%03d\t: %s\r\n"), time.GetHour(), time.GetMinute(), time.GetSecond(), sysTime.wMilliseconds, (time.GetHour() * 3600) + (time.GetMinute() * 60) + time.GetSecond(), sysTime.wMilliseconds, sLogData);

	strDate.Format(_T("%04d%02d%02d"), time.GetYear(), time.GetMonth(), time.GetDay());
	filePath.Format(_T("./Logs/MLog/%s_%s.txt"), lpSystemInfo->m_sEqpName, strDate);

	if (GetFileAttributes(_T("./Logs/")) == -1)
		CreateDirectory(_T("./Logs/"), NULL);

	if (GetFileAttributes(_T("./Logs/MLog")) == -1)
		CreateDirectory(_T("./Logs/MLog"), NULL);


	if (cfp.Open(filePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::typeBinary))
	{
		if (cfp.GetLength() == 0)
		{
			cfp.Write(&nShort, 2);
		}
		cfp.SeekToEnd();
		cfp.Write(strLog, (strLog.GetLength() * 2));
		cfp.Close();
	}
}

void CHseAgingApp::Gf_writeAlarmLog(int rack, int layer, int ch, CString strError)
{
	FILE* fp;

	char szbuf[100] = { 0, };
	char szYear[5] = { 0, };
	char szMonth[5] = { 0, };
	char szDay[5] = { 0, };
	char filename[256] = { 0 };
	char filepath[1024] = { 0 };
	char dataline[4096] = { 0 };

	CString strDate;
	CString strTime;

	SYSTEMTIME sysTime;
	::GetSystemTime(&sysTime);
	CTime time = CTime::GetCurrentTime();

	strDate.Format(_T("%04d%02d%02d"), time.GetYear(), time.GetMonth(), time.GetDay());
	strTime.Format(_T("%02d:%02d:%02d"), time.GetHour(), time.GetMinute(), time.GetSecond());


	// 1. 경로를 찾고 없으면 생성한다.
	sprintf_s(szYear, "%04d", time.GetYear());
	sprintf_s(szMonth, "%02d", time.GetMonth());
	sprintf_s(szDay, "%02d", time.GetDay());

	if ((_access(".\\Logs\\AlarmLog", 0)) == -1)
		_mkdir(".\\Logs\\AlarmLog");

	sprintf_s(szbuf, ".\\Logs\\AlarmLog\\%s", szYear);
	if ((_access(szbuf, 0)) == -1)
		_mkdir(szbuf);

	sprintf_s(szbuf, ".\\Logs\\AlarmLog\\%s\\%s", szYear, szMonth);
	if ((_access(szbuf, 0)) == -1)
		_mkdir(szbuf);

	// 2. file을 open한다.
	sprintf_s(filename, "%04d%02d%02d_AlarmLog.txt", time.GetYear(), time.GetMonth(), time.GetDay());
	sprintf_s(filepath, "%s\\%s", szbuf, filename);

	fopen_s(&fp, filepath, "r+");
	if (fp == NULL)
	{
		fopen_s(&fp, filepath, "a+");
		if (fp == NULL) // 2007-08-01 : fseek.c(101) error
		{
			AfxMessageBox(_T("'Aging Alarm Log' file create fail."), MB_ICONERROR);
			return;
		}
	}

	fseek(fp, 0L, SEEK_END);

	// 3. Log를 Write한다.
	char szdate[100] = { 0, };
	char szerror[1024] = { 0, };

	sprintf_s(szdate, "%04d-%02d-%02d %02d:%02d:%02d", time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());
	sprintf_s(szerror, "%S", strError.GetBuffer(0));

	if (rack < MAX_RACK)
	{
		sprintf_s(dataline, "%s\tRACK-%d\tLAYER-%d\tCH_%d\t%s\t\r\n", szdate, rack + 1, layer + 1, ch + 1, szerror);
	}
	else if (rack == 19)
	{
		// TEMP OVER HEAT
		sprintf_s(dataline, "%s\tTEMP\t \tCH_%02d\t%s\t\r\n", szdate, ch + 1, szerror);
	}
	fwrite(dataline, sizeof(char), strlen(dataline), fp);

	// 4. File을 닫는다.
	fclose(fp);
}

BOOL CHseAgingApp::Gf_ShowMessageBox(CString errMessage)
{
	CMessageError errDlg;
	errDlg.m_strEMessage = errMessage;
	errDlg.DoModal();

	return TRUE;
}

void CHseAgingApp::Gf_setGradientStatic01(CGradientStatic* pGStt, CFont* pFont, BOOL bSplit)// magenta
{
	pGStt->SetFont(pFont);
	pGStt->SetTextAlign(TEXT_ALIGN_CENTER);
	pGStt->SetColor(RGB(38, 88, 137));
	pGStt->SetGradientColor(RGB(51, 76, 100));
	pGStt->SetVerticalGradient();
	if (bSplit == TRUE)	pGStt->SetSplitMode(TRUE);
	pGStt->SetTextColor(RGB(255, 255, 255));
}

void CHseAgingApp::Gf_setGradientStatic02(CGradientStatic* pGStt, CFont* pFont, BOOL bSplit, int txt_align) //gray
{
	pGStt->SetFont(pFont);

	if (txt_align == TEXT_ALIGN_LEFT)			pGStt->SetTextAlign(TEXT_ALIGN_LEFT);
	else if (txt_align == TEXT_ALIGN_CENTER)	pGStt->SetTextAlign(TEXT_ALIGN_CENTER);
	else										pGStt->SetTextAlign(TEXT_ALIGN_RIGHT);

	pGStt->SetColor(RGB(128, 128, 128));
	pGStt->SetGradientColor(RGB(100, 100, 100));
	pGStt->SetVerticalGradient();
	if (bSplit == TRUE)	pGStt->SetSplitMode(TRUE);
	pGStt->SetTextColor(RGB(255, 255, 255));
}

void CHseAgingApp::Gf_setGradientStatic03(CGradientStatic* pGStt, CFont* pFont, BOOL bSplit)// orange
{
	pGStt->SetFont(pFont);
	pGStt->SetTextAlign(TEXT_ALIGN_CENTER);
	pGStt->SetColor(RGB(245, 136, 22));
	pGStt->SetGradientColor(RGB(222, 118, 14));
	pGStt->SetVerticalGradient();
	if (bSplit == TRUE)	pGStt->SetSplitMode(TRUE);
	pGStt->SetTextColor(RGB(255, 255, 255));
}

void CHseAgingApp::Gf_setGradientStatic04(CGradientStatic* pGStt, CFont* pFont, BOOL bSplit)// blue
{
	pGStt->SetFont(pFont);
	pGStt->SetTextAlign(TEXT_ALIGN_CENTER);
	pGStt->SetColor(RGB(72, 82, 92));
	pGStt->SetGradientColor(RGB(42, 52, 62));
	pGStt->SetVerticalGradient();
	if (bSplit == TRUE)	pGStt->SetSplitMode(TRUE);
	pGStt->SetTextColor(COLOR_GRAY192);
}

void CHseAgingApp::Gf_setGradientStatic05(CGradientStatic* pGStt, CFont* pFont, BOOL bSplit)// green
{
	pGStt->SetFont(pFont);
	pGStt->SetTextAlign(TEXT_ALIGN_CENTER);
	pGStt->SetColor(RGB(116, 192, 24));
	pGStt->SetGradientColor(RGB(100, 161, 19));
	pGStt->SetVerticalGradient();
	if (bSplit == TRUE)	pGStt->SetSplitMode(TRUE);
	pGStt->SetTextColor(COLOR_WHITE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHseAgingApp::Gf_InitialSystemInfo()
{
	Gf_initTempRecorder();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHseAgingApp::Lf_IPStringToRackInfo(CString ip, int* rack, int* layer)
{
	CString sdata;
	int nIpAddr;

	// IP 주소를 Int 변수로 변경한다.
	sdata = ip.Right(2);
	if (sdata.Left(1) == _T("."))	sdata.Delete(0, 1);
	nIpAddr = _ttoi(sdata);
	nIpAddr--;

	*rack = nIpAddr / 5;
	*layer = nIpAddr % 5;
}

void CHseAgingApp::Lf_initCreateUdpSocket()
{
	if (m_pUDPSocket->CreatSocket(UDP_SOCKET_PORT, SOCK_DGRAM) == FALSE)
	{
		AfxMessageBox(_T("UDP Socket Create Fail"), MB_ICONERROR);
		return;
	}
}

BOOL CHseAgingApp::procWaitRecvACK(int rack, int layer, int cmd, DWORD waitTime)
{
	DWORD stTick = ::GetTickCount();

	while (1)
	{
		DWORD edTick = GetTickCount();
		if ((edTick - stTick) > waitTime)
			return FALSE;

		if (cmd == m_nAckCmdPG[rack][layer])
		{
			return TRUE;
		}

		ProcessMessage();
	}
	return FALSE;
}

BOOL CHseAgingApp::procWaitRecvACK_DIO(int cmd, DWORD waitTime)
{
	DWORD stTick = ::GetTickCount();

	while (1)
	{
		if (cmd == m_nAckCmdDIO)
		{
			return TRUE;
		}

		DWORD edTick = GetTickCount();

		if ((edTick - stTick) > waitTime)
		{
			return FALSE;
		}

		ProcessMessage();
	}
	return FALSE;
}

BOOL CHseAgingApp::procParseCableOpenCheck(int rack, int layer, CString packet)
{
	int retcode;

	retcode = _tcstol(packet.Mid(PACKET_PT_RET, 1), NULL, 16);

	if (retcode == 0)
	{
		int ptr = PACKET_PT_DATA;

		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			int idx = ptr + ch;
			lpInspWorkInfo->m_ast_CableOpenCheck[rack][layer][ch] = _ttoi(packet.Mid(idx, 1));
		}
	}

	return TRUE;
}

BOOL CHseAgingApp::procParseAgingStatus(int rack, int layer, CString packet)
{
	CString sdata, sRet, sLayer, sStart, chUse, chOnOff, sCable, sErrInfo;
	int dataLen, errCnt, retcode;

#if 0
	//packet.Format(_T("%cA2A100AB001E000000000000001010004010500070B5%c"), 0x02, 0x03);
	sLayer = _T("0");
	sStart = _T("0");
	chUse = _T("0000");		// 0:Use, 1:Unuse
	chOnOff = _T("0000");	// 0:PowerOff, 1:PowerOn
	sCable = _T("0000");	// 0:Connect, 1:Open
	//sErrInfo = _T("1010004010500070");
	packet.Format(_T("%cA2A100AB000E0%s%s%s%s%s%sB5%c"), 0x02, sLayer, sStart, chUse, chOnOff, sCable, sErrInfo, 0x03);
#endif

	dataLen = _tcstol(packet.Mid(PACKET_PT_LEN, 4), NULL, 16);
	retcode = _tcstol(packet.Mid(PACKET_PT_RET, 1), NULL, 16);
	if (dataLen != 15)
	{
		int a = 1;
		a++;
	}
	errCnt = (dataLen - 14) / 8;

	if (retcode == 0)
	{
		int nVal, ptr = 0, len = 5;

		ptr = PACKET_PT_DATA;

		/********************************************************************************/
		// Aging Layer Error Status
		sdata = packet.Mid(ptr, 1);
		lpInspWorkInfo->m_ast_AgingLayerError[rack][layer] = _ttoi(sdata);
		ptr++;

		// Aging Start/Stop Info
		sdata = packet.Mid(ptr, 1);
		lpInspWorkInfo->m_ast_AgingStartStop[rack][layer] = _ttoi(sdata);
		ptr++;

		// Channel Use/Unuse Status
		sdata = packet.Mid(ptr, 4);
		nVal = _tcstol(sdata, NULL, 16);
		ptr += 4;
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] = (nVal >> ch) & 0x0001;
		}

		// Channel On/Off Status
		sdata = packet.Mid(ptr, 4);
		nVal = _tcstol(sdata, NULL, 16);
		ptr += 4;
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			lpInspWorkInfo->m_ast_AgingChOnOff[rack][layer][ch] = (nVal >> ch) & 0x0001;
		}

		// Cable Open Status
		sdata = packet.Mid(ptr, 4);
		nVal = _tcstol(sdata, NULL, 16);
		ptr += 4;
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			lpInspWorkInfo->m_ast_CableOpenCheck[rack][layer][ch] = (nVal >> ch) & 0x0001;
		}

		for(int i = 0; i < errCnt; i++)
		{
			int errResult, errGroup, errCh, errValue;
			sdata = packet.Mid(ptr, 1);		errResult = _ttoi(sdata);		ptr += 1;
			sdata = packet.Mid(ptr, 1);		errGroup = _ttoi(sdata);		ptr += 1;
			sdata = packet.Mid(ptr, 1);		errCh = _ttoi(sdata);			ptr += 1;
			sdata = packet.Mid(ptr, 5);		errValue = _ttoi(sdata);		ptr += 5;

			if (errGroup == 0)	// VCC 0-7
			{
				lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][errCh] = errResult;
				lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][errCh] = ERR_INFO_VCC;
				lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][errCh] = errValue;
			}
			else if (errGroup == 1)	// VCC 8-15
			{
				lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][errCh + 8] = errResult;
				lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][errCh + 8] = ERR_INFO_VCC;
				lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][errCh + 8] = errValue;
			}
			else if (errGroup == 2)	// ICC 0-7
			{
				lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][errCh] = errResult;
				lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][errCh] = ERR_INFO_ICC;
				lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][errCh] = errValue;
			}
			else if (errGroup == 3)	// ICC 8-15
			{
				lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][errCh + 8] = errResult;
				lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][errCh + 8] = ERR_INFO_ICC;
				lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][errCh + 8] = errValue;
			}
			else if (errGroup == 4)	// VBL 0-7
			{
				lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][errCh] = errResult;
				lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][errCh] = ERR_INFO_VBL;
				lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][errCh] = errValue;
			}
			else if (errGroup == 5)	// VBL 8-15
			{
				lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][errCh + 8] = errResult;
				lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][errCh + 8] = ERR_INFO_VBL;
				lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][errCh + 8] = errValue;
			}
			else if (errGroup == 6)	// IBL 0-7
			{
				lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][errCh] = errResult;
				lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][errCh] = ERR_INFO_IBL;
				lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][errCh] = errValue;
			}
			else if (errGroup == 7)	// IBL 8-15
			{
				lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][errCh + 8] = errResult;
				lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][errCh + 8] = ERR_INFO_IBL;
				lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][errCh + 8] = errValue;
			}
		}
	}

	return TRUE;
}

BOOL CHseAgingApp::procParseMeasurePower(int rack, int layer, CString packet)
{
	int retcode;

	// Measure Packet 잘못 입력 되었을 경우의 예외처리
	if (packet.GetLength() < 320)
		return FALSE;

	retcode = _tcstol(packet.Mid(PACKET_PT_RET, 1), NULL, 16);

	if (retcode == 0)
	{
		int ptr = 0, len = 5;
		CString sdata = _T("");

		ptr = PACKET_PT_DATA;

		CString debugStr;
		

		/********************************************************************************/
		//전압전류
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			//ptr += len;	len = 5;
			sdata = packet.Mid(ptr, len);		lpInspWorkInfo->m_nMeasVCC[rack][layer][ch] = _ttoi(sdata);
			ptr += len;
			/*debugStr.Format(_T("[Debug] ptr=%d, len=%d, rack=%d, layer=%d, ch=%d, sdata=%s\n"),
				ptr, len, rack, layer, ch, sdata);
			OutputDebugString(debugStr);*/
		}

		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			//ptr += len;	len = 5;
			sdata = packet.Mid(ptr, len);		lpInspWorkInfo->m_nMeasICC[rack][layer][ch] = _ttoi(sdata);
			ptr += len;
		}

		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			//ptr += len;	len = 5;
			sdata = packet.Mid(ptr, len);		lpInspWorkInfo->m_nMeasVBL[rack][layer][ch] = _ttoi(sdata);
			ptr += len;
		}

		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			//ptr += len;	len = 5;
			sdata = packet.Mid(ptr, len);		lpInspWorkInfo->m_nMeasIBL[rack][layer][ch] = _ttoi(sdata);
			ptr += len;
		}
	}

	return TRUE;
}

BOOL CHseAgingApp::procParseFWVersion(int rack, int layer, CString packet)
{
	int retcode;

	retcode = _tcstol(packet.Mid(PACKET_PT_RET, 1), NULL, 16);

	if (retcode == 0)
	{
		CString sdata;
		sdata = packet.Mid(PACKET_PT_DATA, (packet.GetLength() - 17));

		lpInspWorkInfo->m_sMainFWVersion[rack][layer] = sdata.Left(10);


		CString skey = _T("");
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_FW_VER] = lpInspWorkInfo->m_sMainFWVersion[rack][layer];
		}
	}

	return TRUE;
}

BOOL CHseAgingApp::procParseGotoBootSection(int rack, int layer, CString packet)
{
	int retcode;

	retcode = _tcstol(packet.Mid(PACKET_PT_RET, 1), NULL, 16);
	if (retcode == 0)
	{
		if (m_nDownloadCountUp == TRUE)
		{
			m_nDownloadCountUp = FALSE;
			m_nDownloadReadyAckCount = m_nDownloadReadyAckCount + 1;
		}
	}
	return TRUE;
}

BOOL CHseAgingApp::udp_sendPacketUDP(CString ip, int nCommand, int nSize, char* pdata, int recvACK, int waitTime)
{
	int datalen = 0;
	int packetlen = 0;
	BYTE nChkSum = 0;
	char szbuff[5] = { 0, };
	char sendPacket[PACKET_SIZE] = { 0, };

	datalen = nSize;

	// data 앞까지 Packet 생성
	sprintf_s(sendPacket, "%cA1%02X00%02X%04X", PACKET_STX, TARGET_MAIN, nCommand, datalen);

	// data를 포함하여 packet 생성. hex로 전송할 data가 있으므로 memcpy를 사용
	packetlen = (int)strlen(sendPacket);

	memcpy(&sendPacket[packetlen], pdata, datalen);

	// data 를 포함한 packet의 길이를 구한다.
	packetlen += datalen;

	// 생성된 Packet을 이용하여 CheckSum을 구한다.
	for (int j = 1; j < packetlen; j++)		// Check Sum
	{
		nChkSum += sendPacket[j];
	}
	sprintf_s(szbuff, "%02X%c", nChkSum, 0x03);

	// Checksum과 ETX 3byte를 붙여 다시 Packet을 만든다.
	memcpy(&sendPacket[packetlen], szbuff, 3);
	packetlen += 3;

	// Packet의 마지막에 String의 끝을 알리기 위하여 NULL을 추가한다.
	sendPacket[packetlen] = 0x00;

	// Send Log를 기록
#if	(DEBUG_COMM_LOG==1)
	CString sLog;
	sLog.Format(_T("<UDP Send> [%s] %S"), ip, sendPacket);
	m_pApp->Gf_writeMLog(sLog);
#endif

	// 생성된 Packet을 전송.
	UINT ret = TRUE;

	m_pUDPSocket->SendToUDP(ip, packetlen, sendPacket);

	int rack=-1, layer=-1;
	Lf_IPStringToRackInfo(ip, &rack, &layer);

	// ACK Receive
	if ((rack == -1) && (layer == -1))
		return FALSE;

	m_nAckCmdPG[rack][layer] = 0;
	if (recvACK == ACK)
	{
		if (procWaitRecvACK(rack, layer, nCommand, waitTime) == TRUE)
			ret = TRUE;
		else
			ret = FALSE;
	}

	return ret;
}

BOOL CHseAgingApp::udp_sendPacketUDPRack(int rack, int nCommand, int nSize, char* pdata, int recvACK, int waitTime)
{
	BOOL bRet = TRUE;
	CString ipaddr = _T("");
	int baseip = (rack*MAX_LAYER) + 1;

	ZeroMemory(lpInspWorkInfo->m_nRecvACK_Rack[rack], sizeof(lpInspWorkInfo->m_nRecvACK_Rack[rack]));

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		if(lpInspWorkInfo->m_nMainEthConnect[rack][layer] != 0)
			lpInspWorkInfo->m_nMainEthConnect[rack][layer]--;

		ipaddr.Format(_T("192.168.10.%d"), (baseip + layer));
		udp_sendPacketUDP(ipaddr, nCommand, nSize, pdata, NACK);
	}

	// ACK를 Check한다.
	if (recvACK == ACK)
	{
		DWORD sTick, eTick;
		sTick = ::GetTickCount();

		while (1)
		{
			BOOL bAllAck = TRUE;
			for (int layer = 0; layer < MAX_LAYER; layer++)
			{
				if ((lpInspWorkInfo->m_nMainEthConnect[rack][layer]) && (lpInspWorkInfo->m_nRecvACK_Rack[rack][layer] == FALSE))
				{
					bAllAck = FALSE;
				}
			}

			// 모든 Main에서 ACK가 전송 되었으면 Return TRUE
			if (bAllAck == TRUE)
				return TRUE;

			// ACK Wait Time Check
			eTick = ::GetTickCount();
			if ((eTick - sTick) > (DWORD)waitTime)
				return FALSE;

			ProcessMessage();
		}
	}

	return TRUE;
}

BOOL CHseAgingApp::udp_sendPacketUDPRack_Pid(int ChID, int rack, int nCommand, int nSize, char* pdata, int recvACK, int waitTime)
{
	BOOL bRet = TRUE;
	CString ipaddr = _T("");
	int baseip = (rack * MAX_LAYER) + 1;
	int IpAddr = 0;

	ZeroMemory(lpInspWorkInfo->m_nRecvACK_Rack[rack], sizeof(lpInspWorkInfo->m_nRecvACK_Rack[rack]));

	/*if (ChID > 0 && ChID <= 16)
	{
		IpAddr = 0;
	}
	else if (ChID > 16 && ChID <= 32)
	{
		IpAddr = 1;
	}
	else if (ChID > 32 && ChID <= 48)
	{
		IpAddr = 2;
	}
	else if (ChID > 48 && ChID <= 64)
	{
		IpAddr = 3;
	}
	else if (ChID > 64 && ChID <= 80)
	{
		IpAddr = 4;
	}*/
	if ((ChID >= 1 && ChID <= 8) || (ChID >= 41 && ChID <= 42)) {
		IpAddr = 0;
	}
	else if ((ChID >= 9 && ChID <= 16) || (ChID >= 49 && ChID <= 56)) {
		IpAddr = 1;
	}
	else if ((ChID >= 17 && ChID <= 24) || (ChID >= 57 && ChID <= 64)) {
		IpAddr = 2;
	}
	else if ((ChID >= 25 && ChID <= 32) || (ChID >= 65 && ChID <= 72)) {
		IpAddr = 3;
	}
	else if ((ChID >= 33 && ChID <= 40) || (ChID >= 73 && ChID <= 80)) {
		IpAddr = 4;
	}

		if (lpInspWorkInfo->m_nMainEthConnect[rack][IpAddr] != 0)
			lpInspWorkInfo->m_nMainEthConnect[rack][IpAddr]--;

		ipaddr.Format(_T("192.168.10.%d"), (baseip + IpAddr));
		udp_sendPacketUDP(ipaddr, nCommand, nSize, pdata, NACK);

	// ACK를 Check한다.
	if (recvACK == ACK)
	{
		DWORD sTick, eTick;
		sTick = ::GetTickCount();

		while (1)
		{
			BOOL bAllAck = TRUE;
			for (int layer = 0; layer < MAX_LAYER; layer++)
			{
				if ((lpInspWorkInfo->m_nMainEthConnect[rack][layer]) && (lpInspWorkInfo->m_nRecvACK_Rack[rack][layer] == FALSE))
				{
					bAllAck = FALSE;
				}
			}

			// 모든 Main에서 ACK가 전송 되었으면 Return TRUE
			if (bAllAck == TRUE)
				return TRUE;

			// ACK Wait Time Check
			eTick = ::GetTickCount();
			if ((eTick - sTick) > (DWORD)waitTime)
				return FALSE;

			ProcessMessage();
		}
	}

	return TRUE;
}


void CHseAgingApp::udp_processPacket(CString strPacket)
{
	CString recvPacket;
	CString recvIP;
	int IPaddr;
	int rack = 0;
	int layer = 0;
	int cmd = 0;
	int ntoken = 0;
	int target = 0;

	ntoken = strPacket.Find(_T("#"));
	if (ntoken == -1)	return;

	recvIP = strPacket.Left(ntoken);
	recvPacket = strPacket.Mid(ntoken + 1);

	IPaddr = _ttoi(recvIP.Right(recvIP.GetLength() - recvIP.ReverseFind(_T('.')) - 1));
	if (IPaddr == 0xFF)	return;

	rack = (IPaddr-1) / MAX_LAYER;
	layer = (IPaddr-1) % MAX_LAYER;

	target = _tcstol(recvPacket.Mid(PACKET_PT_SOURCE, 2), NULL, 16);
	cmd = _tcstol(recvPacket.Mid(PACKET_PT_CMD, 2), NULL, 16);

	if (IPaddr <= (MAX_RACK * MAX_LAYER))
	{
		lpInspWorkInfo->m_nMainEthConnect[rack][layer] = 5;	// Connect status count reset
	}

	// Message 처리
	switch (cmd)
	{
		case CMD_SET_CABLE_OPEN_CHECK:	// 0xA5
		{
			procParseCableOpenCheck(rack, layer, recvPacket);
			break;
		}
		case CMD_GET_AGING_STATUS:		// 0xAB
		{
			procParseAgingStatus(rack, layer, recvPacket);
			break;
		}
		case CMD_GET_POWER_MEASURE:		// 0xBD
		{
			procParseMeasurePower(rack, layer, recvPacket);
			break;
		}
		case CMD_GET_FW_VERSION:		// 0xFE
		{
			procParseFWVersion(rack, layer, recvPacket);
			break;
		}
		case CMD_GOTOBOOT_DSECTION:
		{
			procParseGotoBootSection(rack, layer, recvPacket);
			break;
		}
#if 0
		case CMD_GET_CAL_ALL_POWER_MEASURE:
		{
			procParseMeasurePower(grp, ma, ch1, ch2, recvPacket);
			break;
		}
		case CMD_SET_BMP_DONE_CHECK:
		{
			procParseDoneCheck(grp, ma, recvPacket);
			break;
		}
		case CMD_SRUN_STATUS_READ:
		{
			procParseSRunnerStatus(grp, ma, recvPacket);
			break;
		}
		case CMD_SRUN_DATA_WRITE:
		{
			procParseSRunnerWriteRet(grp, ma, recvPacket);
			break;
		}
		case CMD_SET_FUSINGSYSTEMINFO:
		{
			procParseFusingSystem(grp, ma, recvPacket);
			break;
		}
		case  CMD_GET_TCON_ABD_RESULT:
		{
			procParseTconABDResult(grp, ma, recvPacket);
			break;
		}
		case CMD_GET_CAL_READ_ADC:
		{
			procParseCalibrationReadADC(grp, ma, recvPacket);
			break;
		}
		case CMD_SET_CAL_ERASE_DATA:
		{
			procParseCalibrationErase(grp, ma, recvPacket);
			break;
		}
		case CMD_SET_CAL_VOLTAGE:
		{
			procParseCalibrationVoltage(grp, ma, recvPacket);
			break;
		}
		case CMD_SET_CAL_CURRENT:
		{
			procParseCalibrationCurrent(grp, ma, recvPacket);
			break;
		}
		case  CMD_GET_PANEL_SYNC_MEASURE:
		{
			procParsePanelSyncMeasure(grp, ma, recvPacket);
			break;
		}

		case CMD_GET_FPGA_VERSION:
		{
			procParseFpgaVersion(grp, ma, recvPacket);
			break;
		}

		case CMD_TIME_OUT:
		{
			m_nAckCmd = cmd;
			lpWorkInfo->m_nRecvCmdACK[ma] = FALSE;
			if (grp == GROUP_A) { m_nAckCmd_A = cmd;		lpWorkInfo->m_nRecvACK_A[ma] = FALSE; }
			else if (grp == GROUP_B) { m_nAckCmd_B = cmd;		lpWorkInfo->m_nRecvACK_B[ma] = FALSE; }
			else if (grp == GROUP_C) { m_nAckCmd_C = cmd;		lpWorkInfo->m_nRecvACK_C[ma] = FALSE; }
			else if (grp == GROUP_D) { m_nAckCmd_D = cmd;		lpWorkInfo->m_nRecvACK_D[ma] = FALSE; }
			else if (grp == GROUP_E) { m_nAckCmd_E = cmd;		lpWorkInfo->m_nRecvACK_E[ma] = FALSE; }
			else if (grp == GROUP_F) { m_nAckCmd_F = cmd;		lpWorkInfo->m_nRecvACK_F[ma] = FALSE; }
			else if (grp == GROUP_G) { m_nAckCmd_G = cmd;		lpWorkInfo->m_nRecvACK_G[ma] = FALSE; }
			else if (grp == GROUP_H) { m_nAckCmd_H = cmd;		lpWorkInfo->m_nRecvACK_H[ma] = FALSE; }
			return;
		}
#endif
	}

	// ACK Receive Check는 모든 Packet처리가 완료된 이후 Set한다.
	m_nAckCmdPG[rack][layer] = cmd;
	lpInspWorkInfo->m_nRecvACK_Rack[rack][layer] = TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHseAgingApp::procDioParseBoardInitial(CString packet)
{
	int retcode;

	retcode = _tcstol(packet.Mid(PACKET_PT_RET, 1), NULL, 16);
	//if (retcode == 0)
	{
		lpInspWorkInfo->m_nDioNeedInitial = FALSE;
	}
}

void CHseAgingApp::procDioParseInputRead(CString packet)
{
	int retcode;

	if (packet.GetLength() < 25)
		return;

	retcode = _tcstol(packet.Mid(PACKET_PT_RET, 1), NULL, 16);
	if (retcode == 0)
	{
		for (int i = 0; i < 10; i++)
		{
			int ptr = (PACKET_PT_DATA+1) + i;
			lpInspWorkInfo->m_nDioInputData[i] = _ttoi(packet.Mid(ptr, 1));
		}
	}
}

void CHseAgingApp::procDioParseFWVersion(CString packet)
{
	int retcode;

	retcode = _tcstol(packet.Mid(PACKET_PT_RET, 1), NULL, 16);

	if (retcode == 0)
	{
		CString sdata;
		sdata = packet.Mid(PACKET_PT_DATA, (packet.GetLength() - 17));

		lpInspWorkInfo->m_sDioFWVersion = sdata.Left(10);
	}
}

void CHseAgingApp::procDioParseGotoBootSection(CString packet)
{
	int retcode;

	retcode = _tcstol(packet.Mid(PACKET_PT_RET, 1), NULL, 16);
	if (retcode == 0)
	{
		if (m_nDownloadCountUp == TRUE)
		{
			m_nDownloadCountUp = FALSE;
			m_nDownloadReadyAckCount = m_nDownloadReadyAckCount + 1;
		}
	}
}

void CHseAgingApp::udp_processDioPacket(CString strPacket)
{
	CString recvPacket;
	CString recvIP;
	int cmd = 0;
	int ntoken = 0;
	int target = 0;

	ntoken = strPacket.Find(_T("#"));
	if (ntoken == -1)	return;

	recvIP = strPacket.Left(ntoken);
	recvPacket = strPacket.Mid(ntoken + 1);
	cmd = _tcstol(recvPacket.Mid(PACKET_PT_CMD, 2), NULL, 16);

	lpInspWorkInfo->m_nConnectInfo[CONNECT_DIO] = 3;

	// Message 처리
	switch (cmd)
	{
		case CMD_DIO_OUTPUT:		// LAMP Set
		{
			break;
		}
		case CMD_DIO_BOARD_INITIAL:	// DIO Board Initial 응답
		{
			procDioParseBoardInitial(recvPacket);
			break;
		}
		case CMD_DIO_INPUT:			// DIO Read
		{
			procDioParseInputRead(recvPacket);
			break;
		}
		case CMD_GET_FW_VERSION:
		{
			procDioParseFWVersion(recvPacket);
			break;
		}
		case CMD_GOTOBOOT_DSECTION:
		{
			procDioParseGotoBootSection(recvPacket);
			break;
		}
	}
	// ACK Receive Check는 모든 Packet처리가 완료된 이후 Set한다.
	int rack, layer;
	Lf_IPStringToRackInfo(recvIP, &rack, &layer);
	m_nAckCmdPG[rack][layer] = cmd;
	//m_nAckCmdDIO = cmd;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHseAgingApp::Gf_LoadSystemData()
{
	CString skey, sdata = _T("");

	Read_SysIniFile(_T("SYSTEM"), _T("CHAMBER_NO"), &lpSystemInfo->m_sChamberNo);
	Read_SysIniFile(_T("SYSTEM"), _T("EQP_NAME"), &lpSystemInfo->m_sEqpName);
	Read_SysIniFile(_T("SYSTEM"), _T("TEMP_RECORDER_PORT"), &lpSystemInfo->m_nTempRecorderPort);

	Read_SysIniFile(_T("SYSTEM"), _T("TEMP_CONTROLLER_PORT"), &lpSystemInfo->m_nTempControllerPort);

	Read_SysIniFile(_T("SYSTEM"), _T("REFRESH_AGING_STATUS_TIME"), &sdata);
	if (sdata.GetLength() == 0)		lpSystemInfo->m_fRefreshAgingStatusTime = 2.0f;
	else							lpSystemInfo->m_fRefreshAgingStatusTime = (float)_tstof(sdata);

	Read_SysIniFile(_T("SYSTEM"), _T("REFRESH_POWER_MEASURE_TIME"), &sdata);
	if (sdata.GetLength() == 0)		lpSystemInfo->m_fRefreshPowerMeasureTime = 2.0f;
	else							lpSystemInfo->m_fRefreshPowerMeasureTime = (float)_tstof(sdata);

	Read_SysIniFile(_T("SYSTEM"), _T("TEMP_LOG_INTERVAL"), &lpSystemInfo->m_nTempLogInterval);
	Read_SysIniFile(_T("SYSTEM"), _T("SENSING_LOG_INTERVAL"), &lpSystemInfo->m_nSensingLogInterval);


	lpSystemInfo->m_nMesIDType = MES_ID_TYPE_PID;
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		skey.Format(_T("LAST_MODELNAME_RACK%d"), (rack + 1));
		Read_SysIniFile(_T("SYSTEM"), skey, &lpSystemInfo->m_sLastModelName[rack]);
	}

	Read_SysIniFile(_T("MES"), _T("MES_SERVICE_PORT"), &lpSystemInfo->m_sMesServicePort);
	Read_SysIniFile(_T("MES"), _T("MES_NETWORK"), &lpSystemInfo->m_sMesNetWork);
	Read_SysIniFile(_T("MES"), _T("MES_DAEMON_PORT"), &lpSystemInfo->m_sMesDaemonPort);
	Read_SysIniFile(_T("MES"), _T("MES_LOCAL_SUBJECT"), &lpSystemInfo->m_sMesLocalSubject);
	Read_SysIniFile(_T("MES"), _T("MES_REMOTE_SUBJECT"), &lpSystemInfo->m_sMesRemoteSubject);
	Read_SysIniFile(_T("EAS"), _T("EAS_SERVICE_PORT"), &lpSystemInfo->m_sEasServicePort);
	Read_SysIniFile(_T("EAS"), _T("EAS_NETWORK"), &lpSystemInfo->m_sEasNetWork);
	Read_SysIniFile(_T("EAS"), _T("EAS_DAEMON_PORT"), &lpSystemInfo->m_sEasDaemonPort);
	Read_SysIniFile(_T("EAS"), _T("EAS_LOCAL_SUBJECT"), &lpSystemInfo->m_sEasLocalSubject);
	Read_SysIniFile(_T("EAS"), _T("EAS_REMOTE_SUBJECT"), &lpSystemInfo->m_sEasRemoteSubject);

	Read_SysIniFile(_T("RMS"), _T("RMS_SERVICE_PORT"), &lpSystemInfo->m_sRmsServicePort);
	Read_SysIniFile(_T("RMS"), _T("RMS_NETWORK"), &lpSystemInfo->m_sRmsNetWork);
	Read_SysIniFile(_T("RMS"), _T("RMS_DAEMON_PORT"), &lpSystemInfo->m_sRmsDaemonPort);
	Read_SysIniFile(_T("RMS"), _T("RMS_LOCAL_SUBJECT"), &lpSystemInfo->m_sRmsLocalSubject);
	Read_SysIniFile(_T("RMS"), _T("RMS_REMOTE_SUBJECT"), &lpSystemInfo->m_sRmsRemoteSubject);
	
	Read_SysIniFile(_T("SW_VERSION"), _T("SW_VERSION"), &lpSystemInfo->m_SwVersion);
	
	Read_SysIniFile(_T("COUNT"), _T("AGING_COUNT"), &lpSystemInfo->m_Aging_Count);
	Read_SysIniFile(_T("COUNT"), _T("AGING_NG_COUNT"), &lpSystemInfo->m_Aging_Ng_Count);

	CString sSection, sKey, sValue;
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		sKey.Format(_T("RACK%d_BCR_ID"), rack + 1);
		Read_SysIniFile(_T("SYSTEM"), sKey, &lpInspWorkInfo->m_sRackID[rack]);

		for (int layer = 0; layer < MAX_LAYER; layer++)
		{
			for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
			{
				sSection.Format(_T("MES_PID_RACK%d"), rack + 1);

				sKey.Format(_T("RACK%d_LAYER%d_CH%d_USE"), rack + 1, layer + 1, ch + 1);
				Read_MesPIDInfo(sSection, sKey, &sValue);
				lpInspWorkInfo->m_bMesChannelUse[rack][layer][ch] = _ttoi(sValue);

				sKey.Format(_T("RACK%d_LAYER%d_CH%d"), rack + 1, layer + 1, ch + 1);
				Read_MesPIDInfo(sSection, sKey, &lpInspWorkInfo->m_sMesPanelID[rack][layer][ch]);

				sKey.Format(_T("BCR_RACK%d_LAYER%d_CH%d"), rack + 1, layer + 1, ch + 1);
				Read_MesPIDInfo(sSection, sKey, &lpInspWorkInfo->m_sMesBcrChID[rack][layer][ch]);
			}
		}
	}

}

void CHseAgingApp::Gf_loadModelData(CString modelName)
{
	CString sdata;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Timing Set
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_MAIN_CLOCK"), &lpModelInfo->m_fTimingMainClock);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_HOR_TOTAL"), &lpModelInfo->m_nTimingHorTotal);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_HOR_ACTIVE"), &lpModelInfo->m_nTimingHorActive);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_HOR_WIDTH"), &lpModelInfo->m_nTimingHorWidth);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_HOR_BACKPORCH"), &lpModelInfo->m_nTimingHorBP);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_HOR_FRONTPORCH"), &lpModelInfo->m_nTimingHorFP);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_VER_TOTAL"), &lpModelInfo->m_nTimingVerTotal);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_VER_ACTIVE"), &lpModelInfo->m_nTimingVerActive);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_VER_WIDTH"), &lpModelInfo->m_nTimingVerWidth);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_VER_BACKPORCH"), &lpModelInfo->m_nTimingVerBP);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TIMING_VER_FRONTPORCH"), &lpModelInfo->m_nTimingVerFP);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// LCM Set
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_SIGNAL_TYPE"), &lpModelInfo->m_nLcmSignalType);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_PIXEL_TYPE"), &lpModelInfo->m_nLcmPixelType);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_ODD_EVEN"), &lpModelInfo->m_nLcmOddEven);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_SIGNAL_BIT"), &lpModelInfo->m_nLcmSignalBit);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_BIT_SWAP"), &lpModelInfo->m_nLcmBitSwap);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("LCM_LVDS_RS_SEL"), &lpModelInfo->m_nLcmLvdsRsSel);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Inverter & PWM Set
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("DIMMING_SEL"), &lpModelInfo->m_nDimmingSel);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("PWM_FREQ"), &lpModelInfo->m_nPwmFreq);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("PWM_DUTY"), &lpModelInfo->m_nPwmDuty);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VBR_VOLT"), &lpModelInfo->m_fVbrVolt);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Function Set
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("CABLE_OPEN"), &lpModelInfo->m_nFuncCableOpen);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Power Sequence Set
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ1"), &lpModelInfo->m_nPowerOnSeq1);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ2"), &lpModelInfo->m_nPowerOnSeq2);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ3"), &lpModelInfo->m_nPowerOnSeq3);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ4"), &lpModelInfo->m_nPowerOnSeq4);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ5"), &lpModelInfo->m_nPowerOnSeq5);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ6"), &lpModelInfo->m_nPowerOnSeq6);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ7"), &lpModelInfo->m_nPowerOnSeq7);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ8"), &lpModelInfo->m_nPowerOnSeq8);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ9"), &lpModelInfo->m_nPowerOnSeq9);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ10"), &lpModelInfo->m_nPowerOnSeq10);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_SEQ11"), &lpModelInfo->m_nPowerOnSeq11);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY1"), &lpModelInfo->m_nPowerOnDelay1);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY2"), &lpModelInfo->m_nPowerOnDelay2);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY3"), &lpModelInfo->m_nPowerOnDelay3);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY4"), &lpModelInfo->m_nPowerOnDelay4);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY5"), &lpModelInfo->m_nPowerOnDelay5);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY6"), &lpModelInfo->m_nPowerOnDelay6);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY7"), &lpModelInfo->m_nPowerOnDelay7);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY8"), &lpModelInfo->m_nPowerOnDelay8);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY9"), &lpModelInfo->m_nPowerOnDelay9);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_ON_DELAY10"), &lpModelInfo->m_nPowerOnDelay10);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ1"), &lpModelInfo->m_nPowerOffSeq1);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ2"), &lpModelInfo->m_nPowerOffSeq2);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ3"), &lpModelInfo->m_nPowerOffSeq3);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ4"), &lpModelInfo->m_nPowerOffSeq4);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ5"), &lpModelInfo->m_nPowerOffSeq5);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ6"), &lpModelInfo->m_nPowerOffSeq6);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ7"), &lpModelInfo->m_nPowerOffSeq7);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ8"), &lpModelInfo->m_nPowerOffSeq8);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ9"), &lpModelInfo->m_nPowerOffSeq9);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ10"), &lpModelInfo->m_nPowerOffSeq10);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_SEQ11"), &lpModelInfo->m_nPowerOffSeq11);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY1"), &lpModelInfo->m_nPowerOffDelay1);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY2"), &lpModelInfo->m_nPowerOffDelay2);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY3"), &lpModelInfo->m_nPowerOffDelay3);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY4"), &lpModelInfo->m_nPowerOffDelay4);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY5"), &lpModelInfo->m_nPowerOffDelay5);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY6"), &lpModelInfo->m_nPowerOffDelay6);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY7"), &lpModelInfo->m_nPowerOffDelay7);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY8"), &lpModelInfo->m_nPowerOffDelay8);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY9"), &lpModelInfo->m_nPowerOffDelay9);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY10"), &lpModelInfo->m_nPowerOffDelay10);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("POWER_OFF_DELAY"), &lpModelInfo->m_nPowerOffDelay);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Power Set
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_VOLT"), &lpModelInfo->m_fVccVolt);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_VOLT_OFFSET"), &lpModelInfo->m_fVccVoltOffset);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_LIMIT_VOLT_LOW"), &lpModelInfo->m_fVccLimitVoltLow);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_LIMIT_VOLT_HIGH"), &lpModelInfo->m_fVccLimitVoltHigh);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_LIMIT_CURR_LOW"), &lpModelInfo->m_fVccLimitCurrLow);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VCC_LIMIT_CURR_HIGH"), &lpModelInfo->m_fVccLimitCurrHigh);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_VOLT"), &lpModelInfo->m_fVblVolt);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_VOLT_OFFSET"), &lpModelInfo->m_fVblVoltOffset);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_LIMIT_VOLT_LOW"), &lpModelInfo->m_fVblLimitVoltLow);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_LIMIT_VOLT_HIGH"), &lpModelInfo->m_fVblLimitVoltHigh);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_LIMIT_CURR_LOW"), &lpModelInfo->m_fVblLimitCurrLow);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("VBL_LIMIT_CURR_HIGH"), &lpModelInfo->m_fVblLimitCurrHigh);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Aging Set
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("AGING_TIME_HH"), &lpModelInfo->m_nAgingTimeHH);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("AGING_TIME_MM"), &lpModelInfo->m_nAgingTimeMM);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("AGING_TIME_MINUTE"), &lpModelInfo->m_nAgingTimeMinute);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("AGING_END_WAIT_TIME"), &sdata);
	if (sdata.GetLength() == 0)		lpModelInfo->m_nAgingEndWaitTime = 5;
	else							lpModelInfo->m_nAgingEndWaitTime = _ttoi(sdata);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Operation Set
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TEMPERATURE_USE"), &lpModelInfo->m_nOpeTemperatureUse);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TEMPERATURE_MIN"), &lpModelInfo->m_nOpeTemperatureMin);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("TEMPERATURE_MAX"), &lpModelInfo->m_nOpeTemperatureMax);
	Read_ModelFile(modelName, _T("MODEL_INFO"), _T("DOOR_USE"), &lpModelInfo->m_nOpeDoorUse);
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void CHseAgingApp::Gf_sumWriteSummaryLog(int rack, int layer, int channel)
{
	FILE* fp;

	BOOL bNewCsv = FALSE;
	char filepath[128] = { 0 };
	char buff[2048] = { 0 };
	CString sResult = _T("NG");

	SYSTEMTIME sysTime;
	::GetSystemTime(&sysTime);
	CTime time = CTime::GetCurrentTime();

	if ((_access(".\\Logs\\SummaryLog", 0)) == -1)
		_mkdir(".\\Logs\\SummaryLog");

	sprintf_s(filepath, ".\\Logs\\SummaryLog\\Summary_%04d%02d%02d.csv", time.GetYear(), time.GetMonth(), time.GetDay());
	fopen_s(&fp, filepath, "r+");
	if (fp == NULL)
	{
		delayMs(1);
		fopen_s(&fp, filepath, "a+");
		if (fp == NULL) // 2007-08-01 : fseek.c(101) error
		{
			if ((_access(filepath, 2)) != -1) // 2007-09-02 : fseek.c(101) error
			{
				delayMs(1);
				fopen_s(&fp, filepath, "a+");
				if (fp == NULL) // 2007-09-02 : fseek.c(101) error
				{
					return;
				}
			}
		}
		bNewCsv = TRUE;
	}

	CString softwareVer;
	TCHAR szSwVer[1024] = { 0, };
	GetModuleFileName(NULL, szSwVer, 1024);
	softwareVer.Format(_T("%s"), szSwVer);
	softwareVer = softwareVer.Mid(softwareVer.ReverseFind(_T('\\')) + 1);
	softwareVer.Delete(softwareVer.GetLength() - 4, 4);

	m_summaryInfo[rack][layer][channel].m_sumData[SUM_SW_VER] = softwareVer;
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_FW_VER] = lpInspWorkInfo->m_sMainFWVersion[rack][layer];
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_MODEL] = lpSystemInfo->m_sLastModelName[rack];
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_EQP_NAME] = lpSystemInfo->m_sEqpName;
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_PID] = lpInspWorkInfo->m_sMesPanelID[rack][layer][channel];
	//m_summaryInfo[rack][layer][channel].m_sumData[SUM_PID] = m_pApp->m_summaryInfo[rack][layer][channel].m_sumData[SUM_PID];
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_RACK].Format(_T("%d"), rack + 1);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_LAYER].Format(_T("%d"), layer + 1);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_CHANNEL].Format(_T("%d"), channel + 1);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_USER_ID] = m_pApp->m_sUserID;

	m_summaryInfo[rack][layer][channel].m_sumData[SUM_TEMP_MIN].Format(_T("%.1f"), lpInspWorkInfo->m_fOpeAgingTempMin[rack]);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_TEMP_MAX].Format(_T("%.1f"), lpInspWorkInfo->m_fOpeAgingTempMax[rack]);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_TEMP_AVG].Format(_T("%.1f"), (float)(lpInspWorkInfo->m_fOpeAgingTempAvg[rack] / (float)lpInspWorkInfo->m_nAgingTempMeasCount[rack]));
	//m_summaryInfo[rack][layer][channel].m_sumData[SUM_TEMP_AVG].Format(_T("%.1f"), (float)(lpInspWorkInfo->m_fOpeAgingTempAvg[rack] / (float)lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][channel]));

	/*CString dbg;
	dbg.Format(_T("tempavg = %f, tempCount = %f"),
		(float)(lpInspWorkInfo->m_fOpeAgingTempAvg[rack],
			(float)lpInspWorkInfo->m_nAgingTempMeasCount[rack][layer][channel]));

	OutputDebugString(dbg);*/

	m_summaryInfo[rack][layer][channel].m_sumData[SUM_VCC_MIN].Format(_T("%.2f"), lpInspWorkInfo->m_fOpeAgingVccMin[rack][layer][channel]);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_VCC_MAX].Format(_T("%.2f"), lpInspWorkInfo->m_fOpeAgingVccMax[rack][layer][channel]);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_VCC_AVG].Format(_T("%.2f"), (float)(lpInspWorkInfo->m_fOpeAgingVccAvg[rack][layer][channel] / (float)lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][channel]));

	/*dbg.Format(_T("vccavg = %f, vccCount = %f"),
		(float)(lpInspWorkInfo->m_fOpeAgingVccAvg[rack][layer][channel],
			(float)lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][channel]));

	OutputDebugString(dbg);*/

	m_summaryInfo[rack][layer][channel].m_sumData[SUM_ICC_MIN].Format(_T("%.2f"), lpInspWorkInfo->m_fOpeAgingIccMin[rack][layer][channel]);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_ICC_MAX].Format(_T("%.2f"), lpInspWorkInfo->m_fOpeAgingIccMax[rack][layer][channel]);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_ICC_AVG].Format(_T("%.2f"), (float)(lpInspWorkInfo->m_fOpeAgingIccAvg[rack][layer][channel] / (float)lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][channel]));

	/*dbg.Format(_T("iccavg = %f, iccCount = %f"),
		(float)(lpInspWorkInfo->m_fOpeAgingIccAvg[rack][layer][channel],
			(float)lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][channel]));

	OutputDebugString(dbg);*/

	m_summaryInfo[rack][layer][channel].m_sumData[SUM_VBL_MIN].Format(_T("%.2f"), lpInspWorkInfo->m_fOpeAgingVblMin[rack][layer][channel]);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_VBL_MAX].Format(_T("%.2f"), lpInspWorkInfo->m_fOpeAgingVblMax[rack][layer][channel]);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_VBL_AVG].Format(_T("%.2f"), (float)(lpInspWorkInfo->m_fOpeAgingVblAvg[rack][layer][channel] / (float)lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][channel]));

	/*dbg.Format(_T("vblavg = %f, vblCount = %f"),
		(float)(lpInspWorkInfo->m_fOpeAgingVblAvg[rack][layer][channel],
			(float)lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][channel]));

	OutputDebugString(dbg);*/

	m_summaryInfo[rack][layer][channel].m_sumData[SUM_IBL_MIN].Format(_T("%.2f"), lpInspWorkInfo->m_fOpeAgingIblMin[rack][layer][channel]);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_IBL_MAX].Format(_T("%.2f"), lpInspWorkInfo->m_fOpeAgingIblMax[rack][layer][channel]);
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_IBL_AVG].Format(_T("%.2f"), (float)(lpInspWorkInfo->m_fOpeAgingIblAvg[rack][layer][channel] / (float)lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][channel]));

	/*dbg.Format(_T("iblavg = %f, iblCount = %f"),
		(float)(lpInspWorkInfo->m_fOpeAgingIblAvg[rack][layer][channel],
			(float)lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][channel]));

	OutputDebugString(dbg);*/

	if (bNewCsv == TRUE)
	{
		sprintf_s(buff, "SW_VER,FW_VER,MODEL,EQP_NAME,PID,RACK,LAYER,CHANNEL,USER_ID,AGING_TIME,START_TIME,END_TIME,RESULT,FAILED_MESSAGE,FAILED_MESSAGE_TIME,VCC,ICC,VBL,IBL,AGING_TEMP_MIN,AGING_TEMP_MAX,AGING_TEMP_AVG,AGING_VCC_MIN,AGING_VCC_MAX,AGING_VCC_AVG,AGING_ICC_MIN,AGING_ICC_MAX,AGING_ICC_AVG,AGING_VBL_MIN,AGING_VBL_MAX,AGING_VBL_AVG,AGING_IBL_MIN,AGING_IBL_MAX,AGING_IBL_AVG\n");
		fprintf(fp, "%s", buff);
	}

	if (m_summaryInfo[rack][layer][channel].m_sumData[SUM_FAILED_MESSAGE].IsEmpty())
	{
		sResult.Format(_T("OK"));
	}
	m_summaryInfo[rack][layer][channel].m_sumData[SUM_RESULT] = sResult;

	if (sResult == "NG")
	{
		lpSystemInfo->m_Aging_Ng_Count++;
	}

	CString strFailMsg = m_summaryInfo[rack][layer][channel].m_sumData[SUM_FAILED_MESSAGE];
	CString strFailTime = m_summaryInfo[rack][layer][channel].m_sumData[SUM_FAILED_MESSAGE_TIME];

	if (strFailMsg.FindOneOf(_T(",\r\n")) >= 0)
		strFailMsg = _T("\"") + strFailMsg + _T("\"");
	if (strFailTime.FindOneOf(_T(",\r\n")) >= 0)
		strFailTime = _T("\"") + strFailTime + _T("\"");
	

	sprintf_s(buff, "%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S,%S\n",
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_SW_VER].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_FW_VER].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_MODEL].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_EQP_NAME].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_PID].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_RACK].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_LAYER].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_CHANNEL].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_USER_ID].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_AGING_TIME].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_START_TIME].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_END_TIME].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_RESULT].GetBuffer(0),
		/*m_summaryInfo[rack][layer][channel].m_sumData[SUM_FAILED_MESSAGE].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_FAILED_MESSAGE_TIME].GetBuffer(0),*/
		strFailMsg.GetBuffer(),
		strFailTime.GetBuffer(),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_MEAS_VCC].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_MEAS_ICC].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_MEAS_VBL].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_MEAS_IBL].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_TEMP_MIN].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_TEMP_MAX].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_TEMP_AVG].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_VCC_MIN].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_VCC_MAX].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_VCC_AVG].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_ICC_MIN].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_ICC_MAX].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_ICC_AVG].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_VBL_MIN].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_VBL_MAX].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_VBL_AVG].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_IBL_MIN].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_IBL_MAX].GetBuffer(0),
		m_summaryInfo[rack][layer][channel].m_sumData[SUM_IBL_AVG].GetBuffer(0)
		);

	fseek(fp, 0L, SEEK_END);
	fprintf(fp, "%s", buff);

	fclose(fp);
}

void CHseAgingApp::Gf_sumInitSummaryInfo(int rack)
{
	CString skey;

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			for (int sumIndex = 0; sumIndex < SUM_INFO_MAX; sumIndex++)
			{
				m_summaryInfo[rack][layer][ch].m_sumData[sumIndex].Empty();
			}

			skey.Format(_T("RACK%d_LAYER%d_CH%d"), rack+1,  layer+1, ch+1);
			Write_SummaryInfo(_T("PID"), skey, _T(""));
			Write_SummaryInfo(_T("AGING_TIME"), skey, _T(""));
			Write_SummaryInfo(_T("START_TIME"), skey, _T(""));
			Write_SummaryInfo(_T("END_TIME"), skey, _T(""));
			Write_SummaryInfo(_T("AGN_IN_COMPLETE"), skey, _T(""));
			Write_SummaryInfo(_T("FAIL_MESSAGE"), skey, _T(""));
			Write_SummaryInfo(_T("FAIL_TIME"), skey, _T(""));
			Write_SummaryInfo(_T("VCC"), skey, _T(""));
			Write_SummaryInfo(_T("ICC"), skey, _T(""));
			Write_SummaryInfo(_T("VBL"), skey, _T(""));
			Write_SummaryInfo(_T("IBL"), skey, _T(""));
		}
	}
}

void CHseAgingApp::Gf_sumSetSummaryInfo(int rack, int layer, int ch, int sumIndex, CString sdata)
{
	m_summaryInfo[rack][layer][ch].m_sumData[sumIndex].Format(_T("%s"), sdata);

	CString section, skey;
	skey.Format(_T("RACK%d_LAYER%d_CH%d"), rack + 1, layer + 1, ch + 1);
	if (sumIndex == SUM_AGING_TIME)		section = _T("AGING_TIME");
	if (sumIndex == SUM_START_TIME)		section = _T("START_TIME");
	if (sumIndex == SUM_END_TIME)		section = _T("END_TIME");
	if (sumIndex == SUM_MEAS_VCC)		section = _T("VCC");
	if (sumIndex == SUM_MEAS_ICC)		section = _T("ICC");
	if (sumIndex == SUM_MEAS_VBL)		section = _T("VBL");
	if (sumIndex == SUM_MEAS_IBL)		section = _T("IBL");

	Write_SummaryInfo(section, skey, sdata);
}

void CHseAgingApp::Gf_sumSetStartTime(int rack)
{
	CString agingTime, startTime;
	CTime time = CTime::GetCurrentTime();

	agingTime.Format(_T("%dmin"), lpModelInfo->m_nAgingTimeMinute);
	startTime.Format(_T("%04d%02d%02d %02d:%02d:%02d"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			Gf_sumSetSummaryInfo(rack, layer, ch, SUM_AGING_TIME, agingTime);
			Gf_sumSetSummaryInfo(rack, layer, ch, SUM_START_TIME, startTime);
		}
	}
}

void CHseAgingApp::Gf_sumSetEndTime(int rack)
{
	CString startTime;
	CTime time = CTime::GetCurrentTime();

	startTime.Format(_T("%04d%02d%02d %02d:%02d:%02d"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			Gf_sumSetSummaryInfo(rack, layer, ch, SUM_END_TIME, startTime);
		}
	}
}

void CHseAgingApp::Gf_sumSetFailedInfo(int rack, int layer, int ch, CString failMessage)
{
	CString failTime;
	CTime time = CTime::GetCurrentTime();

	Gf_sumSetSummaryInfo(rack, layer, ch, SUM_FAILED_MESSAGE, failMessage);

	failTime.Format(_T("%04d%02d%02d %02d:%02d:%02d"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());
	Gf_sumSetSummaryInfo(rack, layer, ch, SUM_FAILED_MESSAGE_TIME, failTime);
}

void CHseAgingApp::Gf_sumSetPowerMeasureInfo(int rack)
{
	CString vcc, vccLcm, vbl, icc, ibl, vdd, idd;
	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			vcc.Format(_T("%.2f"), (float)(lpInspWorkInfo->m_nMeasVCC[rack][layer][ch] / 100.0));
			Gf_sumSetSummaryInfo(rack, layer, ch, SUM_MEAS_VCC, vcc);

			icc.Format(_T("%d"), lpInspWorkInfo->m_nMeasICC[rack][layer][ch]);
			Gf_sumSetSummaryInfo(rack, layer, ch, SUM_MEAS_ICC, icc);

			vbl.Format(_T("%.2f"), (float)(lpInspWorkInfo->m_nMeasVBL[rack][layer][ch] / 100.0));
			Gf_sumSetSummaryInfo(rack, layer, ch, SUM_MEAS_VBL, vbl);

			ibl.Format(_T("%d"), lpInspWorkInfo->m_nMeasIBL[rack][layer][ch]);
			Gf_sumSetSummaryInfo(rack, layer, ch, SUM_MEAS_IBL, ibl);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CHseAgingApp::Gf_gmesInitServer(BOOL nServerType)
{
	if ((DEBUG_GMES_TEST_SERVER == TRUE) && (nServerType == SERVER_MES))
	{
		pCimNet->SetMachineName(lpSystemInfo->m_sEqpName);
		pCimNet->SetLocalTest(nServerType);
	}
	else if ((DEBUG_GMES_TEST_SERVER == TRUE) && (nServerType == SERVER_EAS))
	{
		pCimNet->SetLocalTest(nServerType);
	}
	else if ((DEBUG_GMES_TEST_SERVER == TRUE) && (nServerType == SERVER_RMS))
	{
		pCimNet->SetLocalTest(nServerType);
	}


	if (pCimNet->Init(nServerType) == TRUE)
	{
		return TRUE;
	}

	return FALSE;
}


BOOL CHseAgingApp::Gf_gmesConnect(int nServerType)
{
	pCimNet->SetMachineName(lpSystemInfo->m_sEqpName);

	if (DEBUG_GMES_TEST_SERVER)
		pCimNet->SetLocalTest(nServerType);

	if (pCimNet->ConnectTibRv(nServerType) == TRUE)
	{
		return TRUE;
	}

	return FALSE;
}

void CHseAgingApp::Gf_gmesSetValueAgcm(int rack, int layer, int ch)
{
	CString strPanelID, strChannelID;

	strPanelID.Format(_T("%s"), lpInspWorkInfo->m_sMesPanelID[rack][layer][ch]);
	strChannelID.Format(_T("%s"), lpSystemInfo->m_sChannelID[rack][layer][ch]);

	if (lpSystemInfo->m_nMesIDType == MES_ID_TYPE_PID)
	{
		pCimNet->SetPanelID(strPanelID);
		pCimNet->SetSerialNumber(_T(""));
		pCimNet->SetChannelID(strChannelID);
	}
	else if (lpSystemInfo->m_nMesIDType == MES_ID_TYPE_SERIAL)
	{
		pCimNet->SetPanelID(_T(""));
		pCimNet->SetSerialNumber(strPanelID);
		pCimNet->SetChannelID(strChannelID);
	}

	pCimNet->SetRwkCode(_T(""));
}

void CHseAgingApp::Lf_gmesSetValueAPDR(int rack, int layer, int ch)
{
	SUMMARYINFO tempAPD;
	CString sdata, sValue, sAPDInfo;

	/*pCimNet->SetPanelID(lpInspWorkInfo->m_sMesPchkRtnPID[rack][layer][ch]);
	pCimNet->SetSerialNumber(_T(""));*/


	tempAPD = m_summaryInfo[rack][layer][ch];

	pCimNet->SetPanelID(tempAPD.m_sumData[SUM_PID]);
	pCimNet->SetSerialNumber(_T(""));
	pCimNet->SetModelName(tempAPD.m_sumData[SUM_MODEL]);


	/*tempAPD.m_sumData[SUM_START_TIME].Replace(_T(":"), _T("_"));
	tempAPD.m_sumData[SUM_END_TIME].Replace(_T(":"), _T("_"));*/
	tempAPD.m_sumData[SUM_START_TIME].Replace(_T(":"), _T(""));
	tempAPD.m_sumData[SUM_START_TIME].Replace(_T(" "), _T(""));

	tempAPD.m_sumData[SUM_END_TIME].Replace(_T(":"), _T(""));
	tempAPD.m_sumData[SUM_END_TIME].Replace(_T(" "), _T(""));


	sdata.Format(_T("AGING_MONITORING:EM_NO:%s,"), tempAPD.m_sumData[SUM_EQP_NAME]);						sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:PID:%s,"), tempAPD.m_sumData[SUM_PID]);							sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:SNID:,"));														sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:MODEL:%s,"), tempAPD.m_sumData[SUM_MODEL]);						sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:CHAMBER:%s,"), lpSystemInfo->m_sChamberNo);						sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:RACK:%s,"), tempAPD.m_sumData[SUM_RACK]);							sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:CHANNEL:%s,"), tempAPD.m_sumData[SUM_CHANNEL]);					sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_RESULT:%s,"), tempAPD.m_sumData[SUM_RESULT]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_TIME:%s,"), tempAPD.m_sumData[SUM_AGING_TIME]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_TIME_START:%s,"), tempAPD.m_sumData[SUM_START_TIME]);		sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_TIME_END:%s,"), tempAPD.m_sumData[SUM_END_TIME]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_TEMP_MIN:%s,"), tempAPD.m_sumData[SUM_TEMP_MIN]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_TEMP_MAX:%s,"), tempAPD.m_sumData[SUM_TEMP_MAX]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_TEMP_AVG:%s,"), tempAPD.m_sumData[SUM_TEMP_AVG]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VCC_MIN:%s,"), tempAPD.m_sumData[SUM_VCC_MIN]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VCC_MAX:%s,"), tempAPD.m_sumData[SUM_VCC_MAX]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VCC_AVG:%s,"), tempAPD.m_sumData[SUM_VCC_AVG]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_ICC_MIN:%s,"), tempAPD.m_sumData[SUM_ICC_MIN]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_ICC_MAX:%s,"), tempAPD.m_sumData[SUM_ICC_MAX]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_ICC_AVG:%s,"), tempAPD.m_sumData[SUM_ICC_AVG]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VBL_MIN:%s,"), tempAPD.m_sumData[SUM_VBL_MIN]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VBL_MAX:%s,"), tempAPD.m_sumData[SUM_VBL_MAX]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VBL_AVG:%s,"), tempAPD.m_sumData[SUM_VBL_AVG]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_IBL_MIN:%s,"), tempAPD.m_sumData[SUM_IBL_MIN]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_IBL_MAX:%s,"), tempAPD.m_sumData[SUM_IBL_MAX]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_IBL_AVG:%s,"), tempAPD.m_sumData[SUM_IBL_AVG]);				sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_TIME_SET:%d,"), lpInspWorkInfo->m_nAgingSetTime[rack]);		sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_TEMP_SET:,"));												sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VCC_SET:%.2f,"), lpInspWorkInfo->m_fOpeVccSetting[rack]);	sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_ICC_SET:,"));												sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VBL_SET:%.2f,"), lpInspWorkInfo->m_fOpeVblSetting[rack]);	sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_IBL_SET:,"));												sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_TEMP_SENSOR_SET_MIN:%d,"), lpInspWorkInfo->m_nOpeTemperatureMin[rack]);		sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_TEMP_SENSOR_SET_MAX:%d,"), lpInspWorkInfo->m_nOpeTemperatureMax[rack]);		sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VCC_SENSOR_SET_MIN:%.2f,"), lpInspWorkInfo->m_fOpeVccSetMin[rack]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VCC_SENSOR_SET_MAX:%.2f,"), lpInspWorkInfo->m_fOpeVccSetMax[rack]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_ICC_SENSOR_SET_MIN:%.2f,"), lpInspWorkInfo->m_fOpeIccSetMin[rack]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_ICC_SENSOR_SET_MAX:%.2f,"), lpInspWorkInfo->m_fOpeIccSetMax[rack]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VBL_SENSOR_SET_MIN:%.2f,"), lpInspWorkInfo->m_fOpeVblSetMin[rack]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_VBL_SENSOR_SET_MAX:%.2f,"), lpInspWorkInfo->m_fOpeVblSetMax[rack]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_IBL_SENSOR_SET_MIN:%.2f,"), lpInspWorkInfo->m_fOpeIblSetMin[rack]);			sAPDInfo.Append(sdata);
	sdata.Format(_T("AGING_MONITORING:AGING_IBL_SENSOR_SET_MAX:%.2f,"), lpInspWorkInfo->m_fOpeIblSetMax[rack]);			sAPDInfo.Append(sdata);

	sAPDInfo.Replace(_T("-"), _T("_"));				// 하이픈(-)은 언더바(_)로 변경
	sAPDInfo.Replace(_T(" "), _T("_"));				// 공백은 언더바(_)로 변경
	sAPDInfo.Replace(_T("("), _T("_"));				// 괄호는 언더바(_)로 변경
	sAPDInfo.Replace(_T(")"), _T("_"));				// 괄호는 언더바(_)로 변경
	sAPDInfo.MakeUpper();							// 대문자 변경

	pCimNet->SetAPDInfo(sAPDInfo);
}

void CHseAgingApp::Lf_setGmesValueAGN_INSP(int rack, int layer, int ch)
{
	CString strPanelID;
	CString strRwkCD;

	strPanelID.Format(_T("%s"), lpInspWorkInfo->m_sMesPanelID[rack][layer][ch]);
	if (lpSystemInfo->m_nMesIDType == MES_ID_TYPE_PID)
	{
		pCimNet->SetPanelID(strPanelID);
		pCimNet->SetSerialNumber(_T(""));
	}
	else if (lpSystemInfo->m_nMesIDType == MES_ID_TYPE_SERIAL)
	{
		pCimNet->SetPanelID(_T(""));
		pCimNet->SetSerialNumber(strPanelID);
	}
}

void CHseAgingApp::Lf_setGmesValueDSPM(int rack, int layer, int ch)
{
	CString strPanelID;
	CString strRwkCD;

	strPanelID.Format(_T("%s"), lpInspWorkInfo->m_sMesPanelID[rack][layer][ch]);
	if (lpSystemInfo->m_nMesIDType == MES_ID_TYPE_PID)
	{
		pCimNet->SetPanelID(strPanelID);
		pCimNet->SetSerialNumber(_T(""));
	}
	else if (lpSystemInfo->m_nMesIDType == MES_ID_TYPE_SERIAL)
	{
		pCimNet->SetPanelID(_T(""));
		pCimNet->SetSerialNumber(strPanelID);
	}
	pCimNet->SetDurableID(lpInspWorkInfo->m_sRackID[rack]);
	pCimNet->SetSlotNo(_T("1"));
	pCimNet->SetActFlag(_T("A"));
}

void CHseAgingApp::Lf_setGmesValueDMIN(int rack)
{
	pCimNet->SetDurableID(lpInspWorkInfo->m_sRackID[rack]);
}

void CHseAgingApp::Lf_setGmesValueDMOU(int rack)
{
	pCimNet->SetDurableID(lpInspWorkInfo->m_sRackID[rack]);
}

void CHseAgingApp::Lf_setGmesValueUNDO(int rack, int layer, int ch)
{
	pCimNet->SetPanelID(lpInspWorkInfo->m_sMesPchkRtnPID[rack][layer][ch]);
}

CString CHseAgingApp::Gf_gmesGetUserID()
{
	pCimNet->GetFieldData(&m_pApp->m_sUserID, _T("USER_ID"));
	return m_pApp->m_sUserID;
}

CString CHseAgingApp::Gf_gmesGetUserName()
{
	pCimNet->GetFieldData(&m_pApp->m_sUserName, _T("USER_NAME"));
	return m_pApp->m_sUserName;
}

CString CHseAgingApp::Gf_gmesGetRTNCD()
{
	CString strBuff;

	pCimNet->GetFieldData(&strBuff, _T("RTN_CD"));
	return strBuff;
}

CString CHseAgingApp::Gf_gmesGetPlanInfo()
{
	CString strBuff;

	pCimNet->GetFieldData(&strBuff, _T("PLAN_INFO"));
	return strBuff;
}


void CHseAgingApp::Gf_gmesShowLocalErrorMsg()
{

	CString strMsg;
	CMessageError errDlg;

	pCimNet->GetFieldData(&strMsg, _T("ERR_MSG_ENG"));	//ERR_MSG_ENG	ERR_MSG_LOC
	errDlg.m_strEMessage.Format(_T("<MES> MES ERROR %s (RACK = [%d], LAYER = [%d], CH = [%d]"), strMsg, lpInspWorkInfo->m_AgnInStartRack+1, lpInspWorkInfo->m_AgnInStartLayer+1, lpInspWorkInfo->m_AgnInStartChannel+1);

	errDlg.DoModal();
}

BOOL CHseAgingApp::Gf_gmesSendHost_PCHK(int hostCMD, CString PID)
{
	int nRtnCD;
	CString sLog, sdata = _T("");
	char Luc_PF = 0;

	if (m_pApp->m_bUserIdAdmin == TRUE)
	{
		return TRUE;
	}

Send_RETRY:

	if (hostCMD == HOST_EAYT)
	{
		nRtnCD = pCimNet->EAYT();
	}
	else if (hostCMD == HOST_PCHK)
	{
		Gf_gmesSetValueAgcm(1, 1, 1);
		nRtnCD = pCimNet->PCHK_B(PID);
		if (nRtnCD == RTN_OK)
		{
			pCimNet->GetFieldData(&lpInspWorkInfo->m_sMesPchkRtnPID[1][1][1], _T("RTN_PID"));
		}
	}

	sLog.Format(_T("<HOST_R> %s"), pCimNet->GetHostRecvMessage());
	Gf_writeMLog(sLog);

	if (nRtnCD == RTN_OK)
	{
		return TRUE;
	}
	else if (nRtnCD == RTN_MSG_NOT_SEND)
	{
		CMessageQuestion msg_dlg;

		msg_dlg.m_strQMessage.Format(_T("Failed to send message. Do you want retry ?"));
		msg_dlg.m_strLButton = _T(" Retry");

		if (msg_dlg.DoModal() == IDOK)
			goto Send_RETRY;
		else
			return FALSE;
	}
	else if (nRtnCD == RTN_RCV_TIMEOUT)
	{
		CMessageQuestion msg_dlg;

		msg_dlg.m_strQMessage.Format(_T("No response frome MES Host. Do you want retry ?"));
		msg_dlg.m_strLButton = _T(" Retry");

		if (msg_dlg.DoModal() == IDOK)
			goto Send_RETRY;
		else
			return FALSE;
	}
	else
	{
		/*while (Gf_gmesGetRTNCD() == _T("2705"))
		{
			Gf_gmesShowLocalErrorMsg();

			return FALSE;
		}
		Gf_gmesShowLocalErrorMsg();*/
	}

	return FALSE;
}

BOOL CHseAgingApp::Gf_gmesSendHost(int hostCMD, int rack, int layer, int ch)
{
	int nRtnCD;
	CString sLog, sdata = _T("");
	char Luc_PF = 0;

	if (m_pApp->m_bUserIdAdmin == TRUE)
	{
		return TRUE;
	}

Send_RETRY:

	if (hostCMD == HOST_EAYT)
	{
		nRtnCD = pCimNet->EAYT();
	}
	else if (hostCMD == HOST_UCHK)
	{
		nRtnCD = pCimNet->UCHK();
		if (nRtnCD == 0)
		{
			sdata = Gf_gmesGetUserID();
			m_pApp->m_sUserID = sdata;
			pCimNet->SetUserId(m_pApp->m_sUserID);

			sdata = Gf_gmesGetUserName();
			m_pApp->m_sUserName = sdata;
		}
	}
	else if (hostCMD == HOST_EDTI)
	{
		nRtnCD = pCimNet->EDTI();
	}
	else if (hostCMD == HOST_PCHK)
	{
		Gf_gmesSetValueAgcm(rack, layer, ch);
		nRtnCD = pCimNet->PCHK();
		if (nRtnCD == RTN_OK)
		{
			pCimNet->GetFieldData(&lpInspWorkInfo->m_sMesPchkRtnPID[rack][layer][ch], _T("RTN_PID"));
		}
	}
	else if (hostCMD == HOST_FLDR)
	{
	}
	else if (hostCMD == HOST_EICR)
	{
	}
	else if (hostCMD == HOST_AGN_IN)
	{
		/*if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_NONE)
		{
			Gf_gmesSetValueAgcm(rack, layer, ch);
			lpInspWorkInfo->m_AgnInStartRack = rack;
			lpInspWorkInfo->m_AgnInStartLayer = layer;
			lpInspWorkInfo->m_AgnInStartChannel = ch;
			nRtnCD = pCimNet->AGN_IN();
		}
		else
		{
			return TRUE;
		}*/
		Gf_gmesSetValueAgcm(rack, layer, ch);
		lpInspWorkInfo->m_AgnInStartRack = rack;
		lpInspWorkInfo->m_AgnInStartLayer = layer;
		lpInspWorkInfo->m_AgnInStartChannel = ch;
		nRtnCD = pCimNet->AGN_IN();
	}
	else if (hostCMD == HOST_AGN_OUT)
	{
		if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_NONE && lpInspWorkInfo->m_ast_AgingTempError[rack][layer][ch] != TRUE)
		{
			Gf_gmesSetValueAgcm(rack, layer, ch);
			lpInspWorkInfo->m_AgnInStartRack = rack;
			lpInspWorkInfo->m_AgnInStartLayer = layer;
			lpInspWorkInfo->m_AgnInStartChannel = ch;
			nRtnCD = pCimNet->AGN_OUT();
		}
		else
		{
			return FALSE;
		}
		/*Gf_gmesSetValueAgcm(rack, layer, ch);
		lpInspWorkInfo->m_AgnInStartRack = rack;
		lpInspWorkInfo->m_AgnInStartLayer = layer;
		lpInspWorkInfo->m_AgnInStartChannel = ch;
		nRtnCD = pCimNet->AGN_OUT();*/
		//return TRUE;
	}
	else if (hostCMD == HOST_APDR)
	{
		/*if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_NONE)
		{
		Lf_gmesSetValueAPDR(rack,layer, ch);
		lpInspWorkInfo->m_AgnInStartRack = rack;
		lpInspWorkInfo->m_AgnInStartLayer = layer;
		lpInspWorkInfo->m_AgnInStartChannel = ch;
		nRtnCD = pCimNet->APDR();
		}
		else
		{
			return TRUE;
		}*/
		Lf_gmesSetValueAPDR(rack, layer, ch);
		lpInspWorkInfo->m_AgnInStartRack = rack;
		lpInspWorkInfo->m_AgnInStartLayer = layer;
		lpInspWorkInfo->m_AgnInStartChannel = ch;
		nRtnCD = pCimNet->APDR();
	}
	else if (hostCMD == HOST_AGN_INSP)
	{
		Lf_setGmesValueAGN_INSP(rack, layer, ch);
		nRtnCD = pCimNet->AGN_INSP();
	}
	else if (hostCMD == HOST_DSPM)
	{
		Lf_setGmesValueDSPM(rack, layer, ch);
		nRtnCD = pCimNet->DSPM();
	}
	else if (hostCMD == HOST_DMIN)
	{
		Lf_setGmesValueDMIN(rack);
		nRtnCD = pCimNet->DMIN();
	}
	else if (hostCMD == HOST_DMOU)
	{
		Lf_setGmesValueDMOU(rack);
		nRtnCD = pCimNet->DMOU();
	}
	else if (hostCMD == HOST_ERCP)
	{

	}
	else if (hostCMD == HOST_UNDO)
	{
		Lf_setGmesValueUNDO(rack, layer, ch);
		nRtnCD = pCimNet->UNDO(rack, layer, ch);
	}
	else if (hostCMD == HOST_RMSO)
	{
		nRtnCD = pCimNet->RMSO();
	}

	sLog.Format(_T("<HOST_R> %s"), pCimNet->GetHostRecvMessage());
	Gf_writeMLog(sLog);

	if (nRtnCD == RTN_OK)
	{
		return TRUE;
	}
	else if (nRtnCD == RTN_MSG_NOT_SEND)
	{
		CMessageQuestion msg_dlg;

		msg_dlg.m_strQMessage.Format(_T("Failed to send message. Do you want retry ?"));
		msg_dlg.m_strLButton = _T(" Retry");

		if (msg_dlg.DoModal() == IDOK)
			goto Send_RETRY;
		else
			return FALSE;
	}
	else if (nRtnCD == RTN_RCV_TIMEOUT)
	{
		CMessageQuestion msg_dlg;

		msg_dlg.m_strQMessage.Format(_T("No response frome MES Host. Do you want retry ?"));
		msg_dlg.m_strLButton = _T(" Retry");

		if (msg_dlg.DoModal() == IDOK)
			goto Send_RETRY;
		else
			return FALSE;
	}
	else
	{
		while (Gf_gmesGetRTNCD() == _T("2705"))
		{
			Gf_gmesShowLocalErrorMsg();

			return FALSE;
		}
		Gf_gmesShowLocalErrorMsg();
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void CHseAgingApp::Gf_clearAgingStatusError()
{
	memset(lpInspWorkInfo->m_ast_AgingChErrorType, 0, sizeof(lpInspWorkInfo->m_ast_AgingChErrorType));
	memset(lpInspWorkInfo->m_ast_AgingChErrorValue, 0, sizeof(lpInspWorkInfo->m_ast_AgingChErrorValue));
}


BOOL CHseAgingApp::Gf_initTempRecorder()
{
	BOOL bRet;

	bRet = m_pTemp2xxx->TempSDR100_Initialize(lpSystemInfo->m_nTempRecorderPort);

	return bRet;
}


