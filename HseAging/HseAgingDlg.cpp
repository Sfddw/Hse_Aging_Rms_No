
// HseAgingDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "HseAging.h"
#include "HseAgingDlg.h"
#include "afxdialogex.h"
#include "UserID.h"
#include "Monitoring.h"
#include "ModelInfo.h"
#include "PidInput.h"
#include "System.h"
#include "MessageQuestion.h"
#include "AutoFirmware.h"
#include "CableOpen.h"
#include "Password.h"
#include "ErcpTest.h"
#include <exception>

#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "setupapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static BOOL bTempErrorOnce[MAX_RACK] = { FALSE };

UINT ThreadAgingStartRack(LPVOID pParam)
{
	CHseAgingDlg* pDlg = (CHseAgingDlg*)pParam;
	ULONGLONG agingTotalSec, agingElapseSec;
	LPINSPWORKINFO lpInspWorkInfo = m_pApp->GetInspWorkInfo();
	LPSYSTEMINFO lpSystemInfo = m_pApp->GetSystemInfo();

	CString sLog;
	DWORD preTick[MAX_RACK] = { 0, 0, 0, 0, 0, 0 };
	DWORD temTick[MAX_RACK] = { 0, 0, 0, 0, 0, 0 }; 
	BOOL debugDoorOpen = DOOR_CLOSE;
	float elapsedTimeToSubtract = 0;
	CPidInput idDlg; 
	DWORD condStartTick[MAX_RACK] = { 0 };

	while (1)
	{
		for (int rack = 0; rack < MAX_RACK; rack++)
		{
			if (pDlg->m_nAgingStart[rack] == FALSE)
				continue;

			// 2025-02-18 KDW. AGING START시 DELAY동안 RUNTIME값 수정
				if ((lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_IDLE || lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_ERROR) &&
					lpInspWorkInfo->m_nAgingInYN[rack] == FALSE)
				{
					BOOL bDoorStatus = lpInspWorkInfo->m_nDoorOpenClose[rack];
					BOOL bTempRange = OK;

					if (lpInspWorkInfo->m_nOpeDoorUse[rack] == FALSE)
						bDoorStatus = DOOR_CLOSE;

					if (lpInspWorkInfo->m_nOpeTemperatureUse[rack] == TRUE)
					{
						bTempRange = OK;
						if (lpInspWorkInfo->m_fTempReadVal[rack] < lpInspWorkInfo->m_nOpeTemperatureMin[rack] ||
							lpInspWorkInfo->m_fTempReadVal[rack] > lpInspWorkInfo->m_nOpeTemperatureMax[rack])
						{
							bTempRange = NG;
						}
					}

					if ((bTempRange == OK) && (bDoorStatus == DOOR_CLOSE))
					{
					CheckAgain:
						// 조건 처음 만족 → 타임스탬프 찍기
						if (condStartTick[rack] == 0)
							condStartTick[rack] = ::GetTickCount();

						// 아직 4초 안 지남 → 아래 계산부로 내려가지 말고 다음 루프로
						if ((::GetTickCount() - condStartTick[rack]) < 4000)
						{continue;
							//goto CheckAgain;
						}

						// 4초 연속 유지됨 → 여기서만 RUNNING 전이
						// (필요시 DoorUse=false 분기 포함)
						lpInspWorkInfo->m_nAgingOperatingMode[rack] = AGING_RUNNING;

						// (필요하다면 MES 관련 코드 복구)
						 idDlg.m_nMesAutoDMOU = MES_AGN_IN_AUTO;
						 lpInspWorkInfo->m_nAgnIn = TRUE;
						 AfxGetApp()->GetMainWnd()->SendMessage(WM_BCR_RACK_ID_INPUT, (WPARAM)rack, NULL);
						 lpInspWorkInfo->m_nAgingInYN[rack] = TRUE;

						m_pApp->Gf_sumSetStartTime(rack);
						lpInspWorkInfo->m_nAgingResumeOffsetSec[rack] = lpSystemInfo->m_sLastTimeOut[rack] * 60;
						lpInspWorkInfo->m_nAgingStartTick[rack] = ::GetTickCount64();

						sLog.Format(_T("DOOR&TEMP Check : OK"));
						pDlg->Lf_writeRackMLog(rack, sLog);

						condStartTick[rack] = 0;  // 전이 후 초기화
						// 전이 완료했으니 계속 진행 (아래 경과시간 계산으로 내려가도 OK)
					}
					else
					{
						// 조건이 깨지면 타이머 초기화
						condStartTick[rack] = 0;
						// 아직 IDLE 유지 → 아래 계산부 수행 이유 없음, 다음 루프로
						continue;
					}
				}

			agingTotalSec = lpInspWorkInfo->m_nAgingSetTime[rack] * 60;
			agingElapseSec = ((::GetTickCount64() - lpInspWorkInfo->m_nAgingStartTick[rack]) / 1000) + lpInspWorkInfo->m_nAgingResumeOffsetSec[rack];

			/*if (agingElapseSec >= 20)
			{
				lpInspWorkInfo->m_ast_AgingChErrorResult[3][0][8] = LIMIT_HIGH;
			}*/

			// Door Open 시간만큼 경과시간을 마이너스 시킨다.
			//agingElapseSec = agingElapseSec - (lpInspWorkInfo->m_nAgingDoorOpenTime[rack] / 1000);

			if (lpInspWorkInfo->m_nOpeDoorUse[rack] == TRUE && lpInspWorkInfo->m_nOpeTemperatureUse[rack] == TRUE)
			{
				// 둘 다 해당될 경우, 더 긴 시간 적용
				elapsedTimeToSubtract = max(lpInspWorkInfo->m_nAgingDoorOpenTime[rack] / 1000, lpInspWorkInfo->m_nAgingTempMatchTime[rack] / 1000);
			}
			else if (lpInspWorkInfo->m_nOpeDoorUse[rack] == TRUE)
			{
				// Door Open만 해당될 경우
				elapsedTimeToSubtract = lpInspWorkInfo->m_nAgingDoorOpenTime[rack] / 1000;
			}
			else if (lpInspWorkInfo->m_nOpeTemperatureUse[rack] == TRUE)
			{
				// 온도 불일치만 해당될 경우
				elapsedTimeToSubtract = lpInspWorkInfo->m_nAgingTempMatchTime[rack] / 1000;
			}

			agingElapseSec -=  elapsedTimeToSubtract;

			// Aging 경과 시간에 DOOR Open 시간을 마이너스 하여 업데이트 한다.
			
			lpInspWorkInfo->m_nAgingRunTime[rack] = (int)(agingElapseSec / 60);
			sLog.Format(_T("LAST_TIMEOUT_RACK%d"), rack + 1);
			Write_SysIniFile(_T("SYSTEM"), sLog, lpInspWorkInfo->m_nAgingRunTime[rack]);

			


			// Aging 완료되면 Start Flag Clear
			if (lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_RUNNING || lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_COMPLETE_DOORCLOSE)
			{
				BOOL bDoorStatus;
				bDoorStatus = lpInspWorkInfo->m_nDoorOpenClose[rack];

#if  (DEBUG_DOOR_OPEN_TEST == 1)
				bDoorStatus = debugDoorOpen;
#endif

				// DOOR OPEN 상태 체크되면 AGING COMPLETE 상태로 변경하고 AGING을 STOP 한다.
				if (agingElapseSec > agingTotalSec)
				{
					// Door 기능 사용하지 않을 경우 Door Close 체크하지 않는다.
					//lpInspWorkInfo->m_nAgingOperatingMode[rack] = AGING_COMPLETE_DOORCLOSE;

					lpInspWorkInfo->m_nLampColor = 1;
					lpInspWorkInfo->m_nAgingStatusS[rack] = 1;
					//m_pApp->pCommand->Gf_dio_setDIOWriteOutput(9, 1);
					

					if (lpInspWorkInfo->m_nOpeDoorUse[rack] == FALSE)
						bDoorStatus = DOOR_OPEN;

					if (bDoorStatus == DOOR_OPEN)
					{
						preTick[rack] = 0;
						temTick[rack] = 0;
						pDlg->m_nAgingStart[rack] = FALSE;
						lpInspWorkInfo->m_nAgingOperatingMode[rack] = AGING_COMPLETE;
						lpInspWorkInfo->m_nAgingStatusS[rack] = 0;
						lpInspWorkInfo->m_nLampColor = 0;
						pDlg->Lf_updateTowerLamp();
						lpInspWorkInfo->m_nAgingInYN[rack] = FALSE;

						// RACK Log 출력
						sLog.Format(_T("DOOR Open Check : OK"));
						pDlg->Lf_writeRackMLog(rack, sLog);
						break;
					}
				}
				else
				{
					if (lpInspWorkInfo->m_nOpeDoorUse[rack] == TRUE)
					{
						// 2025-02-25 PDH. AGING 시간 완료되지 않았는데 DOOR Open 상태이면 시간 업데이트하지 않도록 한다.
						if (bDoorStatus == DOOR_OPEN)
						{
							if (preTick[rack] == 0)
							{
								preTick[rack] = ::GetTickCount();

								sLog.Format(_T("DOOR Open : Aging Hold"));
								pDlg->Lf_writeRackMLog(rack, sLog);

								m_pApp->pCommand->Gf_dio_setDIOWriteOutput(9, 1);
							}
							else
							{
								DWORD tickCnt;
								tickCnt = ::GetTickCount();
								lpInspWorkInfo->m_nAgingDoorOpenTime[rack] += (tickCnt - preTick[rack]);

								preTick[rack] = ::GetTickCount();
							}
						}
						else
						{
							if (preTick[rack] != 0)
							{
								sLog.Format(_T("DOOR Close : Aging Continue"));
								pDlg->Lf_writeRackMLog(rack, sLog);

								preTick[rack] = 0;
							}
						}
					}

					if (lpInspWorkInfo->m_nOpeTemperatureUse[rack] == TRUE)
					{
						static DWORD tempStartTick[MAX_RACK] = { 0 };   // NG 연속 체크용
						DWORD now = ::GetTickCount();

						BOOL isNG =
							(lpInspWorkInfo->m_fTempReadVal[rack] < lpInspWorkInfo->m_nOpeTemperatureMin[rack]) ||
							(lpInspWorkInfo->m_fTempReadVal[rack] > lpInspWorkInfo->m_nOpeTemperatureMax[rack]);

						// AGING 시간 동안 온도가 모델 값에 안맞을 경우 AGING 시간 멈춤
						/*if (lpInspWorkInfo->m_fTempReadVal[rack] < lpInspWorkInfo->m_nOpeTemperatureMin[rack] +2 || lpInspWorkInfo->m_fTempReadVal[rack] > lpInspWorkInfo->m_nOpeTemperatureMax[rack] - 2)*/
						if (isNG)
						{

							if (tempStartTick[rack] == 0)
								tempStartTick[rack] = now;

							// 3초 미만 NG → 아래 코드 실행 금지 → continue로 올라감
							if ((now - tempStartTick[rack]) < 3000)
								continue;
						}
						else
						{
							tempStartTick[rack] = 0;
						}
						if (isNG)
						{
							if (temTick[rack] == 0)
							{
								temTick[rack] = ::GetTickCount();

								sLog.Format(_T("Temp Not Matching"));
								pDlg->Lf_writeRackMLog(rack, sLog);

								m_pApp->pCommand->Gf_dio_setDIOWriteOutput(0x09, 0x01);

								if (bTempErrorOnce[rack] == FALSE)
								{
									bTempErrorOnce[rack] = TRUE;
										for (int layer = 0; layer < MAX_LAYER; layer++)
										{
											// Channel 상태 Update. /Normal/Error
											for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
											{
												lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] = TEMP_LOW;
												lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][ch] = ERR_INFO_TEMP;
											}
										}
								}
							}
							else
							{
								DWORD tickCnt;
								tickCnt = ::GetTickCount();
								lpInspWorkInfo->m_nAgingTempMatchTime[rack] += (tickCnt - temTick[rack]);

								temTick[rack] = ::GetTickCount();
							}
						}
						else
						{
							if (temTick[rack] != 0)
							{
								sLog.Format(_T("Temp Match Up : Aging Continue"));
								pDlg->Lf_writeRackMLog(rack, sLog);

								temTick[rack] = 0;
							}
						}
					}
				}
			}
		}

		Sleep(10);
	}

	return (0);
}

UINT ThreadHandBcrSearch(LPVOID pParam)
{
	CString sLog;
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA stDevInfoData = SP_DEVINFO_DATA();
	LPINSPWORKINFO lpInspWorkInfo;
	lpInspWorkInfo = m_pApp->GetInspWorkInfo();

	hDevInfo = SetupDiGetClassDevs(0L, 0L, 0L, DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_PROFILE);

	if (hDevInfo == INVALID_HANDLE_VALUE)
		return FALSE;

	stDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &stDevInfoData); i++)
	{
		TCHAR szInstanceId[MAX_PATH] = { 0, };
		TCHAR szClassName[MAX_PATH] = { 0, };
		TCHAR szFriendlyName[MAX_PATH] = { 0, };
		TCHAR szClassDescription[MAX_PATH] = { 0, };
		TCHAR szDeviceDescription[MAX_PATH] = { 0, };

		// Get Device Instance ID
		BOOL bResult = SetupDiGetDeviceInstanceId(hDevInfo, &stDevInfoData, szInstanceId, _countof(szInstanceId), 0);
		if (!bResult)
		{
			AfxMessageBox(_T("Failed to get device instance ID"));
			continue;
		}

		CString instanceID;
		instanceID.Format(_T("%s"), szInstanceId);
		if (
			(instanceID.Find(_T("VID_0C2E&PID_1221")) != -1)
			|| (instanceID.Find(_T("VID_0C2E&PID_1181")) != -1)
			)
		{
			lpInspWorkInfo->m_nConnectInfo[CONNECT_BARCODE] = TRUE;
			return (0);
		}

		// 		(VOID)SetupDiGetDeviceRegistryProperty(
		// 			hDevInfo,
		// 			&stDevInfoData,
		// 			SPDRP_CLASS,
		// 			0,
		// 			(PBYTE)szClassName,
		// 			_countof(szClassName),
		// 			0
		// 		);
		// 
		// 		(VOID)SetupDiGetDeviceRegistryProperty(
		// 			hDevInfo,
		// 			&stDevInfoData,
		// 			SPDRP_DEVICEDESC,
		// 			0,
		// 			(PBYTE)szDeviceDescription,
		// 			_countof(szDeviceDescription),
		// 			0
		// 		);
		// 
		// 		(VOID)SetupDiGetDeviceRegistryProperty(
		// 			hDevInfo,
		// 			&stDevInfoData,
		// 			SPDRP_FRIENDLYNAME,
		// 			0,
		// 			(PBYTE)szFriendlyName,
		// 			_countof(szFriendlyName),
		// 			0
		// 		);
		// 
		// 		(VOID)SetupDiGetClassDescription(
		// 			&stDevInfoData.ClassGuid,
		// 			szClassDescription,
		// 			_countof(szClassDescription),
		// 			0
		// 		);
		// 
		// 		sLog.Format(_T("[%d]"), i);
		// 		m_pApp->Gf_writeMLog(sLog);
		// 
		// 		sLog.Format(_T("-- Class: %s"), szClassName);
		// 		m_pApp->Gf_writeMLog(sLog);
		// 
		// 		sLog.Format(_T("-- Friendly Name: %s"), szFriendlyName);
		// 		m_pApp->Gf_writeMLog(sLog);
		// 
		// 		sLog.Format(_T("-- Instance ID: %s"), szInstanceId);
		// 		m_pApp->Gf_writeMLog(sLog);
		// 
		// 		sLog.Format(_T("-- Class Description: %s"), szClassDescription);
		// 		m_pApp->Gf_writeMLog(sLog);
		// 
		// 		sLog.Format(_T("-- Device Description: %s"), szDeviceDescription);
		// 		m_pApp->Gf_writeMLog(sLog);
		Sleep(10);
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	lpInspWorkInfo->m_nConnectInfo[CONNECT_BARCODE] = FALSE;


	return (0);
}
UINT ThreadFwVersionRead(LPVOID pParam)
{
	// FW Read 함수를 Timer 에서 동작 시 S/W 멈춤현상이 있다. Thread 에서 동작하도록 이동
	m_pApp->pCommand->Gf_getMainBoardFwVersionAll();

	return (0);
}

UINT ThreadTempST590_1(LPVOID pParam)
{
	/*m_pApp->m_pTemp2xxx->TempSDR100_readTemp();
	Sleep(300);*/
	m_pApp->m_pTemp2xxx->TempST590_readTemp2();
	Sleep(300);
	m_pApp->m_pTemp2xxx->TempST590_readTemp3();
	Sleep(300);
	m_pApp->m_pTemp2xxx->TempST590_readTemp4();
	Sleep(300);
	/*m_pApp->m_pTemp2xxx->TempSDR100_readTemp();
	Sleep(100);*/
	m_pApp->m_pTemp2xxx->TempST590_SET_readTemp2();
	Sleep(300);
	m_pApp->m_pTemp2xxx->TempST590_SET_readTemp3();
	Sleep(300);
	m_pApp->m_pTemp2xxx->TempST590_SET_readTemp4();
	Sleep(300);
	return (0);
}

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CHseAgingDlg 대화 상자



CHseAgingDlg::CHseAgingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HSEAGING_DIALOG, pParent)
	, m_dlgErcpTest(this)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);


	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		for (int ly = 0; ly < MAX_LAYER; ly++)
		{
			for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
			{
				m_pSttRackState[rack][ly][ch] = NULL;
			}
		}
	}

	m_pDefaultFont = new CFont();
	m_pDefaultFont->CreateFont(15, 6, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
}

void CHseAgingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_MA_USER, m_btnIconUser);
	DDX_Control(pDX, IDC_BTN_MA_MONITORING, m_btnIconMonitoring);
	DDX_Control(pDX, IDC_BTN_MA_MODEL, m_btnIconModel);
	DDX_Control(pDX, IDC_BTN_MA_PID_INPUT, m_btnIconPIDInput);
	DDX_Control(pDX, IDC_BTN_MA_FIRMWARE, m_btnIconFirmware);
	DDX_Control(pDX, IDC_BTN_MA_SYSTEM, m_btnIconSystem);
	DDX_Control(pDX, IDC_BTN_MA_EXIT, m_btnIconExit);
	DDX_Control(pDX, IDC_MBT_MA_BUZZ_OFF, m_mbtBuzzOff);
	DDX_Control(pDX, IDC_STT_MA_MLOG_RACK1, m_sttMaMLogRack1);
	DDX_Control(pDX, IDC_STT_MA_MLOG_RACK2, m_sttMaMLogRack2);
	DDX_Control(pDX, IDC_STT_MA_MLOG_RACK3, m_sttMaMLogRack3);
	DDX_Control(pDX, IDC_STT_MA_MLOG_RACK4, m_sttMaMLogRack4);
	DDX_Control(pDX, IDC_STT_MA_MLOG_RACK5, m_sttMaMLogRack5);
	DDX_Control(pDX, IDC_STT_MA_MLOG_RACK6, m_sttMaMLogRack6);
	DDX_Control(pDX, IDC_LST_MA_MLOG_RACK1, m_lstMaMLogRack1);
	DDX_Control(pDX, IDC_LST_MA_MLOG_RACK2, m_lstMaMLogRack2);
	DDX_Control(pDX, IDC_LST_MA_MLOG_RACK3, m_lstMaMLogRack3);
	DDX_Control(pDX, IDC_LST_MA_MLOG_RACK4, m_lstMaMLogRack4);
	DDX_Control(pDX, IDC_LST_MA_MLOG_RACK5, m_lstMaMLogRack5);
	DDX_Control(pDX, IDC_LST_MA_MLOG_RACK6, m_lstMaMLogRack6);
	DDX_Control(pDX, IDC_CMB_MA_MODEL_RACK1, m_cmbMaModelRack1);
	DDX_Control(pDX, IDC_CMB_MA_MODEL_RACK2, m_cmbMaModelRack2);
	DDX_Control(pDX, IDC_CMB_MA_MODEL_RACK3, m_cmbMaModelRack3);
	DDX_Control(pDX, IDC_CMB_MA_MODEL_RACK4, m_cmbMaModelRack4);
	DDX_Control(pDX, IDC_CMB_MA_MODEL_RACK5, m_cmbMaModelRack5);
	DDX_Control(pDX, IDC_CMB_MA_MODEL_RACK6, m_cmbMaModelRack6);
	DDX_Control(pDX, IDC_CTR_MA_PROGRESS_RACK1, m_ctrMaProgressRack1);
	DDX_Control(pDX, IDC_CTR_MA_PROGRESS_RACK2, m_ctrMaProgressRack2);
	DDX_Control(pDX, IDC_CTR_MA_PROGRESS_RACK3, m_ctrMaProgressRack3);
	DDX_Control(pDX, IDC_CTR_MA_PROGRESS_RACK4, m_ctrMaProgressRack4);
	DDX_Control(pDX, IDC_CTR_MA_PROGRESS_RACK5, m_ctrMaProgressRack5);
	DDX_Control(pDX, IDC_CTR_MA_PROGRESS_RACK6, m_ctrMaProgressRack6);
	DDX_Control(pDX, IDC_STT_TEMP_SENSOR, m_sttTempSensor);
	DDX_Control(pDX, IDC_STT_TEMP_SENSOR1_T, m_sttTempSensor1T);
	DDX_Control(pDX, IDC_STT_TEMP_SENSOR2_T, m_sttTempSensor2T);
	DDX_Control(pDX, IDC_STT_TEMP_SENSOR3_T, m_sttTempSensor3T);
	DDX_Control(pDX, IDC_STT_TEMP_SENSOR4_T, m_sttTempSensor4T);
	DDX_Control(pDX, IDC_STT_TEMP_SENSOR5_T, m_sttTempSensor5T);
	DDX_Control(pDX, IDC_STT_TEMP_SENSOR6_T, m_sttTempSensor6T);
	DDX_Control(pDX, IDC_STT_CONNECT_INFO, m_sttConnectInfo);
}

BEGIN_MESSAGE_MAP(CHseAgingDlg, CDialogEx)
	ON_MESSAGE(WM_ETH_UDP_RECEIVE, OnUdpReceive)
	ON_MESSAGE(WM_ETH_UDP_RECEIVE_DIO, OnUdpReceiveDio)
	ON_MESSAGE(WM_RS232_RECEIVED2, OnRs232Receive)
	ON_MESSAGE(WM_UPDATE_SYSTEM_INFO, OnUpdateSystemInfo)
	ON_MESSAGE(WM_BCR_RACK_ID_INPUT, OnBcrRackIDInput)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_MA_USER, &CHseAgingDlg::OnBnClickedBtnMaUser)
	ON_BN_CLICKED(IDC_BTN_MA_MONITORING, &CHseAgingDlg::OnBnClickedBtnMaMonitoring)
	ON_BN_CLICKED(IDC_BTN_MA_MODEL, &CHseAgingDlg::OnBnClickedBtnMaModel)
	ON_BN_CLICKED(IDC_BTN_MA_SYSTEM, &CHseAgingDlg::OnBnClickedBtnMaSystem)
	ON_BN_CLICKED(IDC_BTN_MA_PID_INPUT, &CHseAgingDlg::OnBnClickedBtnMaPidInput)
	ON_BN_CLICKED(IDC_BTN_MA_FIRMWARE, &CHseAgingDlg::OnBnClickedBtnMaFirmware)
	ON_BN_CLICKED(IDC_BTN_MA_EXIT, &CHseAgingDlg::OnBnClickedBtnMaExit)
	ON_BN_CLICKED(IDC_MBC_MA_FUSING_RACK1, &CHseAgingDlg::OnBnClickedMbcMaFusingRack1)
	ON_BN_CLICKED(IDC_MBC_MA_FUSING_RACK2, &CHseAgingDlg::OnBnClickedMbcMaFusingRack2)
	ON_BN_CLICKED(IDC_MBC_MA_FUSING_RACK3, &CHseAgingDlg::OnBnClickedMbcMaFusingRack3)
	ON_BN_CLICKED(IDC_MBC_MA_FUSING_RACK4, &CHseAgingDlg::OnBnClickedMbcMaFusingRack4)
	ON_BN_CLICKED(IDC_MBC_MA_FUSING_RACK5, &CHseAgingDlg::OnBnClickedMbcMaFusingRack5)
	ON_BN_CLICKED(IDC_MBC_MA_FUSING_RACK6, &CHseAgingDlg::OnBnClickedMbcMaFusingRack6)
	ON_BN_CLICKED(IDC_MBC_MA_START_RACK1, &CHseAgingDlg::OnBnClickedMbcMaStartRack1)
	ON_BN_CLICKED(IDC_MBC_MA_START_RACK2, &CHseAgingDlg::OnBnClickedMbcMaStartRack2)
	ON_BN_CLICKED(IDC_MBC_MA_START_RACK3, &CHseAgingDlg::OnBnClickedMbcMaStartRack3)
	ON_BN_CLICKED(IDC_MBC_MA_START_RACK4, &CHseAgingDlg::OnBnClickedMbcMaStartRack4)
	ON_BN_CLICKED(IDC_MBC_MA_START_RACK5, &CHseAgingDlg::OnBnClickedMbcMaStartRack5)
	ON_BN_CLICKED(IDC_MBC_MA_START_RACK6, &CHseAgingDlg::OnBnClickedMbcMaStartRack6)
	ON_BN_CLICKED(IDC_MBC_MA_STOP_RACK1, &CHseAgingDlg::OnBnClickedMbcMaStopRack1)
	ON_BN_CLICKED(IDC_MBC_MA_STOP_RACK2, &CHseAgingDlg::OnBnClickedMbcMaStopRack2)
	ON_BN_CLICKED(IDC_MBC_MA_STOP_RACK3, &CHseAgingDlg::OnBnClickedMbcMaStopRack3)
	ON_BN_CLICKED(IDC_MBC_MA_STOP_RACK4, &CHseAgingDlg::OnBnClickedMbcMaStopRack4)
	ON_BN_CLICKED(IDC_MBC_MA_STOP_RACK5, &CHseAgingDlg::OnBnClickedMbcMaStopRack5)
	ON_BN_CLICKED(IDC_MBC_MA_STOP_RACK6, &CHseAgingDlg::OnBnClickedMbcMaStopRack6)
	ON_BN_CLICKED(IDC_MBC_MA_CH_SET_RACK1, &CHseAgingDlg::OnBnClickedMbcMaChSetRack1)
	ON_BN_CLICKED(IDC_MBC_MA_CH_SET_RACK2, &CHseAgingDlg::OnBnClickedMbcMaChSetRack2)
	ON_BN_CLICKED(IDC_MBC_MA_CH_SET_RACK3, &CHseAgingDlg::OnBnClickedMbcMaChSetRack3)
	ON_BN_CLICKED(IDC_MBC_MA_CH_SET_RACK4, &CHseAgingDlg::OnBnClickedMbcMaChSetRack4)
	ON_BN_CLICKED(IDC_MBC_MA_CH_SET_RACK5, &CHseAgingDlg::OnBnClickedMbcMaChSetRack5)
	ON_BN_CLICKED(IDC_MBC_MA_CH_SET_RACK6, &CHseAgingDlg::OnBnClickedMbcMaChSetRack6)
	ON_BN_CLICKED(IDC_CHK_MA_SELECT_RACK1, &CHseAgingDlg::OnBnClickedChkMaSelectRack1)
	ON_BN_CLICKED(IDC_CHK_MA_SELECT_RACK2, &CHseAgingDlg::OnBnClickedChkMaSelectRack2)
	ON_BN_CLICKED(IDC_CHK_MA_SELECT_RACK3, &CHseAgingDlg::OnBnClickedChkMaSelectRack3)
	ON_BN_CLICKED(IDC_CHK_MA_SELECT_RACK4, &CHseAgingDlg::OnBnClickedChkMaSelectRack4)
	ON_BN_CLICKED(IDC_CHK_MA_SELECT_RACK5, &CHseAgingDlg::OnBnClickedChkMaSelectRack5)
	ON_BN_CLICKED(IDC_CHK_MA_SELECT_RACK6, &CHseAgingDlg::OnBnClickedChkMaSelectRack6)
	ON_COMMAND_RANGE(IDC_STT_RACK1L1_CH1, IDC_STT_RACK6L5_CH16, &CHseAgingDlg::OnStnClickedChEnableDisable)
	ON_BN_CLICKED(IDC_MBT_MA_BUZZ_OFF, &CHseAgingDlg::OnBnClickedMbtMaBuzzOff)
	ON_BN_CLICKED(IDC_BUTTON_DOOR1, &CHseAgingDlg::OnBnClickedButtonDoor1)
	ON_BN_CLICKED(IDC_BUTTON_DOOR2, &CHseAgingDlg::OnBnClickedButtonDoor2)
	ON_BN_CLICKED(IDC_BUTTON_DOOR3, &CHseAgingDlg::OnBnClickedButtonDoor3)
	ON_BN_CLICKED(IDC_BUTTON_DOOR4, &CHseAgingDlg::OnBnClickedButtonDoor4)
	ON_BN_CLICKED(IDC_BUTTON_DOOR5, &CHseAgingDlg::OnBnClickedButtonDoor5)
	ON_BN_CLICKED(IDC_BUTTON_DOOR6, &CHseAgingDlg::OnBnClickedButtonDoor6)
	ON_BN_CLICKED(IDC_PAUSE1, &CHseAgingDlg::OnBnClickedPause1)
END_MESSAGE_MAP()


// CHseAgingDlg 메시지 처리기
LRESULT CHseAgingDlg::OnUdpReceive(WPARAM wParam, LPARAM lParam)
{
	CString strPacket;

#if (DEBUG_COMM_LOG==1)
	CString sLog;
	sLog.Format(_T("<UDP Recv> %S"), (char*)wParam);
	m_pApp->Gf_writeMLog(sLog);
#endif

	strPacket.Format(_T("%S"), (char*)wParam);
	m_pApp->udp_processPacket(strPacket);

	return 0;
}

LRESULT CHseAgingDlg::OnUdpReceiveDio(WPARAM wParam, LPARAM lParam)
{
	CString strPacket;

	strPacket.Format(_T("%S"), (char*)wParam);
	m_pApp->udp_processDioPacket(strPacket);

	return 0;
}

LRESULT CHseAgingDlg::OnRs232Receive(WPARAM wParam, LPARAM lParam)
{
#if (DEBUG_COMM_LOG==1)
	///////////////////////////////////////////////
	CString sLog;
	sLog.Format(_T("<RS232> RECV : %S"), (char*)wParam);
	m_pApp->Gf_writeMLog(sLog);
	///////////////////////////////////////////////
#endif

	Lf_parseSDR100Packet((char*)wParam);

	return (0);
}

LRESULT CHseAgingDlg::OnUpdateSystemInfo(WPARAM wParam, LPARAM lParam)
{
	Lf_updateSystemInfo();

	return (0);
}

LRESULT CHseAgingDlg::OnBcrRackIDInput(WPARAM wParam, LPARAM lParam)
{
	KillTimer(1);

	CPidInput id_dlg;
	id_dlg.m_nTargetRack = (int)wParam;
	id_dlg.m_nMesAutoDMOU = MES_DMOU_MODE_MANUAL;
	id_dlg.DoModal();

	SetTimer(1, 100, NULL);

	return (0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CHseAgingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	lpSystemInfo = m_pApp->GetSystemInfo();
	lpModelInfo = m_pApp->GetModelInfo();
	lpInspWorkInfo = m_pApp->GetInspWorkInfo();

	//GetDlgItem(IDC_STT_MA_SW_VER)->SetWindowText(lpSystemInfo->m_SwVersion);
	GetDlgItem(IDC_STT_MA_SW_VER)->SetWindowText(_T("HseAging_v1.2.8B"));

	for (int i = 0; i < MAX_RACK; ++i)
	{
		m_bFwMismatchNotified[i] = FALSE;
		lpInspWorkInfo->m_nAgingStatusS[i] = 0;
	}

	if (m_pApp->m_sUserID == "WD")
	{
		CHseAgingDlg* pDlg = (CHseAgingDlg*)AfxGetMainWnd();
		if (pDlg)
		{
			CWnd* pBtnDoor3 = pDlg->GetDlgItem(IDC_BUTTON_DOOR3);
			if (pBtnDoor3)
				pBtnDoor3->ShowWindow(SW_SHOW);  // 버튼 표시
		}
	}
	
	pCimNet = new CCimNetCommApi;

	lpInspWorkInfo->m_nLampColor = 0;

	// Dialog의 기본 FONT 설정.
	SendMessageToDescendants(WM_SETFONT, (WPARAM)m_pDefaultFont->GetSafeHandle(), 1, TRUE, FALSE);

	Lf_InitLocalValue();
	Lf_InitFontset();
	Lf_InitColorBrush();
	Lf_InitButtonIcon();
	Lf_InitDialogDesign();
	Lf_InitCobmoRackModelList();

	// DIO Board 초기화 명령을 전달한다
	Lf_setDIOBoardInitial();

	// Aging Thread 를 시작한다.
	AfxBeginThread(ThreadAgingStartRack, this);

	ShowWindow(SW_MAXIMIZE);

	SetTimer(1, 100, NULL);
	SetTimer(2, 1000, NULL);
	SetTimer(3, 3000, NULL);
	SetTimer(8, 1000, NULL);

	InitRecipeFolderAndFiles(TRUE, TRUE); // Recipe 폴더 생성

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CHseAgingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CHseAgingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트
		CRect rect, rectOri;
		GetClientRect(&rect);
		rectOri = rect;

		rect.bottom = 70;
		dc.FillSolidRect(rect, RGB(48,55,63));

		rect.top = rect.bottom;
		rect.bottom = rectOri.bottom;
		dc.FillSolidRect(rect, COLOR_USER_BACKGROUND);

		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CHseAgingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



HBRUSH CHseAgingDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  여기서 DC의 특성을 변경합니다.
	switch (nCtlColor)
	{
		case CTLCOLOR_MSGBOX:
			break;
		case CTLCOLOR_EDIT:
			break;
		case CTLCOLOR_LISTBOX:
			if ((pWnd->GetDlgCtrlID() == IDC_LST_MA_MLOG_RACK1)
				|| (pWnd->GetDlgCtrlID() == IDC_LST_MA_MLOG_RACK2)
				|| (pWnd->GetDlgCtrlID() == IDC_LST_MA_MLOG_RACK3)
				|| (pWnd->GetDlgCtrlID() == IDC_LST_MA_MLOG_RACK4)
				|| (pWnd->GetDlgCtrlID() == IDC_LST_MA_MLOG_RACK5)
				|| (pWnd->GetDlgCtrlID() == IDC_LST_MA_MLOG_RACK6)
				)
			{
				pDC->SetBkColor(COLOR_BLACK);
				pDC->SetTextColor(COLOR_WHITE);
				return m_Brush[COLOR_IDX_BLACK];
			}
			break;
		case CTLCOLOR_SCROLLBAR:

			break;
		case CTLCOLOR_BTN:
			break;
		case CTLCOLOR_STATIC:		// Static, CheckBox control
			if ((pWnd->GetDlgCtrlID() == IDC_STATIC)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_CHAMBER_NO)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MES_USER_ID)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MES_USER_NAME)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK1_LAYER1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK1_LAYER2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK1_LAYER3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK1_LAYER4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK1_LAYER5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK2_LAYER1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK2_LAYER2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK2_LAYER3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK2_LAYER4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK2_LAYER5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK3_LAYER1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK3_LAYER2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK3_LAYER3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK3_LAYER4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK3_LAYER5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK4_LAYER1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK4_LAYER2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK4_LAYER3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK4_LAYER4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK4_LAYER5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK5_LAYER1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK5_LAYER2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK5_LAYER3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK5_LAYER4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK5_LAYER5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK6_LAYER1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK6_LAYER2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK6_LAYER3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK6_LAYER4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RACK6_LAYER5)
				)
			{
				pDC->SetBkColor(COLOR_SKYBLUE);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_SKYBLUE];
			}
			else if ((pWnd->GetDlgCtrlID() == IDC_STATIC)
				|| (pWnd->GetDlgCtrlID() == IDC_STATIC_NG_COUNT)
				|| (pWnd->GetDlgCtrlID() == IDC_STATIC_NG_COUNT2))
			{
				pDC->SetBkColor(COLOR_SKYBLUE);
				pDC->SetTextColor(COLOR_RED);
				return m_Brush[COLOR_IDX_SKYBLUE];
			}

			if ((pWnd->GetDlgCtrlID() == IDC_GRP_MA_CHAMBER_NO)
				|| (pWnd->GetDlgCtrlID() == IDC_GRP_MA_DESCRIPTION)
				|| (pWnd->GetDlgCtrlID() == IDC_GRP_MA_FW_VERSION)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_MA_SELECT_RACK1)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_MA_SELECT_RACK2)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_MA_SELECT_RACK3)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_MA_SELECT_RACK4)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_MA_SELECT_RACK5)
				|| (pWnd->GetDlgCtrlID() == IDC_CHK_MA_SELECT_RACK6)
				)
			{
				pDC->SetBkColor(COLOR_USER_BACKGROUND);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_USER_BACKGROUND];
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DOOR1)
			{
				if (lpInspWorkInfo->m_nDoorOpenClose[DOOR_1] == DOOR_OPEN)
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
				else
				{
					pDC->SetBkColor(COLOR_BLUE128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_BLUE128];
				}
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DOOR2)
			{
				if (lpInspWorkInfo->m_nDoorOpenClose[DOOR_2] == DOOR_OPEN)
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
				else
				{
					pDC->SetBkColor(COLOR_BLUE128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_BLUE128];
				}
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DOOR3)
			{
				if (lpInspWorkInfo->m_nDoorOpenClose[DOOR_3] == DOOR_OPEN)
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
				else
				{
					pDC->SetBkColor(COLOR_BLUE128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_BLUE128];
				}
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DOOR4)
			{
				if (lpInspWorkInfo->m_nDoorOpenClose[DOOR_4] == DOOR_OPEN)
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
				else
				{
					pDC->SetBkColor(COLOR_BLUE128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_BLUE128];
				}
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DOOR5)
			{
				if (lpInspWorkInfo->m_nDoorOpenClose[DOOR_5] == DOOR_OPEN)
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
				else
				{
					pDC->SetBkColor(COLOR_BLUE128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_BLUE128];
				}
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DOOR6)
			{
				if (lpInspWorkInfo->m_nDoorOpenClose[DOOR_6] == DOOR_OPEN)
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
				else
				{
					pDC->SetBkColor(COLOR_BLUE128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_BLUE128];
				}
			}

			if ((pWnd->GetDlgCtrlID() == IDC_STT_MA_SET_TIME_RACK1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_SET_TIME_RACK2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_SET_TIME_RACK3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_SET_TIME_RACK4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_SET_TIME_RACK5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_SET_TIME_RACK6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RUN_TIME_RACK1)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RUN_TIME_RACK2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RUN_TIME_RACK3)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RUN_TIME_RACK4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RUN_TIME_RACK5)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_MA_RUN_TIME_RACK6)
				)
			{
				pDC->SetBkColor(COLOR_BLACK);
				pDC->SetTextColor(COLOR_GREEN);
				return m_Brush[COLOR_IDX_BLACK];
			}

			if ((pWnd->GetDlgCtrlID() == IDC_STT_TEMP_SENSOR1_V)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_TEMP_SENSOR2_V)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_TEMP_SENSOR3_V)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_TEMP_SENSOR4_V)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_TEMP_SENSOR5_V)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_TEMP_SENSOR6_V)
				)
			{
				pDC->SetBkColor(COLOR_GREEN128);
				pDC->SetTextColor(COLOR_GRAY224);
				return m_Brush[COLOR_IDX_GREEN128];
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_CONNECT_INFO_PG)
			{
				if (lpInspWorkInfo->m_nConnectInfo[CONNECT_PG] == TRUE)
				{
					pDC->SetBkColor(COLOR_GREEN128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_GREEN128];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_CONNECT_INFO_DIO)
			{
				if (lpInspWorkInfo->m_nConnectInfo[CONNECT_DIO] != 0)
				{
					pDC->SetBkColor(COLOR_GREEN128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_GREEN128];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_CONNECT_INFO_TEMP)
			{
				if (lpInspWorkInfo->m_nConnectInfo[CONNECT_TEMP] != 0)
				{
					pDC->SetBkColor(COLOR_GREEN128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_GREEN128];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_CONNECT_INFO_BARCODE)
			{
				if (lpInspWorkInfo->m_nConnectInfo[CONNECT_BARCODE] == TRUE)
				{
					pDC->SetBkColor(COLOR_GREEN128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_GREEN128];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_CONNECT_INFO_MES)
			{
				if (lpInspWorkInfo->m_nConnectInfo[CONNECT_MES] == TRUE)
				{
					pDC->SetBkColor(COLOR_GREEN128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_GREEN128];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}

			if ((pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L1_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L2_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L3_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L4_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH14) 
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK1L5_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L1_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH6) 
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L2_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH6) 
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH10) 
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L3_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH2) 
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L4_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK2L5_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L1_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L2_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L3_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L4_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK3L5_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L1_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L2_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L3_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L4_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK4L5_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L1_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L2_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L3_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L4_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK5L5_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L1_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L2_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L3_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L4_CH16)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH1) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH2)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH3) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH4)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH5) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH6)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH7) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH8)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH9) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH10)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH11) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH12)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH13) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH14)
				|| (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH15) || (pWnd->GetDlgCtrlID() == IDC_STT_RACK6L5_CH16)
				)
			{
				int chStt;
				chStt = Lf_getChannelInfo(pWnd->GetDlgCtrlID());
				if (chStt == STATUS_IDLE)
				{
					pDC->SetBkColor(COLOR_GRAY128);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_GRAY128];
				}
				else if (chStt == STATUS_RUN)
				{
					pDC->SetBkColor(COLOR_LIGHT_GREEN);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_LIGHT_GREEN];
				}
				else if (chStt == STATUS_ERROR)
				{
					pDC->SetBkColor(COLOR_PURPLE);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_PURPLE];
				}
				else if (chStt == STATUS_UNUSE)
				{
					pDC->SetBkColor(COLOR_DARK_YELLOW);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_DARK_YELLOW];
				}
				else
				{
					pDC->SetBkColor(COLOR_DARK_RED);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_DARK_RED];
				}
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DESC_NOT_CONN)
			{
				pDC->SetBkColor(COLOR_DARK_RED);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_DARK_RED];
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DESC_IDLE)
			{
				pDC->SetBkColor(COLOR_GRAY128);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_GRAY128];
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DESC_RUN)
			{
				pDC->SetBkColor(COLOR_LIGHT_GREEN);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_LIGHT_GREEN];
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DESC_ERROR)
			{
				pDC->SetBkColor(COLOR_PURPLE);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_PURPLE];
			}
			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_DESC_UNUSE)
			{
				pDC->SetBkColor(COLOR_DARK_YELLOW);
				pDC->SetTextColor(COLOR_BLACK);
				return m_Brush[COLOR_IDX_DARK_YELLOW];
			}

			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_FW_VER_RACK1)
			{
				if (lpInspWorkInfo->m_nFwVerifyResult[RACK_1] == TRUE)
				{
					pDC->SetBkColor(COLOR_GRAY224);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_GRAY224];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}
			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_FW_VER_RACK2)
			{
				if (lpInspWorkInfo->m_nFwVerifyResult[RACK_2] == TRUE)
				{
					pDC->SetBkColor(COLOR_GRAY224);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_GRAY224];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}
			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_FW_VER_RACK3)
			{
				if (lpInspWorkInfo->m_nFwVerifyResult[RACK_3] == TRUE)
				{
					pDC->SetBkColor(COLOR_GRAY224);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_GRAY224];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}
			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_FW_VER_RACK4)
			{
				if (lpInspWorkInfo->m_nFwVerifyResult[RACK_4] == TRUE)
				{
					pDC->SetBkColor(COLOR_GRAY224);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_GRAY224];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}
			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_FW_VER_RACK5)
			{
				if (lpInspWorkInfo->m_nFwVerifyResult[RACK_5] == TRUE)
				{
					pDC->SetBkColor(COLOR_GRAY224);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_GRAY224];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}
			if (pWnd->GetDlgCtrlID() == IDC_STT_MA_FW_VER_RACK6)
			{
				if (lpInspWorkInfo->m_nFwVerifyResult[RACK_6] == TRUE)
				{
					pDC->SetBkColor(COLOR_GRAY224);
					pDC->SetTextColor(COLOR_BLACK);
					return m_Brush[COLOR_IDX_GRAY224];
				}
				else
				{
					pDC->SetBkColor(COLOR_RED128);
					pDC->SetTextColor(COLOR_WHITE);
					return m_Brush[COLOR_IDX_RED128];
				}
			}

			break;
	}
	// TODO:  기본값이 적당하지 않으면 다른 브러시를 반환합니다.
	return hbr;
}

BOOL CHseAgingDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_F4)
	{
		if (::GetKeyState(VK_MENU) < 0)	return TRUE;
	}

	// 일반 Key 동작에 대한 Event
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return 1;
		}
		else if (pMsg->wParam == VK_RETURN)
		{
			return 1;
		}
		else if (pMsg->wParam == VK_SPACE)
		{
			return 1;
		}
		else if ((pMsg->wParam >= 33) && (pMsg->wParam <= 'z'))	// ! ~ z 까지의문자만 입력받음
		{
			CString sdata;
			sdata.Format(_T("%c"), pMsg->wParam);
			m_nSendKeyInData.Append(sdata);
			CString rackID;
			CString channelID;

			if (m_nSendKeyInData.GetLength() >= 10) // 10개 입력 된 순간부터
			{
				CString IndexStr = m_nSendKeyInData.Right(10);
				CString LeftPart = IndexStr.Left(4);
				CString RightPart = IndexStr.Right(4);
				CString RightPart2 = RightPart.Left(2);
				int rackLength = 6; // "RACK01"의 길이 (6자)
				rackID = m_nSendKeyInData.Left(rackLength); // RACK 부분 저장
				channelID = m_nSendKeyInData.Right(m_nSendKeyInData.GetLength() - rackLength); // CH 부분 저장

				if (LeftPart.CompareNoCase(_T("RACK")) == 0 && RightPart2.CompareNoCase(_T("CH")) == 0)
				{
					Lf_checkBcrRackIDInput();
					m_nSendKeyInData.Empty();
					return 1;
				}
			}

			return 1;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CHseAgingDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == 1)
	{
		KillTimer(1);

		int nowTick;
		nowTick = (int)::GetTickCount();

		if((nowTick - m_nAgingStatusTick) > (lpSystemInfo->m_fRefreshPowerMeasureTime * 1000))
		{
			m_nAgingStatusTick = nowTick;
			Lf_getMeasurePower();
		}
		else if((nowTick - m_nPowerMeasureTick) > (lpSystemInfo->m_fRefreshAgingStatusTime * 1000))
		{
			m_nPowerMeasureTick = nowTick;
			Lf_getAgingStatus();
		}
		else if ((nowTick - m_nSensinglogTick) > (lpSystemInfo->m_nSensingLogInterval * 1000))
		{
			m_nSensinglogTick = nowTick;
			/*Lf_writeSensingLog();*/
		}
		if ((nowTick - m_nMeasureTick) >= 500)
		{
			// Power Measure와 Aging Status를 Update 한다.
			m_nMeasureTick = nowTick;
			Lf_updateEthConnectInfo();
			Lf_updateAgingStatus();
			Lf_updateFirmwareMatching();
			Lf_checkPowerLimitAlarm();

			// Tower Lamp Status 업데이트 한다.
			Lf_updateTowerLamp();

			// AGING Complete 5분 경과 시 Alarm 발생
			Lf_checkComplete5MinOver();

			// Aging Complete RACK에 대하여 UI 깜빡임
			Lf_flickerCompleteRackNumber();

			
		}
		//if ((nowTick - m_nFirmwareTick) >= (0.1 * 60 * 1000) && m_nFirmwareTick_d <= 1.1)
		//{
		//	m_nFirmwareTick = nowTick;
		//	Lf_updateFirmwareMatching();
		//	m_nFirmwareTick_d += 1;
		//}

		//if ((nowTick - m_nFirmwareTick) >= (1 * 60 * 1000)) // 5분(300,000ms)
		//{
		//	m_nFirmwareTick = nowTick;
		//	Lf_updateFirmwareMatching();
		//}


		SetTimer(1, 100, NULL);
	}
	if (nIDEvent == 2)
	{
		// 1초 Timer
		Lf_getTemperature();
		Lf_getDIOStatus();
		// Sensing Log
		//Lf_writeSensingLog();
		Lf_AgingProgressLog();
	}
	if (nIDEvent == 3)
	{
		// 3초 Timer
		AfxBeginThread(ThreadHandBcrSearch, this);
		AfxBeginThread(ThreadFwVersionRead, this);
		AfxBeginThread(ThreadTempST590_1, this);
		

		/*for (int i = 0; i < 6; i++)
		{
			if(i == 0)
			{ 
				lpInspWorkInfo->m_fTempReadVal[i] = 45 + lpInspWorkInfo->TempTest;
			}
			if (i == 1)
			{
				lpInspWorkInfo->m_fTempReadVal[i] = 46 + lpInspWorkInfo->TempTest;
			}
			if (i == 2)
			{
				lpInspWorkInfo->m_fTempReadVal[i] = 47 + lpInspWorkInfo->TempTest;
			}
			if (i == 3)
			{
				lpInspWorkInfo->m_fTempReadVal[i] = 48 + lpInspWorkInfo->TempTest;
			}
			if (i == 4)
			{
				lpInspWorkInfo->m_fTempReadVal[i] = 49 + lpInspWorkInfo->TempTest;
			}
			if (i == 5)
			{
				lpInspWorkInfo->m_fTempReadVal[i] = 50 + lpInspWorkInfo->TempTest;
			}
		}*/

		//AfxBeginThread(ThreadTempControler, this);
	}

	if (nIDEvent == 8)
	{	
		if (lpInspWorkInfo->m_bAlarmOccur == TRUE)
		{
			KillTimer(8);

			m_pApp->Gf_ShowMessageBox(lpInspWorkInfo->m_sAlarmMessage);

			lpInspWorkInfo->m_bAlarmOccur = FALSE;

			SetTimer(8, 1000, NULL);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CHseAgingDlg::OnBnClickedBtnMaUser()
{
	CString sLog;
	sLog.Format(_T("<MESSAGE> USER BUTTON CLICK"));
	m_pApp->Gf_writeMLog(sLog);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	KillTimer(1);

	CUserID user_dlg;
	if (user_dlg.DoModal() == IDOK)
	{
		Lf_updateSystemInfo();
	}

	SetTimer(1, 100, NULL);
}

void CHseAgingDlg::OnBnClickedBtnMaMonitoring()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString sLog;
	sLog.Format(_T("<MESSAGE> MONITORING BUTTON CLICK"));
	m_pApp->Gf_writeMLog(sLog);

	CMonitoring mnt_dlg;
	mnt_dlg.DoModal();
}


void CHseAgingDlg::OnBnClickedBtnMaModel()
{
	CString sLog;
	sLog.Format(_T("<MESSAGE> MODEL BUTTON CLICK"));
	m_pApp->Gf_writeMLog(sLog);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (Lf_checkAgingIDLEMode() == FALSE)
		return;

	KillTimer(1);

	CModelInfo model_dlg;
	model_dlg.DoModal();

	Lf_InitCobmoRackModelList();

	SetTimer(1, 100, NULL);
}


void CHseAgingDlg::OnBnClickedBtnMaPidInput()
{
	CString sLog;
	sLog.Format(_T("<MESSAGE> PID BUTTON CLICK"));
	m_pApp->Gf_writeMLog(sLog);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	KillTimer(1);

	CPidInput id_dlg;
	id_dlg.m_nMesAutoDMOU = MES_DMOU_MODE_MANUAL;
	id_dlg.DoModal();

	SetTimer(1, 100, NULL);
}


void CHseAgingDlg::OnBnClickedBtnMaFirmware()
{
	CString sLog;
	sLog.Format(_T("<MESSAGE> FIRMWARE BUTTON CLICK"));
	m_pApp->Gf_writeMLog(sLog);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (Lf_checkAgingIDLEMode() == FALSE)
		return;

	KillTimer(1);

	CAutoFirmware af_dlg;
	af_dlg.DoModal();

	SetTimer(1, 100, NULL);
}


void CHseAgingDlg::OnBnClickedBtnMaSystem()
{
	CString sLog;
	sLog.Format(_T("<MESSAGE> SYSTEM BUTTON CLICK"));
	m_pApp->Gf_writeMLog(sLog);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (Lf_checkAgingIDLEMode() == FALSE)
		return;

	// 2025-01-13 PDH. Password 입력 기능 추가
	CPassword pw_dlg;
	if (pw_dlg.DoModal() == IDCANCEL)
		return;

	KillTimer(1);

	CSystem sys_dlg;
	if (sys_dlg.DoModal() == IDOK)
	{
		m_pApp->Gf_InitialSystemInfo();
		Lf_updateSystemInfo();
	}

	SetTimer(1, 100, NULL);
}


void CHseAgingDlg::OnBnClickedBtnMaExit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (Lf_checkAgingIDLEMode() == FALSE)
		return;

	KillTimer(1);

	CMessageQuestion msg_dlg;
	msg_dlg.m_strQMessage.Format(_T("Do you want exit the program ?"));
	if (msg_dlg.DoModal() == IDOK)
	{
		CDialog::OnOK();
		return;
	}

	SetTimer(1, 100, NULL);
}

void CHseAgingDlg::OnBnClickedMbtMaBuzzOff()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	lpInspWorkInfo->m_nDioOutputData = lpInspWorkInfo->m_nDioOutputData & ~DIO_OUT_BUZZER;

	m_pApp->pCommand->Gf_dio_setDIOWriteOutput(lpInspWorkInfo->m_nDioOutputData, lpInspWorkInfo->m_nDioOutputMode);
}

void CHseAgingDlg::OnBnClickedMbcMaStartRack1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTART(RACK_1);
}


void CHseAgingDlg::OnBnClickedMbcMaStopRack1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTOP(RACK_1);
}


void CHseAgingDlg::OnBnClickedMbcMaFusingRack1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingFUSING(RACK_1);
}


void CHseAgingDlg::OnBnClickedMbcMaStartRack2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTART(RACK_2);
}


void CHseAgingDlg::OnBnClickedMbcMaStopRack2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTOP(RACK_2);
}


void CHseAgingDlg::OnBnClickedMbcMaFusingRack2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingFUSING(RACK_2);
}


void CHseAgingDlg::OnBnClickedMbcMaStartRack3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTART(RACK_3);
}


void CHseAgingDlg::OnBnClickedMbcMaStopRack3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTOP(RACK_3);
}


void CHseAgingDlg::OnBnClickedMbcMaFusingRack3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingFUSING(RACK_3);
}


void CHseAgingDlg::OnBnClickedMbcMaStartRack4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTART(RACK_4);
}


void CHseAgingDlg::OnBnClickedMbcMaStopRack4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTOP(RACK_4);
}


void CHseAgingDlg::OnBnClickedMbcMaFusingRack4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingFUSING(RACK_4);
}


void CHseAgingDlg::OnBnClickedMbcMaStartRack5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTART(RACK_5);
}


void CHseAgingDlg::OnBnClickedMbcMaStopRack5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTOP(RACK_5);
}


void CHseAgingDlg::OnBnClickedMbcMaFusingRack5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingFUSING(RACK_5);
}


void CHseAgingDlg::OnBnClickedMbcMaStartRack6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTART(RACK_6);
}


void CHseAgingDlg::OnBnClickedMbcMaStopRack6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingSTOP(RACK_6);
}


void CHseAgingDlg::OnBnClickedMbcMaFusingRack6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setAgingFUSING(RACK_6);
}

void CHseAgingDlg::OnBnClickedChkMaSelectRack1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_channelUseButtonShowHide(RACK_1);
}


void CHseAgingDlg::OnBnClickedChkMaSelectRack2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_channelUseButtonShowHide(RACK_2);
}


void CHseAgingDlg::OnBnClickedChkMaSelectRack3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_channelUseButtonShowHide(RACK_3);
}


void CHseAgingDlg::OnBnClickedChkMaSelectRack4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_channelUseButtonShowHide(RACK_4);
}


void CHseAgingDlg::OnBnClickedChkMaSelectRack5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_channelUseButtonShowHide(RACK_5);
}


void CHseAgingDlg::OnBnClickedChkMaSelectRack6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_channelUseButtonShowHide(RACK_6);
}


void CHseAgingDlg::OnBnClickedMbcMaChSetRack1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setChannelUseUnuse(RACK_1);
}


void CHseAgingDlg::OnBnClickedMbcMaChSetRack2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setChannelUseUnuse(RACK_2);
}


void CHseAgingDlg::OnBnClickedMbcMaChSetRack3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setChannelUseUnuse(RACK_3);
}


void CHseAgingDlg::OnBnClickedMbcMaChSetRack4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setChannelUseUnuse(RACK_4);
}


void CHseAgingDlg::OnBnClickedMbcMaChSetRack5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setChannelUseUnuse(RACK_5);
}


void CHseAgingDlg::OnBnClickedMbcMaChSetRack6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setChannelUseUnuse(RACK_6);
}

void CHseAgingDlg::OnStnClickedChEnableDisable(UINT nID)
{
	if (m_pChkChSelect[RACK_1]->GetCheck() == TRUE)
	{
		switch (nID)
		{
		case IDC_STT_RACK1L1_CH1: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_1);	break; }
		case IDC_STT_RACK1L1_CH2: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_2);	break; }
		case IDC_STT_RACK1L1_CH3: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_3);	break; }
		case IDC_STT_RACK1L1_CH4: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_4);	break; }
		case IDC_STT_RACK1L1_CH5: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_5);	break; }
		case IDC_STT_RACK1L1_CH6: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_6);	break; }
		case IDC_STT_RACK1L1_CH7: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_7);	break; }
		case IDC_STT_RACK1L1_CH8: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_8);	break; }
		case IDC_STT_RACK1L1_CH9: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_9);	break; }
		case IDC_STT_RACK1L1_CH10: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_10);	break; }
		case IDC_STT_RACK1L1_CH11: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_11);	break; }
		case IDC_STT_RACK1L1_CH12: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_12);	break; }
		case IDC_STT_RACK1L1_CH13: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_13);	break; }
		case IDC_STT_RACK1L1_CH14: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_14);	break; }
		case IDC_STT_RACK1L1_CH15: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_15);	break; }
		case IDC_STT_RACK1L1_CH16: {Lf_toggleChUseUnuse(RACK_1, LAYER_1, CH_16);	break; }
		case IDC_STT_RACK1L2_CH1: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_1);	break; }
		case IDC_STT_RACK1L2_CH2: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_2);	break; }
		case IDC_STT_RACK1L2_CH3: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_3);	break; }
		case IDC_STT_RACK1L2_CH4: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_4);	break; }
		case IDC_STT_RACK1L2_CH5: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_5);	break; }
		case IDC_STT_RACK1L2_CH6: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_6);	break; }
		case IDC_STT_RACK1L2_CH7: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_7);	break; }
		case IDC_STT_RACK1L2_CH8: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_8);	break; }
		case IDC_STT_RACK1L2_CH9: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_9);	break; }
		case IDC_STT_RACK1L2_CH10: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_10);	break; }
		case IDC_STT_RACK1L2_CH11: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_11);	break; }
		case IDC_STT_RACK1L2_CH12: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_12);	break; }
		case IDC_STT_RACK1L2_CH13: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_13);	break; }
		case IDC_STT_RACK1L2_CH14: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_14);	break; }
		case IDC_STT_RACK1L2_CH15: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_15);	break; }
		case IDC_STT_RACK1L2_CH16: {Lf_toggleChUseUnuse(RACK_1, LAYER_2, CH_16);	break; }
		case IDC_STT_RACK1L3_CH1: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_1);	break; }
		case IDC_STT_RACK1L3_CH2: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_2);	break; }
		case IDC_STT_RACK1L3_CH3: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_3);	break; }
		case IDC_STT_RACK1L3_CH4: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_4);	break; }
		case IDC_STT_RACK1L3_CH5: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_5);	break; }
		case IDC_STT_RACK1L3_CH6: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_6);	break; }
		case IDC_STT_RACK1L3_CH7: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_7);	break; }
		case IDC_STT_RACK1L3_CH8: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_8);	break; }
		case IDC_STT_RACK1L3_CH9: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_9);	break; }
		case IDC_STT_RACK1L3_CH10: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_10);	break; }
		case IDC_STT_RACK1L3_CH11: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_11);	break; }
		case IDC_STT_RACK1L3_CH12: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_12);	break; }
		case IDC_STT_RACK1L3_CH13: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_13);	break; }
		case IDC_STT_RACK1L3_CH14: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_14);	break; }
		case IDC_STT_RACK1L3_CH15: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_15);	break; }
		case IDC_STT_RACK1L3_CH16: {Lf_toggleChUseUnuse(RACK_1, LAYER_3, CH_16);	break; }
		case IDC_STT_RACK1L4_CH1: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_1);	break; }
		case IDC_STT_RACK1L4_CH2: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_2);	break; }
		case IDC_STT_RACK1L4_CH3: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_3);	break; }
		case IDC_STT_RACK1L4_CH4: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_4);	break; }
		case IDC_STT_RACK1L4_CH5: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_5);	break; }
		case IDC_STT_RACK1L4_CH6: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_6);	break; }
		case IDC_STT_RACK1L4_CH7: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_7);	break; }
		case IDC_STT_RACK1L4_CH8: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_8);	break; }
		case IDC_STT_RACK1L4_CH9: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_9);	break; }
		case IDC_STT_RACK1L4_CH10: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_10);	break; }
		case IDC_STT_RACK1L4_CH11: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_11);	break; }
		case IDC_STT_RACK1L4_CH12: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_12);	break; }
		case IDC_STT_RACK1L4_CH13: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_13);	break; }
		case IDC_STT_RACK1L4_CH14: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_14);	break; }
		case IDC_STT_RACK1L4_CH15: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_15);	break; }
		case IDC_STT_RACK1L4_CH16: {Lf_toggleChUseUnuse(RACK_1, LAYER_4, CH_16);	break; }
		case IDC_STT_RACK1L5_CH1: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_1);	break; }
		case IDC_STT_RACK1L5_CH2: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_2);	break; }
		case IDC_STT_RACK1L5_CH3: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_3);	break; }
		case IDC_STT_RACK1L5_CH4: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_4);	break; }
		case IDC_STT_RACK1L5_CH5: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_5);	break; }
		case IDC_STT_RACK1L5_CH6: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_6);	break; }
		case IDC_STT_RACK1L5_CH7: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_7);	break; }
		case IDC_STT_RACK1L5_CH8: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_8);	break; }
		case IDC_STT_RACK1L5_CH9: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_9);	break; }
		case IDC_STT_RACK1L5_CH10: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_10);	break; }
		case IDC_STT_RACK1L5_CH11: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_11);	break; }
		case IDC_STT_RACK1L5_CH12: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_12);	break; }
		case IDC_STT_RACK1L5_CH13: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_13);	break; }
		case IDC_STT_RACK1L5_CH14: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_14);	break; }
		case IDC_STT_RACK1L5_CH15: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_15);	break; }
		case IDC_STT_RACK1L5_CH16: {Lf_toggleChUseUnuse(RACK_1, LAYER_5, CH_16);	break; }
		}
	}

	if (m_pChkChSelect[RACK_2]->GetCheck() == TRUE)
	{
		switch (nID)
		{
		case IDC_STT_RACK2L1_CH1: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_1);	break; }
		case IDC_STT_RACK2L1_CH2: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_2);	break; }
		case IDC_STT_RACK2L1_CH3: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_3);	break; }
		case IDC_STT_RACK2L1_CH4: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_4);	break; }
		case IDC_STT_RACK2L1_CH5: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_5);	break; }
		case IDC_STT_RACK2L1_CH6: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_6);	break; }
		case IDC_STT_RACK2L1_CH7: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_7);	break; }
		case IDC_STT_RACK2L1_CH8: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_8);	break; }
		case IDC_STT_RACK2L1_CH9: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_9);	break; }
		case IDC_STT_RACK2L1_CH10: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_10);	break; }
		case IDC_STT_RACK2L1_CH11: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_11);	break; }
		case IDC_STT_RACK2L1_CH12: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_12);	break; }
		case IDC_STT_RACK2L1_CH13: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_13);	break; }
		case IDC_STT_RACK2L1_CH14: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_14);	break; }
		case IDC_STT_RACK2L1_CH15: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_15);	break; }
		case IDC_STT_RACK2L1_CH16: {Lf_toggleChUseUnuse(RACK_2, LAYER_1, CH_16);	break; }
		case IDC_STT_RACK2L2_CH1: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_1);	break; }
		case IDC_STT_RACK2L2_CH2: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_2);	break; }
		case IDC_STT_RACK2L2_CH3: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_3);	break; }
		case IDC_STT_RACK2L2_CH4: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_4);	break; }
		case IDC_STT_RACK2L2_CH5: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_5);	break; }
		case IDC_STT_RACK2L2_CH6: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_6);	break; }
		case IDC_STT_RACK2L2_CH7: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_7);	break; }
		case IDC_STT_RACK2L2_CH8: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_8);	break; }
		case IDC_STT_RACK2L2_CH9: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_9);	break; }
		case IDC_STT_RACK2L2_CH10: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_10);	break; }
		case IDC_STT_RACK2L2_CH11: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_11);	break; }
		case IDC_STT_RACK2L2_CH12: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_12);	break; }
		case IDC_STT_RACK2L2_CH13: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_13);	break; }
		case IDC_STT_RACK2L2_CH14: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_14);	break; }
		case IDC_STT_RACK2L2_CH15: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_15);	break; }
		case IDC_STT_RACK2L2_CH16: {Lf_toggleChUseUnuse(RACK_2, LAYER_2, CH_16);	break; }
		case IDC_STT_RACK2L3_CH1: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_1);	break; }
		case IDC_STT_RACK2L3_CH2: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_2);	break; }
		case IDC_STT_RACK2L3_CH3: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_3);	break; }
		case IDC_STT_RACK2L3_CH4: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_4);	break; }
		case IDC_STT_RACK2L3_CH5: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_5);	break; }
		case IDC_STT_RACK2L3_CH6: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_6);	break; }
		case IDC_STT_RACK2L3_CH7: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_7);	break; }
		case IDC_STT_RACK2L3_CH8: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_8);	break; }
		case IDC_STT_RACK2L3_CH9: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_9);	break; }
		case IDC_STT_RACK2L3_CH10: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_10);	break; }
		case IDC_STT_RACK2L3_CH11: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_11);	break; }
		case IDC_STT_RACK2L3_CH12: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_12);	break; }
		case IDC_STT_RACK2L3_CH13: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_13);	break; }
		case IDC_STT_RACK2L3_CH14: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_14);	break; }
		case IDC_STT_RACK2L3_CH15: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_15);	break; }
		case IDC_STT_RACK2L3_CH16: {Lf_toggleChUseUnuse(RACK_2, LAYER_3, CH_16);	break; }
		case IDC_STT_RACK2L4_CH1: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_1);	break; }
		case IDC_STT_RACK2L4_CH2: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_2);	break; }
		case IDC_STT_RACK2L4_CH3: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_3);	break; }
		case IDC_STT_RACK2L4_CH4: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_4);	break; }
		case IDC_STT_RACK2L4_CH5: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_5);	break; }
		case IDC_STT_RACK2L4_CH6: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_6);	break; }
		case IDC_STT_RACK2L4_CH7: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_7);	break; }
		case IDC_STT_RACK2L4_CH8: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_8);	break; }
		case IDC_STT_RACK2L4_CH9: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_9);	break; }
		case IDC_STT_RACK2L4_CH10: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_10);	break; }
		case IDC_STT_RACK2L4_CH11: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_11);	break; }
		case IDC_STT_RACK2L4_CH12: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_12);	break; }
		case IDC_STT_RACK2L4_CH13: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_13);	break; }
		case IDC_STT_RACK2L4_CH14: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_14);	break; }
		case IDC_STT_RACK2L4_CH15: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_15);	break; }
		case IDC_STT_RACK2L4_CH16: {Lf_toggleChUseUnuse(RACK_2, LAYER_4, CH_16);	break; }
		case IDC_STT_RACK2L5_CH1: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_1);	break; }
		case IDC_STT_RACK2L5_CH2: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_2);	break; }
		case IDC_STT_RACK2L5_CH3: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_3);	break; }
		case IDC_STT_RACK2L5_CH4: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_4);	break; }
		case IDC_STT_RACK2L5_CH5: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_5);	break; }
		case IDC_STT_RACK2L5_CH6: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_6);	break; }
		case IDC_STT_RACK2L5_CH7: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_7);	break; }
		case IDC_STT_RACK2L5_CH8: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_8);	break; }
		case IDC_STT_RACK2L5_CH9: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_9);	break; }
		case IDC_STT_RACK2L5_CH10: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_10);	break; }
		case IDC_STT_RACK2L5_CH11: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_11);	break; }
		case IDC_STT_RACK2L5_CH12: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_12);	break; }
		case IDC_STT_RACK2L5_CH13: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_13);	break; }
		case IDC_STT_RACK2L5_CH14: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_14);	break; }
		case IDC_STT_RACK2L5_CH15: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_15);	break; }
		case IDC_STT_RACK2L5_CH16: {Lf_toggleChUseUnuse(RACK_2, LAYER_5, CH_16);	break; }
		}
	}

	if (m_pChkChSelect[RACK_3]->GetCheck() == TRUE)
	{
		switch (nID)
		{
		case IDC_STT_RACK3L1_CH1: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_1);	break; }
		case IDC_STT_RACK3L1_CH2: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_2);	break; }
		case IDC_STT_RACK3L1_CH3: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_3);	break; }
		case IDC_STT_RACK3L1_CH4: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_4);	break; }
		case IDC_STT_RACK3L1_CH5: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_5);	break; }
		case IDC_STT_RACK3L1_CH6: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_6);	break; }
		case IDC_STT_RACK3L1_CH7: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_7);	break; }
		case IDC_STT_RACK3L1_CH8: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_8);	break; }
		case IDC_STT_RACK3L1_CH9: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_9);	break; }
		case IDC_STT_RACK3L1_CH10: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_10);	break; }
		case IDC_STT_RACK3L1_CH11: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_11);	break; }
		case IDC_STT_RACK3L1_CH12: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_12);	break; }
		case IDC_STT_RACK3L1_CH13: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_13);	break; }
		case IDC_STT_RACK3L1_CH14: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_14);	break; }
		case IDC_STT_RACK3L1_CH15: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_15);	break; }
		case IDC_STT_RACK3L1_CH16: {Lf_toggleChUseUnuse(RACK_3, LAYER_1, CH_16);	break; }
		case IDC_STT_RACK3L2_CH1: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_1);	break; }
		case IDC_STT_RACK3L2_CH2: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_2);	break; }
		case IDC_STT_RACK3L2_CH3: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_3);	break; }
		case IDC_STT_RACK3L2_CH4: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_4);	break; }
		case IDC_STT_RACK3L2_CH5: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_5);	break; }
		case IDC_STT_RACK3L2_CH6: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_6);	break; }
		case IDC_STT_RACK3L2_CH7: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_7);	break; }
		case IDC_STT_RACK3L2_CH8: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_8);	break; }
		case IDC_STT_RACK3L2_CH9: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_9);	break; }
		case IDC_STT_RACK3L2_CH10: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_10);	break; }
		case IDC_STT_RACK3L2_CH11: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_11);	break; }
		case IDC_STT_RACK3L2_CH12: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_12);	break; }
		case IDC_STT_RACK3L2_CH13: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_13);	break; }
		case IDC_STT_RACK3L2_CH14: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_14);	break; }
		case IDC_STT_RACK3L2_CH15: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_15);	break; }
		case IDC_STT_RACK3L2_CH16: {Lf_toggleChUseUnuse(RACK_3, LAYER_2, CH_16);	break; }
		case IDC_STT_RACK3L3_CH1: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_1);	break; }
		case IDC_STT_RACK3L3_CH2: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_2);	break; }
		case IDC_STT_RACK3L3_CH3: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_3);	break; }
		case IDC_STT_RACK3L3_CH4: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_4);	break; }
		case IDC_STT_RACK3L3_CH5: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_5);	break; }
		case IDC_STT_RACK3L3_CH6: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_6);	break; }
		case IDC_STT_RACK3L3_CH7: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_7);	break; }
		case IDC_STT_RACK3L3_CH8: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_8);	break; }
		case IDC_STT_RACK3L3_CH9: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_9);	break; }
		case IDC_STT_RACK3L3_CH10: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_10);	break; }
		case IDC_STT_RACK3L3_CH11: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_11);	break; }
		case IDC_STT_RACK3L3_CH12: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_12);	break; }
		case IDC_STT_RACK3L3_CH13: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_13);	break; }
		case IDC_STT_RACK3L3_CH14: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_14);	break; }
		case IDC_STT_RACK3L3_CH15: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_15);	break; }
		case IDC_STT_RACK3L3_CH16: {Lf_toggleChUseUnuse(RACK_3, LAYER_3, CH_16);	break; }
		case IDC_STT_RACK3L4_CH1: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_1);	break; }
		case IDC_STT_RACK3L4_CH2: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_2);	break; }
		case IDC_STT_RACK3L4_CH3: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_3);	break; }
		case IDC_STT_RACK3L4_CH4: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_4);	break; }
		case IDC_STT_RACK3L4_CH5: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_5);	break; }
		case IDC_STT_RACK3L4_CH6: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_6);	break; }
		case IDC_STT_RACK3L4_CH7: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_7);	break; }
		case IDC_STT_RACK3L4_CH8: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_8);	break; }
		case IDC_STT_RACK3L4_CH9: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_9);	break; }
		case IDC_STT_RACK3L4_CH10: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_10);	break; }
		case IDC_STT_RACK3L4_CH11: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_11);	break; }
		case IDC_STT_RACK3L4_CH12: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_12);	break; }
		case IDC_STT_RACK3L4_CH13: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_13);	break; }
		case IDC_STT_RACK3L4_CH14: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_14);	break; }
		case IDC_STT_RACK3L4_CH15: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_15);	break; }
		case IDC_STT_RACK3L4_CH16: {Lf_toggleChUseUnuse(RACK_3, LAYER_4, CH_16);	break; }
		case IDC_STT_RACK3L5_CH1: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_1);	break; }
		case IDC_STT_RACK3L5_CH2: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_2);	break; }
		case IDC_STT_RACK3L5_CH3: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_3);	break; }
		case IDC_STT_RACK3L5_CH4: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_4);	break; }
		case IDC_STT_RACK3L5_CH5: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_5);	break; }
		case IDC_STT_RACK3L5_CH6: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_6);	break; }
		case IDC_STT_RACK3L5_CH7: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_7);	break; }
		case IDC_STT_RACK3L5_CH8: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_8);	break; }
		case IDC_STT_RACK3L5_CH9: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_9);	break; }
		case IDC_STT_RACK3L5_CH10: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_10);	break; }
		case IDC_STT_RACK3L5_CH11: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_11);	break; }
		case IDC_STT_RACK3L5_CH12: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_12);	break; }
		case IDC_STT_RACK3L5_CH13: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_13);	break; }
		case IDC_STT_RACK3L5_CH14: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_14);	break; }
		case IDC_STT_RACK3L5_CH15: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_15);	break; }
		case IDC_STT_RACK3L5_CH16: {Lf_toggleChUseUnuse(RACK_3, LAYER_5, CH_16);	break; }
		}
	}

	if (m_pChkChSelect[RACK_4]->GetCheck() == TRUE)
	{
		switch (nID)
		{
		case IDC_STT_RACK4L1_CH1: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_1);	break; }
		case IDC_STT_RACK4L1_CH2: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_2);	break; }
		case IDC_STT_RACK4L1_CH3: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_3);	break; }
		case IDC_STT_RACK4L1_CH4: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_4);	break; }
		case IDC_STT_RACK4L1_CH5: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_5);	break; }
		case IDC_STT_RACK4L1_CH6: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_6);	break; }
		case IDC_STT_RACK4L1_CH7: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_7);	break; }
		case IDC_STT_RACK4L1_CH8: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_8);	break; }
		case IDC_STT_RACK4L1_CH9: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_9);	break; }
		case IDC_STT_RACK4L1_CH10: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_10);	break; }
		case IDC_STT_RACK4L1_CH11: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_11);	break; }
		case IDC_STT_RACK4L1_CH12: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_12);	break; }
		case IDC_STT_RACK4L1_CH13: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_13);	break; }
		case IDC_STT_RACK4L1_CH14: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_14);	break; }
		case IDC_STT_RACK4L1_CH15: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_15);	break; }
		case IDC_STT_RACK4L1_CH16: {Lf_toggleChUseUnuse(RACK_4, LAYER_1, CH_16);	break; }
		case IDC_STT_RACK4L2_CH1: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_1);	break; }
		case IDC_STT_RACK4L2_CH2: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_2);	break; }
		case IDC_STT_RACK4L2_CH3: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_3);	break; }
		case IDC_STT_RACK4L2_CH4: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_4);	break; }
		case IDC_STT_RACK4L2_CH5: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_5);	break; }
		case IDC_STT_RACK4L2_CH6: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_6);	break; }
		case IDC_STT_RACK4L2_CH7: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_7);	break; }
		case IDC_STT_RACK4L2_CH8: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_8);	break; }
		case IDC_STT_RACK4L2_CH9: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_9);	break; }
		case IDC_STT_RACK4L2_CH10: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_10);	break; }
		case IDC_STT_RACK4L2_CH11: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_11);	break; }
		case IDC_STT_RACK4L2_CH12: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_12);	break; }
		case IDC_STT_RACK4L2_CH13: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_13);	break; }
		case IDC_STT_RACK4L2_CH14: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_14);	break; }
		case IDC_STT_RACK4L2_CH15: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_15);	break; }
		case IDC_STT_RACK4L2_CH16: {Lf_toggleChUseUnuse(RACK_4, LAYER_2, CH_16);	break; }
		case IDC_STT_RACK4L3_CH1: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_1);	break; }
		case IDC_STT_RACK4L3_CH2: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_2);	break; }
		case IDC_STT_RACK4L3_CH3: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_3);	break; }
		case IDC_STT_RACK4L3_CH4: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_4);	break; }
		case IDC_STT_RACK4L3_CH5: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_5);	break; }
		case IDC_STT_RACK4L3_CH6: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_6);	break; }
		case IDC_STT_RACK4L3_CH7: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_7);	break; }
		case IDC_STT_RACK4L3_CH8: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_8);	break; }
		case IDC_STT_RACK4L3_CH9: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_9);	break; }
		case IDC_STT_RACK4L3_CH10: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_10);	break; }
		case IDC_STT_RACK4L3_CH11: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_11);	break; }
		case IDC_STT_RACK4L3_CH12: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_12);	break; }
		case IDC_STT_RACK4L3_CH13: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_13);	break; }
		case IDC_STT_RACK4L3_CH14: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_14);	break; }
		case IDC_STT_RACK4L3_CH15: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_15);	break; }
		case IDC_STT_RACK4L3_CH16: {Lf_toggleChUseUnuse(RACK_4, LAYER_3, CH_16);	break; }
		case IDC_STT_RACK4L4_CH1: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_1);	break; }
		case IDC_STT_RACK4L4_CH2: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_2);	break; }
		case IDC_STT_RACK4L4_CH3: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_3);	break; }
		case IDC_STT_RACK4L4_CH4: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_4);	break; }
		case IDC_STT_RACK4L4_CH5: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_5);	break; }
		case IDC_STT_RACK4L4_CH6: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_6);	break; }
		case IDC_STT_RACK4L4_CH7: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_7);	break; }
		case IDC_STT_RACK4L4_CH8: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_8);	break; }
		case IDC_STT_RACK4L4_CH9: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_9);	break; }
		case IDC_STT_RACK4L4_CH10: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_10);	break; }
		case IDC_STT_RACK4L4_CH11: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_11);	break; }
		case IDC_STT_RACK4L4_CH12: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_12);	break; }
		case IDC_STT_RACK4L4_CH13: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_13);	break; }
		case IDC_STT_RACK4L4_CH14: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_14);	break; }
		case IDC_STT_RACK4L4_CH15: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_15);	break; }
		case IDC_STT_RACK4L4_CH16: {Lf_toggleChUseUnuse(RACK_4, LAYER_4, CH_16);	break; }
		case IDC_STT_RACK4L5_CH1: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_1);	break; }
		case IDC_STT_RACK4L5_CH2: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_2);	break; }
		case IDC_STT_RACK4L5_CH3: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_3);	break; }
		case IDC_STT_RACK4L5_CH4: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_4);	break; }
		case IDC_STT_RACK4L5_CH5: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_5);	break; }
		case IDC_STT_RACK4L5_CH6: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_6);	break; }
		case IDC_STT_RACK4L5_CH7: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_7);	break; }
		case IDC_STT_RACK4L5_CH8: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_8);	break; }
		case IDC_STT_RACK4L5_CH9: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_9);	break; }
		case IDC_STT_RACK4L5_CH10: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_10);	break; }
		case IDC_STT_RACK4L5_CH11: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_11);	break; }
		case IDC_STT_RACK4L5_CH12: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_12);	break; }
		case IDC_STT_RACK4L5_CH13: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_13);	break; }
		case IDC_STT_RACK4L5_CH14: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_14);	break; }
		case IDC_STT_RACK4L5_CH15: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_15);	break; }
		case IDC_STT_RACK4L5_CH16: {Lf_toggleChUseUnuse(RACK_4, LAYER_5, CH_16);	break; }
		}
	}

	if (m_pChkChSelect[RACK_5]->GetCheck() == TRUE)
	{
		switch (nID)
		{
		case IDC_STT_RACK5L1_CH1: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_1);	break; }
		case IDC_STT_RACK5L1_CH2: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_2);	break; }
		case IDC_STT_RACK5L1_CH3: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_3);	break; }
		case IDC_STT_RACK5L1_CH4: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_4);	break; }
		case IDC_STT_RACK5L1_CH5: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_5);	break; }
		case IDC_STT_RACK5L1_CH6: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_6);	break; }
		case IDC_STT_RACK5L1_CH7: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_7);	break; }
		case IDC_STT_RACK5L1_CH8: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_8);	break; }
		case IDC_STT_RACK5L1_CH9: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_9);	break; }
		case IDC_STT_RACK5L1_CH10: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_10);	break; }
		case IDC_STT_RACK5L1_CH11: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_11);	break; }
		case IDC_STT_RACK5L1_CH12: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_12);	break; }
		case IDC_STT_RACK5L1_CH13: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_13);	break; }
		case IDC_STT_RACK5L1_CH14: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_14);	break; }
		case IDC_STT_RACK5L1_CH15: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_15);	break; }
		case IDC_STT_RACK5L1_CH16: {Lf_toggleChUseUnuse(RACK_5, LAYER_1, CH_16);	break; }
		case IDC_STT_RACK5L2_CH1: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_1);	break; }
		case IDC_STT_RACK5L2_CH2: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_2);	break; }
		case IDC_STT_RACK5L2_CH3: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_3);	break; }
		case IDC_STT_RACK5L2_CH4: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_4);	break; }
		case IDC_STT_RACK5L2_CH5: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_5);	break; }
		case IDC_STT_RACK5L2_CH6: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_6);	break; }
		case IDC_STT_RACK5L2_CH7: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_7);	break; }
		case IDC_STT_RACK5L2_CH8: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_8);	break; }
		case IDC_STT_RACK5L2_CH9: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_9);	break; }
		case IDC_STT_RACK5L2_CH10: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_10);	break; }
		case IDC_STT_RACK5L2_CH11: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_11);	break; }
		case IDC_STT_RACK5L2_CH12: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_12);	break; }
		case IDC_STT_RACK5L2_CH13: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_13);	break; }
		case IDC_STT_RACK5L2_CH14: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_14);	break; }
		case IDC_STT_RACK5L2_CH15: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_15);	break; }
		case IDC_STT_RACK5L2_CH16: {Lf_toggleChUseUnuse(RACK_5, LAYER_2, CH_16);	break; }
		case IDC_STT_RACK5L3_CH1: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_1);	break; }
		case IDC_STT_RACK5L3_CH2: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_2);	break; }
		case IDC_STT_RACK5L3_CH3: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_3);	break; }
		case IDC_STT_RACK5L3_CH4: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_4);	break; }
		case IDC_STT_RACK5L3_CH5: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_5);	break; }
		case IDC_STT_RACK5L3_CH6: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_6);	break; }
		case IDC_STT_RACK5L3_CH7: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_7);	break; }
		case IDC_STT_RACK5L3_CH8: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_8);	break; }
		case IDC_STT_RACK5L3_CH9: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_9);	break; }
		case IDC_STT_RACK5L3_CH10: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_10);	break; }
		case IDC_STT_RACK5L3_CH11: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_11);	break; }
		case IDC_STT_RACK5L3_CH12: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_12);	break; }
		case IDC_STT_RACK5L3_CH13: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_13);	break; }
		case IDC_STT_RACK5L3_CH14: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_14);	break; }
		case IDC_STT_RACK5L3_CH15: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_15);	break; }
		case IDC_STT_RACK5L3_CH16: {Lf_toggleChUseUnuse(RACK_5, LAYER_3, CH_16);	break; }
		case IDC_STT_RACK5L4_CH1: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_1);	break; }
		case IDC_STT_RACK5L4_CH2: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_2);	break; }
		case IDC_STT_RACK5L4_CH3: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_3);	break; }
		case IDC_STT_RACK5L4_CH4: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_4);	break; }
		case IDC_STT_RACK5L4_CH5: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_5);	break; }
		case IDC_STT_RACK5L4_CH6: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_6);	break; }
		case IDC_STT_RACK5L4_CH7: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_7);	break; }
		case IDC_STT_RACK5L4_CH8: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_8);	break; }
		case IDC_STT_RACK5L4_CH9: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_9);	break; }
		case IDC_STT_RACK5L4_CH10: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_10);	break; }
		case IDC_STT_RACK5L4_CH11: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_11);	break; }
		case IDC_STT_RACK5L4_CH12: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_12);	break; }
		case IDC_STT_RACK5L4_CH13: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_13);	break; }
		case IDC_STT_RACK5L4_CH14: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_14);	break; }
		case IDC_STT_RACK5L4_CH15: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_15);	break; }
		case IDC_STT_RACK5L4_CH16: {Lf_toggleChUseUnuse(RACK_5, LAYER_4, CH_16);	break; }
		case IDC_STT_RACK5L5_CH1: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_1);	break; }
		case IDC_STT_RACK5L5_CH2: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_2);	break; }
		case IDC_STT_RACK5L5_CH3: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_3);	break; }
		case IDC_STT_RACK5L5_CH4: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_4);	break; }
		case IDC_STT_RACK5L5_CH5: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_5);	break; }
		case IDC_STT_RACK5L5_CH6: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_6);	break; }
		case IDC_STT_RACK5L5_CH7: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_7);	break; }
		case IDC_STT_RACK5L5_CH8: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_8);	break; }
		case IDC_STT_RACK5L5_CH9: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_9);	break; }
		case IDC_STT_RACK5L5_CH10: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_10);	break; }
		case IDC_STT_RACK5L5_CH11: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_11);	break; }
		case IDC_STT_RACK5L5_CH12: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_12);	break; }
		case IDC_STT_RACK5L5_CH13: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_13);	break; }
		case IDC_STT_RACK5L5_CH14: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_14);	break; }
		case IDC_STT_RACK5L5_CH15: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_15);	break; }
		case IDC_STT_RACK5L5_CH16: {Lf_toggleChUseUnuse(RACK_5, LAYER_5, CH_16);	break; }
		}
	}

	if (m_pChkChSelect[RACK_6]->GetCheck() == TRUE)
	{
		switch (nID)
		{
		case IDC_STT_RACK6L1_CH1: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_1);	break; }
		case IDC_STT_RACK6L1_CH2: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_2);	break; }
		case IDC_STT_RACK6L1_CH3: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_3);	break; }
		case IDC_STT_RACK6L1_CH4: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_4);	break; }
		case IDC_STT_RACK6L1_CH5: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_5);	break; }
		case IDC_STT_RACK6L1_CH6: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_6);	break; }
		case IDC_STT_RACK6L1_CH7: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_7);	break; }
		case IDC_STT_RACK6L1_CH8: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_8);	break; }
		case IDC_STT_RACK6L1_CH9: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_9);	break; }
		case IDC_STT_RACK6L1_CH10: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_10);	break; }
		case IDC_STT_RACK6L1_CH11: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_11);	break; }
		case IDC_STT_RACK6L1_CH12: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_12);	break; }
		case IDC_STT_RACK6L1_CH13: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_13);	break; }
		case IDC_STT_RACK6L1_CH14: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_14);	break; }
		case IDC_STT_RACK6L1_CH15: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_15);	break; }
		case IDC_STT_RACK6L1_CH16: {Lf_toggleChUseUnuse(RACK_6, LAYER_1, CH_16);	break; }
		case IDC_STT_RACK6L2_CH1: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_1);	break; }
		case IDC_STT_RACK6L2_CH2: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_2);	break; }
		case IDC_STT_RACK6L2_CH3: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_3);	break; }
		case IDC_STT_RACK6L2_CH4: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_4);	break; }
		case IDC_STT_RACK6L2_CH5: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_5);	break; }
		case IDC_STT_RACK6L2_CH6: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_6);	break; }
		case IDC_STT_RACK6L2_CH7: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_7);	break; }
		case IDC_STT_RACK6L2_CH8: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_8);	break; }
		case IDC_STT_RACK6L2_CH9: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_9);	break; }
		case IDC_STT_RACK6L2_CH10: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_10);	break; }
		case IDC_STT_RACK6L2_CH11: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_11);	break; }
		case IDC_STT_RACK6L2_CH12: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_12);	break; }
		case IDC_STT_RACK6L2_CH13: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_13);	break; }
		case IDC_STT_RACK6L2_CH14: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_14);	break; }
		case IDC_STT_RACK6L2_CH15: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_15);	break; }
		case IDC_STT_RACK6L2_CH16: {Lf_toggleChUseUnuse(RACK_6, LAYER_2, CH_16);	break; }
		case IDC_STT_RACK6L3_CH1: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_1);	break; }
		case IDC_STT_RACK6L3_CH2: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_2);	break; }
		case IDC_STT_RACK6L3_CH3: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_3);	break; }
		case IDC_STT_RACK6L3_CH4: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_4);	break; }
		case IDC_STT_RACK6L3_CH5: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_5);	break; }
		case IDC_STT_RACK6L3_CH6: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_6);	break; }
		case IDC_STT_RACK6L3_CH7: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_7);	break; }
		case IDC_STT_RACK6L3_CH8: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_8);	break; }
		case IDC_STT_RACK6L3_CH9: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_9);	break; }
		case IDC_STT_RACK6L3_CH10: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_10);	break; }
		case IDC_STT_RACK6L3_CH11: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_11);	break; }
		case IDC_STT_RACK6L3_CH12: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_12);	break; }
		case IDC_STT_RACK6L3_CH13: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_13);	break; }
		case IDC_STT_RACK6L3_CH14: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_14);	break; }
		case IDC_STT_RACK6L3_CH15: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_15);	break; }
		case IDC_STT_RACK6L3_CH16: {Lf_toggleChUseUnuse(RACK_6, LAYER_3, CH_16);	break; }
		case IDC_STT_RACK6L4_CH1: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_1);	break; }
		case IDC_STT_RACK6L4_CH2: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_2);	break; }
		case IDC_STT_RACK6L4_CH3: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_3);	break; }
		case IDC_STT_RACK6L4_CH4: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_4);	break; }
		case IDC_STT_RACK6L4_CH5: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_5);	break; }
		case IDC_STT_RACK6L4_CH6: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_6);	break; }
		case IDC_STT_RACK6L4_CH7: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_7);	break; }
		case IDC_STT_RACK6L4_CH8: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_8);	break; }
		case IDC_STT_RACK6L4_CH9: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_9);	break; }
		case IDC_STT_RACK6L4_CH10: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_10);	break; }
		case IDC_STT_RACK6L4_CH11: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_11);	break; }
		case IDC_STT_RACK6L4_CH12: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_12);	break; }
		case IDC_STT_RACK6L4_CH13: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_13);	break; }
		case IDC_STT_RACK6L4_CH14: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_14);	break; }
		case IDC_STT_RACK6L4_CH15: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_15);	break; }
		case IDC_STT_RACK6L4_CH16: {Lf_toggleChUseUnuse(RACK_6, LAYER_4, CH_16);	break; }
		case IDC_STT_RACK6L5_CH1: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_1);	break; }
		case IDC_STT_RACK6L5_CH2: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_2);	break; }
		case IDC_STT_RACK6L5_CH3: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_3);	break; }
		case IDC_STT_RACK6L5_CH4: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_4);	break; }
		case IDC_STT_RACK6L5_CH5: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_5);	break; }
		case IDC_STT_RACK6L5_CH6: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_6);	break; }
		case IDC_STT_RACK6L5_CH7: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_7);	break; }
		case IDC_STT_RACK6L5_CH8: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_8);	break; }
		case IDC_STT_RACK6L5_CH9: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_9);	break; }
		case IDC_STT_RACK6L5_CH10: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_10);	break; }
		case IDC_STT_RACK6L5_CH11: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_11);	break; }
		case IDC_STT_RACK6L5_CH12: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_12);	break; }
		case IDC_STT_RACK6L5_CH13: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_13);	break; }
		case IDC_STT_RACK6L5_CH14: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_14);	break; }
		case IDC_STT_RACK6L5_CH15: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_15);	break; }
		case IDC_STT_RACK6L5_CH16: {Lf_toggleChUseUnuse(RACK_6, LAYER_5, CH_16);	break; }
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void CHseAgingDlg::Lf_InitLocalValue()
{
	//////////////////////////////////////////////////////////////////////////////////////
	// Local 변수 초기화
	//////////////////////////////////////////////////////////////////////////////////////
	m_nMeasureTick = 0;
	m_nAgingStatusTick = 0;
	m_nPowerMeasureTick = 0;
	m_nSensinglogTick = 0;
	m_nFirmwareTick = 0;
	m_nFirmwareTick_d = 0.1;
	memset(m_nAgnOutFlag, 0, sizeof(m_nAgnOutFlag));
	memset(m_nAgingStart, 0, sizeof(m_nAgingStart));

	if (lpInspWorkInfo->m_nConnectInfo[CONNECT_MES] == TRUE)
	{
		GetDlgItem(IDC_STT_CONNECT_INFO_MES)->SetWindowText(_T("[MES]ON-LINE"));
	}
	else
	{
		GetDlgItem(IDC_STT_CONNECT_INFO_MES)->SetWindowText(_T("[MES]OFF-LINE"));
	}
	GetDlgItem(IDC_STT_MES_USER_ID)->SetWindowText(m_pApp->m_sUserID);
	GetDlgItem(IDC_STT_MES_USER_NAME)->SetWindowText(m_pApp->m_sUserName);

	//////////////////////////////////////////////////////////////////////////////////////
	// Control ID 초기화 및 변수 할당
	//////////////////////////////////////////////////////////////////////////////////////
	int i = 0, rack = 0, ly = 0, ch = 0;

	m_pDoorState[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_DOOR1);
	m_pDoorState[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_DOOR2);
	m_pDoorState[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_DOOR3);
	m_pDoorState[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_DOOR4);
	m_pDoorState[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_DOOR5);
	m_pDoorState[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_DOOR6);

	i = 0;
	m_pSttRackNo[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RACK1);
	m_pSttRackNo[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RACK2);
	m_pSttRackNo[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RACK3);
	m_pSttRackNo[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RACK4);
	m_pSttRackNo[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RACK5);
	m_pSttRackNo[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RACK6);

	i = 0;
	m_pCmbMaModel[i++] = (CComboBox*)GetDlgItem(IDC_CMB_MA_MODEL_RACK1);
	m_pCmbMaModel[i++] = (CComboBox*)GetDlgItem(IDC_CMB_MA_MODEL_RACK2);
	m_pCmbMaModel[i++] = (CComboBox*)GetDlgItem(IDC_CMB_MA_MODEL_RACK3);
	m_pCmbMaModel[i++] = (CComboBox*)GetDlgItem(IDC_CMB_MA_MODEL_RACK4);
	m_pCmbMaModel[i++] = (CComboBox*)GetDlgItem(IDC_CMB_MA_MODEL_RACK5);
	m_pCmbMaModel[i++] = (CComboBox*)GetDlgItem(IDC_CMB_MA_MODEL_RACK6);

	i = 0;
	m_pBtnAgingStart[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_START_RACK1);
	m_pBtnAgingStart[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_START_RACK2);
	m_pBtnAgingStart[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_START_RACK3);
	m_pBtnAgingStart[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_START_RACK4);
	m_pBtnAgingStart[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_START_RACK5);
	m_pBtnAgingStart[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_START_RACK6);

	i = 0;
	m_pBtnAgingStop[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_STOP_RACK1);
	m_pBtnAgingStop[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_STOP_RACK2);
	m_pBtnAgingStop[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_STOP_RACK3);
	m_pBtnAgingStop[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_STOP_RACK4);
	m_pBtnAgingStop[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_STOP_RACK5);
	m_pBtnAgingStop[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_STOP_RACK6);

	i = 0;
	m_pBtnAgingFusing[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_FUSING_RACK1);
	m_pBtnAgingFusing[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_FUSING_RACK2);
	m_pBtnAgingFusing[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_FUSING_RACK3);
	m_pBtnAgingFusing[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_FUSING_RACK4);
	m_pBtnAgingFusing[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_FUSING_RACK5);
	m_pBtnAgingFusing[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_FUSING_RACK6);

	i = 0;
	m_pBtnChUseUnuseSet[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_CH_SET_RACK1);
	m_pBtnChUseUnuseSet[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_CH_SET_RACK2);
	m_pBtnChUseUnuseSet[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_CH_SET_RACK3);
	m_pBtnChUseUnuseSet[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_CH_SET_RACK4);
	m_pBtnChUseUnuseSet[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_CH_SET_RACK5);
	m_pBtnChUseUnuseSet[i++] = (CMFCButton*)GetDlgItem(IDC_MBC_MA_CH_SET_RACK6);

	i = 0;
	m_pCtrMaProgress[i++] = (CProgressCtrl*)GetDlgItem(IDC_CTR_MA_PROGRESS_RACK1);
	m_pCtrMaProgress[i++] = (CProgressCtrl*)GetDlgItem(IDC_CTR_MA_PROGRESS_RACK2);
	m_pCtrMaProgress[i++] = (CProgressCtrl*)GetDlgItem(IDC_CTR_MA_PROGRESS_RACK3);
	m_pCtrMaProgress[i++] = (CProgressCtrl*)GetDlgItem(IDC_CTR_MA_PROGRESS_RACK4);
	m_pCtrMaProgress[i++] = (CProgressCtrl*)GetDlgItem(IDC_CTR_MA_PROGRESS_RACK5);
	m_pCtrMaProgress[i++] = (CProgressCtrl*)GetDlgItem(IDC_CTR_MA_PROGRESS_RACK6);

	i = 0;
	m_pSttMaSetTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_SET_TIME_RACK1);
	m_pSttMaSetTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_SET_TIME_RACK2);
	m_pSttMaSetTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_SET_TIME_RACK3);
	m_pSttMaSetTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_SET_TIME_RACK4);
	m_pSttMaSetTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_SET_TIME_RACK5);
	m_pSttMaSetTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_SET_TIME_RACK6);

	i = 0;
	m_pSttMaRunTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RUN_TIME_RACK1);
	m_pSttMaRunTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RUN_TIME_RACK2);
	m_pSttMaRunTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RUN_TIME_RACK3);
	m_pSttMaRunTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RUN_TIME_RACK4);
	m_pSttMaRunTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RUN_TIME_RACK5);
	m_pSttMaRunTime[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_RUN_TIME_RACK6);

	i = 0;
	m_pLstMLog[i++] = (CListBox*)GetDlgItem(IDC_LST_MA_MLOG_RACK1);
	m_pLstMLog[i++] = (CListBox*)GetDlgItem(IDC_LST_MA_MLOG_RACK2);
	m_pLstMLog[i++] = (CListBox*)GetDlgItem(IDC_LST_MA_MLOG_RACK3);
	m_pLstMLog[i++] = (CListBox*)GetDlgItem(IDC_LST_MA_MLOG_RACK4);
	m_pLstMLog[i++] = (CListBox*)GetDlgItem(IDC_LST_MA_MLOG_RACK5);
	m_pLstMLog[i++] = (CListBox*)GetDlgItem(IDC_LST_MA_MLOG_RACK6);

	rack = RACK_1;
	ly = LAYER_1;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L1_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L1_CH16;


	rack = RACK_1;
	ly = LAYER_2;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L2_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L2_CH16;

	rack = RACK_1;
	ly = LAYER_3;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L3_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L3_CH16;

	rack = RACK_1;
	ly = LAYER_4;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L4_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L4_CH16;

	rack = RACK_1;
	ly = LAYER_5;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK1L5_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK1L5_CH16;

	rack = RACK_2;
	ly = LAYER_1;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L1_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L1_CH16;

	rack = RACK_2;
	ly = LAYER_2;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L2_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L2_CH16;

	rack = RACK_2;
	ly = LAYER_3;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L3_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L3_CH16;

	rack = RACK_2;
	ly = LAYER_4;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L4_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L4_CH16;

	rack = RACK_2;
	ly = LAYER_5;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK2L5_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK2L5_CH16;

	rack = RACK_3;
	ly = LAYER_1;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L1_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L1_CH16;

	rack = RACK_3;
	ly = LAYER_2;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L2_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L2_CH16;

	rack = RACK_3;
	ly = LAYER_3;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L3_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L3_CH16;

	rack = RACK_3;
	ly = LAYER_4;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L4_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L4_CH16;

	rack = RACK_3;
	ly = LAYER_5;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK3L5_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK3L5_CH16;

	rack = RACK_4;
	ly = LAYER_1;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L1_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L1_CH16;

	rack = RACK_4;
	ly = LAYER_2;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L2_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L2_CH16;


	rack = RACK_4;
	ly = LAYER_3;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L3_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L3_CH16;

	rack = RACK_4;
	ly = LAYER_4;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L4_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L4_CH16;

	rack = RACK_4;
	ly = LAYER_5;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK4L5_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK4L5_CH16;

	rack = RACK_5;
	ly = LAYER_1;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L1_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L1_CH16;

	rack = RACK_5;
	ly = LAYER_2;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L2_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L2_CH16;

	rack = RACK_5;
	ly = LAYER_3;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L3_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L3_CH16;

	rack = RACK_5;
	ly = LAYER_4;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L4_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L4_CH16;

	rack = RACK_5;
	ly = LAYER_5;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK5L5_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK5L5_CH16;

	rack = RACK_6;
	ly = LAYER_1;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L1_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L1_CH16;

	rack = RACK_6;
	ly = LAYER_2;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L2_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L2_CH16;

	rack = RACK_6;
	ly = LAYER_3;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L3_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L3_CH16;

	rack = RACK_6;
	ly = LAYER_4;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L4_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L4_CH16;

	rack = RACK_6;
	ly = LAYER_5;
	ch = 0;
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH1);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH2);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH3);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH4);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH5);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH6);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH7);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH8);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH9);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH10);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH11);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH12);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH13);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH14);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH15);
	m_pSttRackState[rack][ly][ch++] = (CStatic*)GetDlgItem(IDC_STT_RACK6L5_CH16);
	ch = 0;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH1;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH2;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH3;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH4;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH5;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH6;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH7;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH8;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH9;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH10;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH11;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH12;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH13;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH14;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH15;
	m_pSttRackCtrlID[rack][ly][ch++] = IDC_STT_RACK6L5_CH16;

	i = 0;
	m_pSttTempInfo[i++] = (CStatic*)GetDlgItem(IDC_STT_TEMP_SENSOR1_V);
	m_pSttTempInfo[i++] = (CStatic*)GetDlgItem(IDC_STT_TEMP_SENSOR2_V);
	m_pSttTempInfo[i++] = (CStatic*)GetDlgItem(IDC_STT_TEMP_SENSOR3_V);
	m_pSttTempInfo[i++] = (CStatic*)GetDlgItem(IDC_STT_TEMP_SENSOR4_V);
	m_pSttTempInfo[i++] = (CStatic*)GetDlgItem(IDC_STT_TEMP_SENSOR5_V);
	m_pSttTempInfo[i++] = (CStatic*)GetDlgItem(IDC_STT_TEMP_SENSOR6_V);

	i = 0;
	m_pChkChSelect[i++] = (CButton*)GetDlgItem(IDC_CHK_MA_SELECT_RACK1);
	m_pChkChSelect[i++] = (CButton*)GetDlgItem(IDC_CHK_MA_SELECT_RACK2);
	m_pChkChSelect[i++] = (CButton*)GetDlgItem(IDC_CHK_MA_SELECT_RACK3);
	m_pChkChSelect[i++] = (CButton*)GetDlgItem(IDC_CHK_MA_SELECT_RACK4);
	m_pChkChSelect[i++] = (CButton*)GetDlgItem(IDC_CHK_MA_SELECT_RACK5);
	m_pChkChSelect[i++] = (CButton*)GetDlgItem(IDC_CHK_MA_SELECT_RACK6);

	i = 0;
	m_pSttFWVersion[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_FW_VER_RACK1);
	m_pSttFWVersion[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_FW_VER_RACK2);
	m_pSttFWVersion[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_FW_VER_RACK3);
	m_pSttFWVersion[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_FW_VER_RACK4);
	m_pSttFWVersion[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_FW_VER_RACK5);
	m_pSttFWVersion[i++] = (CStatic*)GetDlgItem(IDC_STT_MA_FW_VER_RACK6);
}

void CHseAgingDlg::Lf_InitFontset()
{
	m_Font[0].CreateFont(150, 70, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[1].CreateFont(80, 35, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[2].CreateFont(50, 20, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[3].CreateFont(23, 10, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_MA_CHAMBER_NO)->SetFont(&m_Font[3]);
	GetDlgItem(IDC_MBT_MA_BUZZ_OFF)->SetFont(&m_Font[3]);

	m_Font[4].CreateFont(21, 9, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);

	m_Font[5].CreateFont(19, 8, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_MA_SET_TIME_RACK1)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_SET_TIME_RACK2)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_SET_TIME_RACK3)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_SET_TIME_RACK4)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_SET_TIME_RACK5)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_SET_TIME_RACK6)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_RUN_TIME_RACK1)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_RUN_TIME_RACK2)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_RUN_TIME_RACK3)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_RUN_TIME_RACK4)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_RUN_TIME_RACK5)->SetFont(&m_Font[5]);
	GetDlgItem(IDC_STT_MA_RUN_TIME_RACK6)->SetFont(&m_Font[5]);


	m_Font[6].CreateFont(17, 7, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_STT_MA_RACK1)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_RACK2)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_RACK3)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_RACK4)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_RACK5)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_RACK6)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_TEMP_SENSOR1_V)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_TEMP_SENSOR2_V)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_TEMP_SENSOR3_V)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_TEMP_SENSOR4_V)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_TEMP_SENSOR5_V)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_TEMP_SENSOR6_V)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_DOOR1)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_DOOR2)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_DOOR3)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_DOOR4)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_DOOR5)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_MA_DOOR6)->SetFont(&m_Font[6]);

	GetDlgItem(IDC_STT_CONNECT_INFO_PG)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_CONNECT_INFO_DIO)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_CONNECT_INFO_TEMP)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_CONNECT_INFO_BARCODE)->SetFont(&m_Font[6]);
	GetDlgItem(IDC_STT_CONNECT_INFO_MES)->SetFont(&m_Font[6]);


	m_Font[7].CreateFont(15, 6, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	GetDlgItem(IDC_LST_MA_MLOG_RACK1)->SetFont(&m_Font[7]);
	GetDlgItem(IDC_LST_MA_MLOG_RACK2)->SetFont(&m_Font[7]);
	GetDlgItem(IDC_LST_MA_MLOG_RACK3)->SetFont(&m_Font[7]);
	GetDlgItem(IDC_LST_MA_MLOG_RACK4)->SetFont(&m_Font[7]);
	GetDlgItem(IDC_LST_MA_MLOG_RACK5)->SetFont(&m_Font[7]);
	GetDlgItem(IDC_LST_MA_MLOG_RACK6)->SetFont(&m_Font[7]);

	m_Font[8].CreateFont(13, 5, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);


	m_Font[9].CreateFont(11, 4, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_FONT);
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		for (int layer = 0; layer < MAX_LAYER; layer++)
		{
			for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
			{
				m_pSttRackState[rack][layer][ch]->SetFont(&m_Font[9]);
			}
		}
	}
}

void CHseAgingDlg::Lf_InitColorBrush()
{
	m_Brush[COLOR_IDX_USER_BACKGROUND].CreateSolidBrush(COLOR_USER_BACKGROUND);
	m_Brush[COLOR_IDX_BLACK].CreateSolidBrush(COLOR_BLACK);
	m_Brush[COLOR_IDX_WHITE].CreateSolidBrush(COLOR_WHITE);
	m_Brush[COLOR_IDX_RED].CreateSolidBrush(COLOR_RED);
	m_Brush[COLOR_IDX_RED128].CreateSolidBrush(COLOR_RED128);
	m_Brush[COLOR_IDX_GREEN].CreateSolidBrush(COLOR_GREEN);
	m_Brush[COLOR_IDX_GREEN128].CreateSolidBrush(COLOR_GREEN128);
	m_Brush[COLOR_IDX_BLUE].CreateSolidBrush(COLOR_BLUE);
	m_Brush[COLOR_IDX_BLUE128].CreateSolidBrush(COLOR_BLUE128);
	m_Brush[COLOR_IDX_ORANGE].CreateSolidBrush(COLOR_ORANGE);
	m_Brush[COLOR_IDX_YELLOW].CreateSolidBrush(COLOR_YELLOW);
	m_Brush[COLOR_IDX_MAGENTA].CreateSolidBrush(COLOR_MAGENTA);
	m_Brush[COLOR_IDX_VERDANT2].CreateSolidBrush(COLOR_VERDANT2);
	m_Brush[COLOR_IDX_SKYBLUE].CreateSolidBrush(COLOR_SKYBLUE);
	m_Brush[COLOR_IDX_JADEBLUE].CreateSolidBrush(COLOR_JADEBLUE);
	m_Brush[COLOR_IDX_JADEGREEN].CreateSolidBrush(COLOR_JADEGREEN);
	m_Brush[COLOR_IDX_BLUISH].CreateSolidBrush(COLOR_BLUISH);
	m_Brush[COLOR_IDX_PURPLE].CreateSolidBrush(COLOR_PURPLE);
	m_Brush[COLOR_IDX_LIGHT_GREEN].CreateSolidBrush(COLOR_LIGHT_GREEN);
	m_Brush[COLOR_IDX_LIGHT_RED].CreateSolidBrush(COLOR_LIGHT_RED);
	m_Brush[COLOR_IDX_LIGHT_YELLOW].CreateSolidBrush(COLOR_LIGHT_YELLOW);
	m_Brush[COLOR_IDX_LIGHT_ORANGE].CreateSolidBrush(COLOR_LIGHT_ORANGE);
	m_Brush[COLOR_IDX_DARK_RED].CreateSolidBrush(COLOR_DARK_RED);
	m_Brush[COLOR_IDX_DARK_GREEN].CreateSolidBrush(COLOR_DARK_GREEN);
	m_Brush[COLOR_IDX_DARK_BLUE].CreateSolidBrush(COLOR_DARK_BLUE);
	m_Brush[COLOR_IDX_DARK_MAGENTA].CreateSolidBrush(COLOR_DARK_MAGENTA);
	m_Brush[COLOR_IDX_DARK_ORANGE].CreateSolidBrush(COLOR_DARK_ORANGE);
	m_Brush[COLOR_IDX_DARK_YELLOW].CreateSolidBrush(COLOR_DARK_YELLOW);
	m_Brush[COLOR_IDX_GRAY96].CreateSolidBrush(COLOR_GRAY96);
	m_Brush[COLOR_IDX_GRAY128].CreateSolidBrush(COLOR_GRAY128);
	m_Brush[COLOR_IDX_GRAY159].CreateSolidBrush(COLOR_GRAY159);
	m_Brush[COLOR_IDX_GRAY192].CreateSolidBrush(COLOR_GRAY192);
	m_Brush[COLOR_IDX_GRAY224].CreateSolidBrush(COLOR_GRAY224);
	m_Brush[COLOR_IDX_GRAY240].CreateSolidBrush(COLOR_GRAY240);
}


void CHseAgingDlg::Lf_InitButtonIcon()
{
	m_btnIconUser.LoadBitmaps(IDB_BMP_USER, IDB_BMP_USER, IDB_BMP_USER, IDB_BMP_USER);
	m_btnIconUser.SizeToContent();
	m_btnIconMonitoring.LoadBitmaps(IDB_BMP_MONITORING, IDB_BMP_MONITORING, IDB_BMP_MONITORING, IDB_BMP_MONITORING);
	m_btnIconMonitoring.SizeToContent();
	m_btnIconModel.LoadBitmaps(IDB_BMP_MODEL, IDB_BMP_MODEL, IDB_BMP_MODEL, IDB_BMP_MODEL);
	m_btnIconModel.SizeToContent();
	m_btnIconPIDInput.LoadBitmaps(IDB_BMP_PID_INPUT, IDB_BMP_PID_INPUT, IDB_BMP_PID_INPUT, IDB_BMP_PID_INPUT);
	m_btnIconPIDInput.SizeToContent();
	m_btnIconFirmware.LoadBitmaps(IDB_BMP_FIRMWARE, IDB_BMP_FIRMWARE, IDB_BMP_FIRMWARE, IDB_BMP_FIRMWARE);
	m_btnIconFirmware.SizeToContent();
	m_btnIconSystem.LoadBitmaps(IDB_BMP_SYSTEM, IDB_BMP_SYSTEM, IDB_BMP_SYSTEM, IDB_BMP_SYSTEM);
	m_btnIconSystem.SizeToContent();
	m_btnIconExit.LoadBitmaps(IDB_BMP_EXIT, IDB_BMP_EXIT, IDB_BMP_EXIT, IDB_BMP_EXIT);
	m_btnIconExit.SizeToContent();
}

void CHseAgingDlg::Lf_InitDialogDesign()
{
	// X Button 비활성화
	CMenu* p_menu = this->GetSystemMenu(FALSE);
	p_menu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);

	SetWindowTheme(GetDlgItem(IDC_GRP_MA_CHAMBER_NO)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_GRP_MA_DESCRIPTION)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_GRP_MA_FW_VERSION)->GetSafeHwnd(), L"", L"");

	SetWindowTheme(GetDlgItem(IDC_CHK_MA_SELECT_RACK1)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_MA_SELECT_RACK2)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_MA_SELECT_RACK3)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_MA_SELECT_RACK4)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_MA_SELECT_RACK5)->GetSafeHwnd(), L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHK_MA_SELECT_RACK6)->GetSafeHwnd(), L"", L"");

	m_mbtBuzzOff.EnableWindowsTheming(FALSE);
	m_mbtBuzzOff.SetFaceColor(RGB(236,28,66));
	m_mbtBuzzOff.SetTextColor(COLOR_WHITE);

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		m_pBtnAgingStart[rack]->EnableWindowsTheming(FALSE);
		m_pBtnAgingStart[rack]->SetFaceColor(COLOR_VERDANT2);
		m_pBtnAgingStart[rack]->SetTextColor(COLOR_BLACK);

		m_pBtnAgingStop[rack]->EnableWindowsTheming(FALSE);
		m_pBtnAgingStop[rack]->SetFaceColor(COLOR_LIGHT_RED);
		m_pBtnAgingStop[rack]->SetTextColor(COLOR_BLACK);

		m_pBtnAgingFusing[rack]->EnableWindowsTheming(FALSE);
		m_pBtnAgingFusing[rack]->SetFaceColor(COLOR_LIGHT_BLUE);
		m_pBtnAgingFusing[rack]->SetTextColor(COLOR_BLACK);

		m_pBtnChUseUnuseSet[rack]->EnableWindowsTheming(FALSE);
		m_pBtnChUseUnuseSet[rack]->SetFaceColor(COLOR_LIGHT_RED);
		m_pBtnChUseUnuseSet[rack]->SetTextColor(COLOR_BLACK);

		CRect rect_move, rect_curr;
		m_pBtnAgingStart[rack]->GetWindowRect(rect_move);
		m_pBtnChUseUnuseSet[rack]->GetWindowRect(rect_curr);

		rect_curr.left = rect_move.left - 9;
		rect_curr.right = rect_curr.right - 9;
		rect_curr.top = rect_move.top - 31;
		rect_curr.bottom = rect_move.bottom - 31;
		m_pBtnChUseUnuseSet[rack]->MoveWindow(rect_curr);
	}



	int nWidth = 50;
	m_pApp->Gf_setGradientStatic01(&m_sttTempSensor, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic01(&m_sttConnectInfo, &m_Font[6], FALSE);

	m_pApp->Gf_setGradientStatic02(&m_sttMaMLogRack1, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic02(&m_sttMaMLogRack2, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic02(&m_sttMaMLogRack3, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic02(&m_sttMaMLogRack4, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic02(&m_sttMaMLogRack5, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic02(&m_sttMaMLogRack6, &m_Font[6], FALSE);

	m_pApp->Gf_setGradientStatic04(&m_sttTempSensor1T, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic04(&m_sttTempSensor2T, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic04(&m_sttTempSensor3T, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic04(&m_sttTempSensor4T, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic04(&m_sttTempSensor5T, &m_Font[6], FALSE);
	m_pApp->Gf_setGradientStatic04(&m_sttTempSensor6T, &m_Font[6], FALSE);
}

void CHseAgingDlg::Lf_InitCobmoRackModelList()
{
	CString strfilename = _T("");
	CString strfilepath = _T("");
	WIN32_FIND_DATA wfd;
	HANDLE hSearch;

	strfilepath.Format(_T("./Model/*.ini"));
	hSearch = FindFirstFile(strfilepath, &wfd);

	m_cmbMaModelRack1.ResetContent();
	m_cmbMaModelRack2.ResetContent();
	m_cmbMaModelRack3.ResetContent();
	m_cmbMaModelRack4.ResetContent();
	m_cmbMaModelRack5.ResetContent();
	m_cmbMaModelRack6.ResetContent();
	m_cmbMaModelRack1.AddString(_T("- MODEL LIST -"));
	m_cmbMaModelRack2.AddString(_T("- MODEL LIST -"));
	m_cmbMaModelRack3.AddString(_T("- MODEL LIST -"));
	m_cmbMaModelRack4.AddString(_T("- MODEL LIST -"));
	m_cmbMaModelRack5.AddString(_T("- MODEL LIST -"));
	m_cmbMaModelRack6.AddString(_T("- MODEL LIST -"));
	if (hSearch != INVALID_HANDLE_VALUE)
	{
		if (wfd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			strfilename.Format(_T("%s"), wfd.cFileName);
			strfilename = strfilename.Mid(0, strfilename.GetLength() - 4);
			strfilename.MakeUpper();
			m_cmbMaModelRack1.AddString(strfilename);
			m_cmbMaModelRack2.AddString(strfilename);
			m_cmbMaModelRack3.AddString(strfilename);
			m_cmbMaModelRack4.AddString(strfilename);
			m_cmbMaModelRack5.AddString(strfilename);
			m_cmbMaModelRack6.AddString(strfilename);
		}
		while (FindNextFile(hSearch, &wfd))
		{
			if (wfd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
			{
				strfilename.Format(_T("%s"), wfd.cFileName);
				strfilename = strfilename.Mid(0, strfilename.GetLength() - 4);
				strfilename.MakeUpper();
				m_cmbMaModelRack1.AddString(strfilename);
				m_cmbMaModelRack2.AddString(strfilename);
				m_cmbMaModelRack3.AddString(strfilename);
				m_cmbMaModelRack4.AddString(strfilename);
				m_cmbMaModelRack5.AddString(strfilename);
				m_cmbMaModelRack6.AddString(strfilename);
			}
		}
		FindClose(hSearch);
	}

	m_cmbMaModelRack1.SetCurSel(0);
	m_cmbMaModelRack2.SetCurSel(0);
	m_cmbMaModelRack3.SetCurSel(0);
	m_cmbMaModelRack4.SetCurSel(0);
	m_cmbMaModelRack5.SetCurSel(0);
	m_cmbMaModelRack6.SetCurSel(0);
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		for (int i = 0; i < m_pCmbMaModel[rack]->GetCount(); i++)
		{
			CString selModelName;
			selModelName = lpSystemInfo->m_sLastModelName[rack];
			m_pCmbMaModel[rack]->GetLBText(i, strfilename);
			if (strfilename == selModelName)
			{
				m_pCmbMaModel[rack]->SetCurSel(i);
				break;
			}
		}
	}
}

void CHseAgingDlg::Lf_setDoorOnOff(int rack)
{
	CString strMsg;

	if (lpInspWorkInfo->m_nDoorOpenClose[rack] == DOOR_CLOSE)
	{
		lpInspWorkInfo->m_nDoorOpenClose[rack] = DOOR_OPEN;
		//lpInspWorkInfo->m_nDoorOpenClose[rack] = DOOR_OPEN;
		if (lpInspWorkInfo->m_nDoorOpenClose[rack] == DOOR_OPEN)
			strMsg.Format(_T("DOOR%d OPEN"), rack + 1);
		else
			strMsg.Format(_T("DOOR%d CLOSE"), rack + 1);

		m_pDoorState[rack]->SetWindowText(strMsg);

	}
	else if (lpInspWorkInfo->m_nDoorOpenClose[rack] == 16843009)
	{
		lpInspWorkInfo->m_nDoorOpenClose[rack] = DOOR_OPEN;
		//lpInspWorkInfo->m_nDoorOpenClose[rack] = DOOR_OPEN;
		if (lpInspWorkInfo->m_nDoorOpenClose[rack] == DOOR_OPEN)
			strMsg.Format(_T("DOOR%d OPEN"), rack + 1);
		else
			strMsg.Format(_T("DOOR%d CLOSE"), rack + 1);

		m_pDoorState[rack]->SetWindowText(strMsg);
	}
	else
	{
		//memset(lpInspWorkInfo->m_nDoorOpenClose, 1, sizeof(lpInspWorkInfo->m_nDoorOpenClose));
		//strMsg.Format(_T("DOOR%d CLOSE"), rack + 1);
		lpInspWorkInfo->m_nDoorOpenClose[rack] = DOOR_CLOSE;
		if (lpInspWorkInfo->m_nDoorOpenClose[rack] == DOOR_OPEN)
			strMsg.Format(_T("DOOR%d OPEN"), rack + 1);
		else
			strMsg.Format(_T("DOOR%d CLOSE"), rack + 1);

		m_pDoorState[rack]->SetWindowText(strMsg);
	}

	//lpInspWorkInfo->m_ast_AgingChErrorResult[0][1][0] = LIMIT_HIGH;
	

	//memset(lpInspWorkInfo->m_nDoorOpenClose, 1, sizeof(lpInspWorkInfo->m_nDoorOpenClose));
}

void CHseAgingDlg::Lf_setAgingSTART_PID(int rack, int ch)
{
	m_pApp->pCommand->Gf_setPowerSequenceOnOff_BCR_IP(rack, POWER_ON,1,ch,1);
	lpInspWorkInfo->m_StopRackID.Format(_T("%d"), rack);
	lpInspWorkInfo->m_PidFlag = true;
}

void CHseAgingDlg::Lf_setAgingSTOP_PID(int rack)
{
	//m_pApp->pCommand->Gf_setPowerSequenceOnOff(rack, POWER_OFF);
	m_pApp->pCommand->Gf_setPowerSequenceOnOff_BCR(rack, POWER_OFF,1,0, 1);
}

void CHseAgingDlg::Lf_setChannelUseUnuse_PID_ON(int rack, int Ch)
{
	CString ip;
	BOOL setInfo[16];

	m_pBtnChUseUnuseSet[rack]->EnableWindow(FALSE);

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			// ch == 7일 때만 Use, 나머지는 Unuse (0-based index)
			setInfo[ch] = (ch == Ch) ? CHANNEL_USE : CHANNEL_UNUSE;

			// 옵션: 내부 상태도 같이 변경 (lpInspWorkInfo도 반영)
			lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] = setInfo[ch];
		}

		ip.Format(_T("192.168.10.%d"), (rack * 5) + layer + 1);
		m_pApp->pCommand->Gf_setChannelUseUnuse(ip, setInfo);
	}

	m_pBtnChUseUnuseSet[rack]->EnableWindow(TRUE);
}

void CHseAgingDlg::Lf_setChannelUseUnuse_PID_OFF(int rack)
{
	CString ip;
	BOOL setInfo[16];

	m_pBtnChUseUnuseSet[rack]->EnableWindow(FALSE);

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			// 모든 채널을 사용 상태로 설정
			setInfo[ch] = CHANNEL_USE;

			// 내부 상태도 같이 반영
			lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] = CHANNEL_USE;
		}

		ip.Format(_T("192.168.10.%d"), (rack * 5) + layer + 1);
		m_pApp->pCommand->Gf_setChannelUseUnuse(ip, setInfo);
	}

	m_pBtnChUseUnuseSet[rack]->EnableWindow(TRUE);
}



void CHseAgingDlg::Lf_setAgingSTART(int rack) 
{
	if (m_pCmbMaModel[rack]->GetCurSel() == 0)
	{
		m_pApp->Gf_ShowMessageBox(_T("No model was selected. Please select a model."));
		return;
	}

	CMessageQuestion msg_dlg;

	CString sModelName, sdata, sLog, sTimeOut;

	sLog.Format(_T("<MESSAGE> AGING START CLICK [RACK %d]"), rack + 1);
	m_pApp->Gf_writeMLog(sLog);

	bTempErrorOnce[rack] = FALSE;

	lpInspWorkInfo->m_PidTestError[rack] = false; // PID, AGING START ERROR MESSAGE FLAG

	// Button Disable
	m_pBtnAgingStart[rack]->EnableWindow(FALSE);
	m_pBtnAgingFusing[rack]->EnableWindow(FALSE);
	m_pChkChSelect[rack]->EnableWindow(FALSE);

	// RACK 선택된 모델명을 가져온다
	m_pCmbMaModel[rack]->GetWindowText(sModelName);

	// CurModel.ini에 기록
	SaveCurModelIni_FromModelText(sModelName);

	m_pApp->Gf_loadModelData(sModelName);

	sTimeOut.Format(_T("LAST_TIMEOUT_RACK%d"), (rack + 1));
	Read_SysIniFile(_T("SYSTEM"), sTimeOut, &lpSystemInfo->m_sLastTimeOut[rack]);

	m_pCmbMaModel[rack]->GetWindowText(sModelName);
	m_pApp->Gf_loadModelData(sModelName);
	Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("AGING_TIME_MINUTE"), &lpInspWorkInfo->m_nAgingSetTime[rack]);

	m_pApp->pCimNet->r_strmodelName = sModelName;

	//Lf_rmsErcpSet();

	/*if (lpSystemInfo->m_sLastTimeOut[rack] != 0 && lpSystemInfo->m_sLastTimeOut[rack] <= 240)*/
	if (lpSystemInfo->m_sLastTimeOut[rack] != 0 && lpInspWorkInfo->m_nAgingSetTime[rack] >= lpSystemInfo->m_sLastTimeOut[rack])
	{
		msg_dlg.m_strQMessage.Format(_T("Aging Continue ? [%d]Minute"), lpSystemInfo->m_sLastTimeOut[rack]);
		if (msg_dlg.DoModal() == IDOK)
		{
			CString Msg;
			Msg.Format(_T("Start From [%d]Minute"), lpSystemInfo->m_sLastTimeOut[rack]);
			Lf_writeRackMLog(rack, Msg);
			lpInspWorkInfo->m_nAgingRunTime[rack] = lpSystemInfo->m_sLastTimeOut[rack];
		}
		else
		{
			lpInspWorkInfo->m_nAgingRunTime[rack] = 0;
			sTimeOut.Format(_T("LAST_TIMEOUT_RACK%d"), rack + 1);
			Write_SysIniFile(_T("SYSTEM"), sLog, lpInspWorkInfo->m_nAgingRunTime[rack]);
			lpSystemInfo->m_sLastTimeOut[rack] = 0;
		}
	}
	else
	{
		lpInspWorkInfo->m_nAgingRunTime[rack] = 0;
		lpSystemInfo->m_sLastTimeOut[rack] = 0;
	}

	// Cable Open Check 진행한다.
	if (Lf_checkCableOpen(rack) == FALSE)
	{
		// Button Disable
		m_pBtnAgingStart[rack]->EnableWindow(TRUE);
		m_pBtnAgingFusing[rack]->EnableWindow(TRUE);
		m_pChkChSelect[rack]->EnableWindow(TRUE);

		return;
	}

	// Aging START 및 Panel Power ON
	m_pApp->pCommand->Gf_setAgingSTART(rack);
	m_pApp->pCommand->Gf_setPowerSequenceOnOff(rack, POWER_ON);

	// 모델명에서 Aging Set Time 정보를 Read 한다.
	/*m_pCmbMaModel[rack]->GetWindowText(sModelName);
	m_pApp->Gf_loadModelData(sModelName);
	Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("AGING_TIME_MINUTE"), &lpInspWorkInfo->m_nAgingSetTime[rack]);*/

	// 모델에 설정된 AGING 시간 정보를 UI에 표시한다.
	sdata.Format(_T("%02d:%02d"), (lpInspWorkInfo->m_nAgingSetTime[rack] / 60), (lpInspWorkInfo->m_nAgingSetTime[rack] % 60));
	m_pSttMaSetTime[rack]->SetWindowText(sdata);
	m_pSttMaRunTime[rack]->SetWindowText(_T("00:00"));

	// Thread Aging 시작 Flag Set
	m_nAgingStart[rack] = TRUE;
	m_nAgnOutFlag[rack] = FALSE;

	// Last Model Name을 저장한다.
	CString skey;
	lpSystemInfo->m_sLastModelName[rack] = sModelName;
	skey.Format(_T("LAST_MODELNAME_RACK%d"), (rack + 1));
	Write_SysIniFile(_T("SYSTEM"), skey, sModelName);

	// RACK Log 출력
	sLog.Format(_T("Aging START : %s"), sModelName);
	Lf_writeRackMLog(rack, sLog);

	// RACK별 모델정보, 실처리 정보, Summary Log 정보를 저장한다.
	lpInspWorkInfo->m_nOpeDoorUse[rack] = lpModelInfo->m_nOpeDoorUse;
	lpInspWorkInfo->m_nAgingEndWaitTime[rack] = lpModelInfo->m_nAgingEndWaitTime;
	lpInspWorkInfo->m_nOpeTemperatureUse[rack] = lpModelInfo->m_nOpeTemperatureUse;
	lpInspWorkInfo->m_nOpeTemperatureMin[rack] = lpModelInfo->m_nOpeTemperatureMin;
	lpInspWorkInfo->m_nOpeTemperatureMax[rack] = lpModelInfo->m_nOpeTemperatureMax;
	lpInspWorkInfo->m_fOpeVccSetting[rack] = lpModelInfo->m_fVccVolt;
	lpInspWorkInfo->m_fOpeVccSetMin[rack] = lpModelInfo->m_fVccLimitVoltLow;
	lpInspWorkInfo->m_fOpeVccSetMax[rack] = lpModelInfo->m_fVccLimitVoltHigh;
	lpInspWorkInfo->m_fOpeIccSetMin[rack] = lpModelInfo->m_fVccLimitCurrLow;
	lpInspWorkInfo->m_fOpeIccSetMax[rack] = lpModelInfo->m_fVccLimitCurrHigh;
	lpInspWorkInfo->m_fOpeVblSetting[rack] = lpModelInfo->m_fVblVolt;
	lpInspWorkInfo->m_fOpeVblSetMin[rack] = lpModelInfo->m_fVblLimitVoltLow;
	lpInspWorkInfo->m_fOpeVblSetMax[rack] = lpModelInfo->m_fVblLimitVoltHigh;
	lpInspWorkInfo->m_fOpeIblSetMin[rack] = lpModelInfo->m_fVblLimitCurrLow;
	lpInspWorkInfo->m_fOpeIblSetMax[rack] = lpModelInfo->m_fVblLimitCurrHigh;


	// Summary Log 및 실처리 전송 관련 정보를 초기화 한다
	
	//lpInspWorkInfo->m_nAgingRunTime[rack] = 0;

	lpInspWorkInfo->m_nAgingDoorOpenTime[rack] = 0;

	lpInspWorkInfo->m_nAgingTempMatchTime[rack] = 0;

	lpInspWorkInfo->m_nAgingTempMeasCount[rack] = 0;
	//lpInspWorkInfo->m_nAgingPowerMeasCount[rack] = 0;
	for (int rack = 0; rack < MAX_LAYER; rack++)
	{
		for (int layer = 0; layer < MAX_LAYER; layer++)
		{
			for (int ch = 0; ch < MAX_CHANNEL; ch++)
			{
				lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][ch] = 0;
				lpInspWorkInfo->m_ast_AgingTempError[rack][layer][ch] = FALSE;
			}
		}
	}
	lpInspWorkInfo->m_fOpeAgingTempMin[rack] = 60;
	lpInspWorkInfo->m_fOpeAgingTempMax[rack] = 0;
	lpInspWorkInfo->m_fOpeAgingTempAvg[rack] = 0;
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

	lpInspWorkInfo->m_StopRackID.Format(_T("%d"), RESET_RACK_PID);
	lpInspWorkInfo->m_nAgingInYN[rack] = FALSE;

	//Lf_rmsErcpSet();
}

void CHseAgingDlg::Lf_setAgingSTOP(int rack)
{
	CString sLog;

	lpInspWorkInfo->m_PidTestError[rack] = false; // PID, AGING START ERROR MESSAGE FLAG

	// Thread Aging 시작 Flag Clear
	m_nAgingStart[rack] = FALSE;

	// Aging 상태를 IDLE 으로 변경한다.
	lpInspWorkInfo->m_nAgingOperatingMode[rack] = AGING_IDLE;
	Lf_updateTowerLamp();
	lpInspWorkInfo->m_nAgingStatusS[rack] = 0;

	// Aging STOP 및 Panel Power OFF
	m_pApp->pCommand->Gf_setAgingSTOP(rack);
	m_pApp->pCommand->Gf_setPowerSequenceOnOff(rack, POWER_OFF);

	// RACK Log 출력
	sLog.Format(_T("Aging STOP"));
	Lf_writeRackMLog(rack, sLog);

	sLog.Format(_T("<MESSAGE> AGING STOP CLICK [RACK %d]"),rack+1);

	m_pApp->Gf_writeMLog(sLog);

	// ✅ STOP 시 CurModel.ini에서 해당 모델 삭제
	{
		CString sModelName;
		m_pCmbMaModel[rack]->GetWindowText(sModelName);   // 예: "01_LP110WU3"
		RemoveCurModelIni_ByModelText(sModelName);
	}

	// Button Enable
	m_pBtnAgingStart[rack]->EnableWindow(TRUE);
	m_pBtnAgingFusing[rack]->EnableWindow(TRUE);
	m_pChkChSelect[rack]->EnableWindow(TRUE);
}

void CHseAgingDlg::Lf_setAgingFUSING(int rack)
{
	CString sLog;
	sLog.Format(_T("<MESSAGE> AGING FUSING CLICK [RACK %d]"), rack + 1);
	m_pApp->Gf_writeMLog(sLog);

	if (m_pCmbMaModel[rack]->GetCurSel() == 0)
	{
		m_pApp->Gf_ShowMessageBox(_T("No model was selected. Please select a model."));
		return;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// 2025-01-13 PDH. 모든 Layer 연결이 안되어있을 경우 N/C Fusing Skip 하도록 수정
	BOOL bFusing = FALSE;
	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		if ((lpInspWorkInfo->m_nMainEthConnect[rack][layer]) != 0)
		//if (true)
		{
			bFusing = TRUE;
			break;
		}
	}
	if (bFusing == FALSE)
	{
		Lf_writeRackMLog(rack, _T("Fusing : N/C"));
		return;
	}
	////////////////////////////////////////////////////////////////////////////////////



	CString sModelName, sdata;
	int nValue;

	lpInspWorkInfo->m_PidTestError[rack] = false; // PID, AGING START ERROR MESSAGE FLAG

	// Button Disable
	m_pBtnAgingStart[rack]->EnableWindow(FALSE);
	m_pBtnAgingStop[rack]->EnableWindow(FALSE);
	m_pBtnAgingFusing[rack]->EnableWindow(FALSE);

	memset(lpInspWorkInfo->m_ast_AgingChErrorResult[rack], 0, sizeof(lpInspWorkInfo->m_ast_AgingChErrorResult[rack]));
	memset(lpInspWorkInfo->m_ast_AgingChErrorType[rack], 0, sizeof(lpInspWorkInfo->m_ast_AgingChErrorType[rack]));
	memset(lpInspWorkInfo->m_ast_AgingChErrorValue[rack], 0, sizeof(lpInspWorkInfo->m_ast_AgingChErrorValue[rack]));


	// RACK 선택된 모델명을 가져온다
	m_pCmbMaModel[rack]->GetWindowText(sModelName);
	m_pApp->Gf_loadModelData(sModelName);

	m_pApp->pCimNet->r_strmodelName = sModelName;

	// 모델정보에서 AGING 시간 정보를 LOAD하여 UI에 표시한다.
	Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("AGING_TIME_MINUTE"), &nValue);
	sdata.Format(_T("%02d:%02d"), nValue / 60, nValue % 60);
	m_pSttMaSetTime[rack]->SetWindowText(sdata);
	m_pSttMaRunTime[rack]->SetWindowText(_T("00:00"));

	// SYSTEM Fusing
	Lf_writeRackMLog(rack, _T("Fusing : Start"));
	if (m_pApp->pCommand->Gf_setFusingSystemInfo(rack) == TRUE)
		Lf_writeRackMLog(rack, _T("Fusing : OK"));
	else
		Lf_writeRackMLog(rack, _T("Fusing : NG"));

	// Progress Bard 진행상태를 초기화 한다.
	m_pCtrMaProgress[rack]->SetPos(0);

	// Last Model Name을 저장한다.
	CString skey;
	lpSystemInfo->m_sLastModelName[rack] = sModelName;
	skey.Format(_T("LAST_MODELNAME_RACK%d"), (rack + 1));
	Write_SysIniFile(_T("SYSTEM"), skey, sModelName);

	// Button Disable
	m_pBtnAgingStart[rack]->EnableWindow(TRUE);
	m_pBtnAgingStop[rack]->EnableWindow(TRUE);
	m_pBtnAgingFusing[rack]->EnableWindow(TRUE);

	m_nAgnOutFlag[rack] = FALSE;

	m_pApp->Gf_sumInitSummaryInfo(rack); // summary.ini 초기화

}

int CHseAgingDlg::Lf_getChannelInfo(int ctrlID)
{
	int retChStatus = STATUS_IDLE;
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		for (int layer = 0; layer < MAX_LAYER; layer++)
		{
			for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
			{
				if (ctrlID == m_pSttRackCtrlID[rack][layer][ch])
				{
#if 1
					if (lpInspWorkInfo->m_nMainEthConnect[rack][layer] == FALSE)
						retChStatus = STATUS_NOT_CONNECT;
					else if (lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] == CHANNEL_UNUSE)
						retChStatus = STATUS_UNUSE;
					else if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] != LIMIT_NONE)
						retChStatus = STATUS_ERROR;
					else if (lpInspWorkInfo->m_ast_AgingChOnOff[rack][layer][ch] == ON)
						retChStatus = STATUS_RUN;
#else
					if (lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] == CHANNEL_UNUSE)
						retChStatus = STATUS_UNUSE;
					else if (lpInspWorkInfo->m_nMainEthConnect[rack][layer] == FALSE)
						retChStatus = STATUS_NOT_CONNECT;
					else if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] != LIMIT_NONE)
						retChStatus = STATUS_ERROR;
					else if (lpInspWorkInfo->m_ast_AgingChOnOff[rack][layer][ch] == ON)
						retChStatus = STATUS_RUN;
#endif
					lpInspWorkInfo->m_nChMainUiStatusOld[rack][layer][ch] = retChStatus;
				}
			}
		}
	}

	return retChStatus;
}

int CHseAgingDlg::Lf_getChannelStatus(int rack, int layer, int ch)
{
	int retChStatus = STATUS_IDLE;

#if 1
	if (lpInspWorkInfo->m_nMainEthConnect[rack][layer] == FALSE)
		retChStatus = STATUS_NOT_CONNECT;
	else if (lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] == CHANNEL_UNUSE)
		retChStatus = STATUS_UNUSE;
	else if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] != LIMIT_NONE)
		retChStatus = STATUS_ERROR;
	else if (lpInspWorkInfo->m_ast_AgingChOnOff[rack][layer][ch] == ON)
		retChStatus = STATUS_RUN;
#else
	if (lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] == CHANNEL_UNUSE)
		retChStatus = STATUS_UNUSE;
	else if (lpInspWorkInfo->m_nMainEthConnect[rack][layer] == FALSE)
		retChStatus = STATUS_NOT_CONNECT;
	else if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] != LIMIT_NONE)
		retChStatus = STATUS_ERROR;
	else if (lpInspWorkInfo->m_ast_AgingChOnOff[rack][layer][ch] == ON)
		retChStatus = STATUS_RUN;
#endif

	return retChStatus;
}

void CHseAgingDlg::Lf_updateSystemInfo()
{
	////////////////////////////////////////////////////////////////////////////////
	GetDlgItem(IDC_STT_CONNECT_INFO_PG)->Invalidate(FALSE);
	GetDlgItem(IDC_STT_CONNECT_INFO_DIO)->Invalidate(FALSE);
	GetDlgItem(IDC_STT_CONNECT_INFO_TEMP)->Invalidate(FALSE);
	GetDlgItem(IDC_STT_CONNECT_INFO_BARCODE)->Invalidate(FALSE);
	GetDlgItem(IDC_STT_CONNECT_INFO_MES)->Invalidate(FALSE);

	GetDlgItem(IDC_STT_MA_CHAMBER_NO)->SetWindowText(lpSystemInfo->m_sChamberNo);

	if (lpInspWorkInfo->m_nConnectInfo[CONNECT_MES] == TRUE)
	{
		GetDlgItem(IDC_STT_CONNECT_INFO_MES)->SetWindowText(_T("[MES]ON-LINE"));
	}
	else
	{
		GetDlgItem(IDC_STT_CONNECT_INFO_MES)->SetWindowText(_T("[MES]OFF-LINE"));
	}
	GetDlgItem(IDC_STT_MES_USER_ID)->SetWindowText(m_pApp->m_sUserID);
	GetDlgItem(IDC_STT_MES_USER_NAME)->SetWindowText(m_pApp->m_sUserName);
}

void CHseAgingDlg::Lf_writeRackMLog(int rack, CString sLog)
{
	int maxAlarmList = 1000;

	if (m_pLstMLog[rack]->GetCount() > maxAlarmList)
	{
		m_pLstMLog[rack]->DeleteString(maxAlarmList - 1);
	}

	CString message;
	CTime time = CTime::GetCurrentTime();
	message.Format(_T("[%02d:%02d] %s"), time.GetHour(), time.GetMinute(), sLog);
	m_pLstMLog[rack]->SetCurSel(m_pLstMLog[rack]->AddString(message));
}

void CHseAgingDlg::Lf_getMeasurePower()
{
	m_pApp->pCommand->Gf_getPowerMeasureAllGroup();
}


void CHseAgingDlg::Lf_getAgingStatus()
{
	int skipGroup[6];

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		skipGroup[rack] = m_pChkChSelect[rack]->GetCheck();
	}

	m_pApp->pCommand->Gf_getAgingStatusAllGroup(skipGroup);
}


void CHseAgingDlg::Lf_updateEthConnectInfo()
{
	lpInspWorkInfo->m_nConnectInfo[CONNECT_PG] = FALSE;

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		for (int layer = 0; layer < MAX_LAYER; layer++)
		{
			if (lpInspWorkInfo->m_nMainEthConnect[rack][layer] != 0)
			{
				lpInspWorkInfo->m_nConnectInfo[CONNECT_PG] = TRUE;
			}
		}
	}
	Lf_updateSystemInfo();
}

void CHseAgingDlg::Lf_updateAgingStatus()
{
	int ActiveGroup = 0, chStatus;
	CString sLog, sevent, sdata, skey;

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		if (m_pChkChSelect[rack]->GetCheck() == TRUE)
			continue;

		// CH의 상태를 Update 한다.
		for (int layer = 0; layer < MAX_LAYER; layer++)
		{
			for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
			{
				chStatus = Lf_getChannelStatus(rack, layer, ch);
				if (chStatus != lpInspWorkInfo->m_nChMainUiStatusOld[rack][layer][ch])
				{
					m_pSttRackState[rack][layer][ch]->Invalidate(FALSE);
				}
			}
		}

		if (lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_RUNNING)
		{
			// 선택된 Group 의 Aging  Status
			CString sdata;
			int nVal;

			sdata.Format(_T("%02d:%02d"), lpInspWorkInfo->m_nAgingRunTime[rack] / 60, lpInspWorkInfo->m_nAgingRunTime[rack] % 60);
			m_pSttMaRunTime[rack]->SetWindowText(sdata);
			if (lpInspWorkInfo->m_nAgingSetTime[rack] > 0)
			{
				nVal = (int)(((float)lpInspWorkInfo->m_nAgingRunTime[rack] / (float)lpInspWorkInfo->m_nAgingSetTime[rack]) * 100.0f);
				m_pCtrMaProgress[rack]->SetPos(nVal);
			}
		}
		else if (lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_COMPLETE)
		{
			if (m_nAgnOutFlag[rack] == FALSE)
			{
				m_nAgnOutFlag[rack] = TRUE;

				// RACK Log 출력
				sLog.Format(_T("Aging COMPLETE"));
				Lf_writeRackMLog(rack, sLog);

				Lf_setAgingSTOP(rack);

				// Aging END 시간을 기록한다.
				m_pApp->Gf_sumSetEndTime(rack);

				// RACK Log 출력
				sLog.Format(_T("MES Send"));
				Lf_writeRackMLog(rack, sLog);

				// Progress, Run Time을 Max로 표시한다.
				m_pCtrMaProgress[rack]->SetPos(100);
				sdata.Format(_T("%02d:%02d"), lpInspWorkInfo->m_nAgingSetTime[rack] / 60, lpInspWorkInfo->m_nAgingSetTime[rack] % 60);
				m_pSttMaRunTime[rack]->SetWindowText(sdata);

				// MES 자동보고 진행한다.
				CPidInput idDlg;
				idDlg.m_nMesAutoRackNo = rack;
				idDlg.m_nMesAutoDMOU = MES_DMOU_MODE_AUTO;

				lpInspWorkInfo->m_nAgnOutYn[rack] = TRUE;
				lpInspWorkInfo->m_nAgnRack = TRUE;
				AfxGetApp()->GetMainWnd()->SendMessage(WM_BCR_RACK_ID_INPUT, (WPARAM)rack, NULL);
				//idDlg.DoModal();

				Lf_readSummaryIni(rack);

				// RACK Log 출력
				sLog.Format(_T("Summary Log Write"));
				Lf_writeRackMLog(rack, sLog);

				for (int layer = 0; layer < MAX_LAYER; layer++)
				{
					for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
					{
						m_pApp->Gf_sumWriteSummaryLog(rack, layer, ch);

						///////////////////////////////////////////////////////////////////////////////////////////////
						// 2023-12-01 PDH. Aging 실처리 보고
						// Summary Log Write 정보 기준으로 실처리하기 때문에 Summary Log Write 뒤에 보고해야 한다.
						///////////////////////////////////////////////////////////////////////////////////////////////
						if ((m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_PID].IsEmpty() == TRUE)
							|| (m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_CHANNEL].IsEmpty() == TRUE)
							)
						{
							CString strLog;

							strLog.Format(_T("<EAS> APDR send skip. 'Channel ID' or 'Panel ID' is empty, (%s,%s)"), m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_CHANNEL], m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_PID]);
							m_pApp->Gf_writeMLog(strLog);
							continue;
						}
						lpSystemInfo->m_Aging_Count++;
						lpSystemInfo->m_Aging_Month_Count++;
						m_pApp->Gf_gmesSendHost(HOST_APDR, rack, layer, ch);
						///////////////////////////////////////////////////////////////////////////////////////////////
						///////////////////////////////////////////////////////////////////////////////////////////////
					}
				}

				m_pApp->Gf_sumInitSummaryInfo(rack);

				sLog.Format(_T("<MESSAGE> AGING END [RACK %d]"), rack + 1);

				m_pApp->Gf_writeMLog(sLog);

				// Time Out 값을 초기화 한다.
				skey.Format(_T("LAST_TIMEOUT_RACK%d"), rack+1);
				Write_SysIniFile(_T("SYSTEM"), skey, 0);

				Write_SysIniFile(_T("COUNT"), _T("AGING_COUNT"), lpSystemInfo->m_Aging_Count);
				Write_SysIniFile(_T("COUNT"), _T("AGING_NG_COUNT"), lpSystemInfo->m_Aging_Ng_Count);
				Write_SysIniFile(_T("COUNT"), _T("AGING_MONTH_COUNT"), lpSystemInfo->m_Aging_Month_Count);
				Write_SysIniFile(_T("COUNT"), _T("AGING_MONTH_NG_COUNT"), lpSystemInfo->m_Aging_Month_Ng_Count);

			}


			if (m_bOnOffFlag[rack] == FALSE)
			{
				m_bOnOffFlag[rack] = TRUE;
				m_pApp->pCommand->Gf_setPowerSequenceOnOff(rack, OFF, NACK);
			}
		}
	}
}

void CHseAgingDlg::Lf_updateTowerLamp()
{
	int towerStatus = 0;
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		if (lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_IDLE)
			towerStatus |= TOWER_LAMP_READY;
		else if (lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_RUNNING)
			towerStatus |= TOWER_LAMP_RUNNING;
		else if (lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_COMPLETE)
			towerStatus |= TOWER_LAMP_COMPLETE;
		else if (lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_ERROR)
			towerStatus |= TOWER_LAMP_ERROR;
		else if (lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_COMPLETE_DOORCLOSE)
			towerStatus |= 0x16;

		/*if (lpInspWorkInfo->m_nAgingStatusS[rack] == 0)
		{
		}
		else if (lpInspWorkInfo->m_nAgingStatusS[rack] == 1)
		{

		}
		else if (lpInspWorkInfo->m_nAgingStatusS[rack] == 2)
		{

		}*/
	}

	if (towerStatus != lpInspWorkInfo->m_nTowerLampStatus || lpInspWorkInfo->m_nLampColor == 1)
	{
		lpInspWorkInfo->m_nTowerLampStatus = towerStatus;

		CString Dxt;

		int outData = 0, blinkMode = 0;
		if (towerStatus & TOWER_LAMP_ERROR)
		{
			outData = outData | (DIO_OUT_RED | DIO_OUT_BUZZER);
			blinkMode = blinkMode | DIO_OUT_RED_BLINK;

			Sleep(5000);
			if (lpInspWorkInfo->m_nAgingInYN[lpInspWorkInfo->m_AgingErrorRack] == TRUE)
			{
				lpInspWorkInfo->m_nAgingOperatingMode[lpInspWorkInfo->m_AgingErrorRack] = AGING_RUNNING;
			}
		}
		if (towerStatus & TOWER_LAMP_RUNNING)
		{
			outData = outData | DIO_OUT_GREEN;
			/*Dxt.Format(_T("초록색"));
			OutputDebugString(Dxt);*/
		}
		if (towerStatus & TOWER_LAMP_COMPLETE)
		{
			/*outData = outData | DIO_OUT_GREEN;
			blinkMode = blinkMode | DIO_OUT_GREEN_BLINK;*/
			outData = outData | DIO_OUT_GREEN;
			Lf_setDIOWrite(outData, 0);
			outData = outData | DIO_OUT_YELLOW;
			Lf_setDIOWrite(outData, 0);
			/*Dxt.Format(_T("초록 노랑"));
			OutputDebugString(Dxt);*/
		}
		if(towerStatus & TOWER_LAMP_READY)
		{
			outData = outData | DIO_OUT_YELLOW;
			/*Dxt.Format(_T("노란색"));
			OutputDebugString(Dxt);*/
		}

		for (int rack = 0; rack < MAX_RACK; rack++)
		{
			if (lpInspWorkInfo->m_nAgingStatusS[rack] == 1)
			{
				outData = outData | (DIO_OUT_GREEN | DIO_OUT_BUZZER);
				blinkMode = blinkMode | DIO_OUT_GREEN_BLINK;
				/*Dxt.Format(_T("초록반짝"));
				OutputDebugString(Dxt);*/
			}
			
		}

		Lf_setDIOWrite(outData, blinkMode);
	}
}

void CHseAgingDlg::Lf_setDIOWrite(int outData, int mode)
{
	lpInspWorkInfo->m_nDioOutputData = outData;
	lpInspWorkInfo->m_nDioOutputMode = mode;

	m_pApp->pCommand->Gf_dio_setDIOWriteOutput(outData, mode);
}

void CHseAgingDlg::Lf_setDIOBoardInitial()
{
	m_pApp->pCommand->Gf_dio_setDIOBoardInitial();
}

void CHseAgingDlg::Lf_getDIOStatus()
{
	if (lpInspWorkInfo->m_nDioNeedInitial == TRUE)
	{
		Lf_setDIOBoardInitial();
		return;
	}

	if (lpInspWorkInfo->m_nConnectInfo[CONNECT_DIO] != 0)
	{
		lpInspWorkInfo->m_nConnectInfo[CONNECT_DIO] = lpInspWorkInfo->m_nConnectInfo[CONNECT_DIO] - 1;

		if (lpInspWorkInfo->m_nConnectInfo[CONNECT_DIO] == 0)
		{
			lpInspWorkInfo->m_nDioNeedInitial = TRUE;
		}
	}

	m_pApp->pCommand->Gf_dio_getDIOReadStatus();

	///////////////////////////////////////////////////////////////////
	Lf_checkDoorOpenClose();
}

void CHseAgingDlg::Lf_readSummaryIni(int rack)
{
	int ch = 0;
	CString skey = _T("");

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			skey.Format(_T("RACK%d_LAYER%d_CH%d"), rack+1, layer+1, (ch + 1));
			Read_SummaryInfo(_T("PID"), skey, &m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_PID]);
			Read_SummaryInfo(_T("AGING_TIME"), skey, &m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_AGING_TIME]);
			Read_SummaryInfo(_T("START_TIME"), skey, &m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_START_TIME]);
			Read_SummaryInfo(_T("END_TIME"), skey, &m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_END_TIME]);
			Read_SummaryInfo(_T("FAIL_MESSAGE"), skey, &m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_FAILED_MESSAGE]);
			Read_SummaryInfo(_T("FAIL_TIME"), skey, &m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_FAILED_MESSAGE_TIME]);
			Read_SummaryInfo(_T("VCC"), skey, &m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_MEAS_VCC]);
			Read_SummaryInfo(_T("ICC"), skey, &m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_MEAS_ICC]);
			Read_SummaryInfo(_T("VBL"), skey, &m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_MEAS_VBL]);
			Read_SummaryInfo(_T("IBL"), skey, &m_pApp->m_summaryInfo[rack][layer][ch].m_sumData[SUM_MEAS_IBL]);
		}
	}
}

//void CHseAgingDlg::Lf_getTemperature()
//{
//	if (lpInspWorkInfo->m_nConnectInfo[CONNECT_TEMP] != 0)
//		lpInspWorkInfo->m_nConnectInfo[CONNECT_TEMP]--;
//
//	m_pApp->m_pTemp2xxx->TempSDR100_readTemp();
//
//	// Temp Log Interval 시간이되면 Temperature Log 를 기록한다.
//	if (lpSystemInfo->m_nTempLogInterval != 0)
//	{
//		CTime time = CTime::GetCurrentTime();
//		if ((time.GetMinute() - m_nTempLogWriteMin) >= lpSystemInfo->m_nTempLogInterval)
//		{
//			m_nTempLogWriteMin = time.GetMinute();
//			Lf_writeTempLog();
//
//			Lf_saveTempMinMax();
//		}
//	}
//
//	// 온도 정보를 UI에 업데이트 한다.
//	Lf_updateTempature();
//}

void CHseAgingDlg::Lf_getTemperature()
{
	if (lpInspWorkInfo->m_nConnectInfo[CONNECT_TEMP] != 0)
		lpInspWorkInfo->m_nConnectInfo[CONNECT_TEMP]--;

	m_pApp->m_pTemp2xxx->TempSDR100_readTemp();

	// Temp Log Interval 시간이되면 Temperature Log 를 기록한다.
	if (lpSystemInfo->m_nTempLogInterval != 0)
	{
		CTime time = CTime::GetCurrentTime();
		int currentHour = time.GetHour();
		int currentMinute = time.GetMinute();

		// 시간과 분을 함께 고려하여 로그 저장 조건을 수정
		if ((currentHour != m_nTempLogWriteHour) ||
			(currentMinute - m_nTempLogWriteMin) >= lpSystemInfo->m_nTempLogInterval)
		{
			m_nTempLogWriteHour = currentHour; // 현재 시간을 저장
			m_nTempLogWriteMin = currentMinute; // 현재 분을 저장

			Lf_writeTempLog();
			Lf_writeTempLog_Rackfile();

			Lf_saveTempMinMax();

			// Sensing Log 기록한다.
			char dateStr[64];
			sprintf_s(dateStr, "%04d%02d%02d", time.GetYear(), time.GetMonth(), time.GetDay());

			char basePath[256] = ".\\Logs\\SensingLog";
			if (_access(basePath, 0) == -1)
				_mkdir(basePath);

			char timeKey[32];
			sprintf_s(timeKey, "%02d:%02d:%02d",
				time.GetHour(), time.GetMinute(), time.GetSecond());

			// RACK별 파일 생성
			for (int rack = 0; rack < MAX_RACK; rack++)
			{
				char rackPath[256];
				sprintf_s(rackPath, "%s\\Rack%d", basePath, rack + 1);

				if (_access(rackPath, 0) == -1)
					_mkdir(rackPath);

				char filePath[256];
				sprintf_s(filePath, "%s\\SensingLog%d_%s.csv",
					rackPath, rack + 1, dateStr);

				FILE* fp;
				fopen_s(&fp, filePath, "r+");

				if (fp == NULL)   // 새 파일 생성
				{
					fopen_s(&fp, filePath, "a+");
					if (fp == NULL) continue;

					// ★ 헤더 작성
					fprintf(fp, "TIME,RACK,LAYER,CH,PID,VCC,ICC,VBL,IBL,TEMP\n");
				}

				fseek(fp, 0L, SEEK_END);

				for (int layer = 0; layer < MAX_LAYER; layer++)
				{
					for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
					{
						char panelID[128];
						sprintf_s(panelID, "%S",
							lpInspWorkInfo->m_sMesPanelID[rack][layer][ch].GetString());

						float measVCC = lpInspWorkInfo->m_nMeasVCC[rack][layer][ch] / 100.0f;
						float measICC = lpInspWorkInfo->m_nMeasICC[rack][layer][ch] / 100.0f;
						float measVBL = lpInspWorkInfo->m_nMeasVBL[rack][layer][ch] / 100.0f;
						float measIBL = lpInspWorkInfo->m_nMeasIBL[rack][layer][ch] / 100.0f;
						float tempVal = lpInspWorkInfo->m_fTempReadVal[rack];

						// ★ 데이터 라인 작성
						fprintf(fp, "%s,%d,%d,%d,%s,%.2f,%.2f,%.2f,%.2f,%.1f\n",
							timeKey,
							rack + 1,
							layer + 1,
							ch + 1,
							panelID,
							measVCC, measICC, measVBL, measIBL,
							tempVal);

						// ★ Min/Max/Avg 처리는 기존대로 유지
						lpInspWorkInfo->m_fOpeAgingVccAvg[rack][layer][ch] += measVCC;
						lpInspWorkInfo->m_fOpeAgingIccAvg[rack][layer][ch] += measICC;
						lpInspWorkInfo->m_fOpeAgingVblAvg[rack][layer][ch] += measVBL;
						lpInspWorkInfo->m_fOpeAgingIblAvg[rack][layer][ch] += measIBL;

						lpInspWorkInfo->m_nAgingPowerMeasCount[rack][layer][ch]++;

						Lf_savePowerMeasureMinMax(rack, layer, ch);
					}
				}
				fclose(fp);
			}
		}
	}

	// 온도 정보를 UI에 업데이트 한다.
	Lf_updateTempature();
}


void CHseAgingDlg::Lf_writeTempLog()
{
	FILE* fp;

	char filepath[128] = { 0 };
	char buff[256] = { 0 };
	char dataline[1024] = { 0 };

	SYSTEMTIME sysTime;
	::GetSystemTime(&sysTime);
	CTime time = CTime::GetCurrentTime();

	sprintf_s(filepath, ".\\Logs\\TemperatureLog\\TempLog_%04d%02d%02d.csv", time.GetYear(), time.GetMonth(), time.GetDay());
	fopen_s(&fp, filepath, "r+");

	static int lastInitMonth = -1;
	int curDay = time.GetDay();
	int curMonth = time.GetMonth();

	if (fp == NULL)
	{

		if (curDay == 1) // 매월 초기화
		{
			if (lastInitMonth != curMonth)
			{
				lpSystemInfo->m_Aging_Month_Count = 0;
				lpSystemInfo->m_Aging_Month_Ng_Count = 0;

				Write_SysIniFile(_T("COUNT"), _T("AGING_MONTH_COUNT"), lpSystemInfo->m_Aging_Month_Count);
				Write_SysIniFile(_T("COUNT"), _T("AGING_MONTH_NG_COUNT"), lpSystemInfo->m_Aging_Month_Ng_Count);

				lastInitMonth = curMonth;
			}
		}
		else                 
		{
			lastInitMonth = -1;
		}
		lpSystemInfo->m_Aging_Count = 0;
		lpSystemInfo->m_Aging_Ng_Count = 0;
		Write_SysIniFile(_T("COUNT"), _T("AGING_COUNT"), lpSystemInfo->m_Aging_Count);
		Write_SysIniFile(_T("COUNT"), _T("AGING_NG_COUNT"), lpSystemInfo->m_Aging_Ng_Count); // 매일 초기화
		if ((_access(".\\Logs\\TemperatureLog", 0)) == -1)
			_mkdir(".\\Logs\\TemperatureLog");

		delayMs(1);
		fopen_s(&fp, filepath, "a+");
		if (fp == NULL)
		{
			if ((_access(filepath, 2)) != -1)
			{
				delayMs(1);
				fopen_s(&fp, filepath, "a+");
				if (fp == NULL)
				{
					return;
				}
			}
		}
		sprintf_s(buff, "Hour,Minute,RACK1,RACK2,RACK3,RACK4,RACK5,RACK6,ZONE1,ZONE2,ZONE3,ZONE1_SET,ZONE2_SET,ZONE3_SET\n");
		fprintf(fp, "%s", buff);
			
	}

	fseek(fp, 0L, SEEK_END);

	sprintf_s(buff, "%02d,%02d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f",
		time.GetHour(),
		time.GetMinute(),
		(lpInspWorkInfo->m_fTempReadVal[0]), // 1ZONE FRONT 온도 (S1)
		(lpInspWorkInfo->m_fTempReadVal[1]), // 1ZONE BACK  온도 (S2)
		(lpInspWorkInfo->m_fTempReadVal[2]), // 2ZONE FRONT 온도 (S3)
		(lpInspWorkInfo->m_fTempReadVal[3]), // 2ZONE BACK  온도 (S4)
		(lpInspWorkInfo->m_fTempReadVal[4]), // 3ZONE FRONT 온도 (S5)
		(lpInspWorkInfo->m_fTempReadVal[5]), // 3ZONE BACK  온도 (S6)
		(lpInspWorkInfo->m_fTempReadValST590_2[0]), // 메인 컨트롤러 셋팅값 (1ZONE)
		(lpInspWorkInfo->m_fTempReadValST590_3[0]), // 메인 컨트롤러 셋팅값 (2ZONE)
		(lpInspWorkInfo->m_fTempReadValST590_4[0]), // 메인 컨트롤러 셋팅값 (3ZONE)
		(lpInspWorkInfo->m_fTempReadValST590_2_SET[0]), // 메인 컨트롤러 현재값 (1ZONE)
		(lpInspWorkInfo->m_fTempReadValST590_3_SET[0]), // 메인 컨트롤러 현재값 (2ZONE)
		(lpInspWorkInfo->m_fTempReadValST590_4_SET[0])  // 메인 컨트롤러 현재값 (3ZONE)
	);

	char* pos = dataline;

	sprintf_s(dataline, "%s\n", buff);
	fprintf(fp, "%s", pos);

	fclose(fp);
}

void CHseAgingDlg::Lf_writeTempLog_Rackfile()
{
	FILE* fp;
	char basePath[256] = ".\\Logs\\TemperatureLog";
	char rackPath[256];
	char filePath[256];

	SYSTEMTIME st;
	::GetLocalTime(&st);

	char dateStr[32];
	sprintf_s(dateStr, "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);

	// TemperatureLog 폴더 생성
	if (_access(basePath, 0) == -1)
		_mkdir(basePath);

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		sprintf_s(rackPath, "%s\\Rack%d", basePath, rack + 1);

		if (_access(rackPath, 0) == -1)
			_mkdir(rackPath);

		sprintf_s(filePath, "%s\\TempLog%d_%s.csv", rackPath, rack + 1, dateStr);

		fopen_s(&fp, filePath, "r+");
		if (fp == NULL)
		{
			fopen_s(&fp, filePath, "a+");
			if (fp == NULL) continue;

			// ★ Rack별 zone 그룹 계산
			int zoneIndex = rack / 2 + 1;   // 0,1→1 / 2,3→2 / 4,5→3

			fprintf(fp,
				"Hour, Minute, RACK%d, ZONE%d, ZONE%d_SET\n",
				rack + 1, zoneIndex, zoneIndex);
		}

		fseek(fp, 0L, SEEK_END);

		// ★ Rack별 Zone 매핑
		float rackTemp = lpInspWorkInfo->m_fTempReadVal[rack];

		float zoneTemp, zoneSetVal;

		if (rack < 2)              // Rack1, Rack2 → Zone1
		{
			zoneTemp = lpInspWorkInfo->m_fTempReadValST590_2[rack];
			zoneSetVal = lpInspWorkInfo->m_fTempReadValST590_2_SET[rack];
		}
		else if (rack < 4)        // Rack3, Rack4 → Zone2
		{
			zoneTemp = lpInspWorkInfo->m_fTempReadValST590_3[rack - 2];
			zoneSetVal = lpInspWorkInfo->m_fTempReadValST590_3_SET[rack - 2];
		}
		else                      // Rack5, Rack6 → Zone3
		{
			zoneTemp = lpInspWorkInfo->m_fTempReadValST590_4[rack - 4];
			zoneSetVal = lpInspWorkInfo->m_fTempReadValST590_4_SET[rack - 4];
		}

		fprintf(fp,
			"%02d, %02d, %.1f, %.1f, %.1f\n",
			st.wHour,
			st.wMinute,
			rackTemp,
			zoneTemp,
			zoneSetVal
		);

		fclose(fp);
	}
}

void CHseAgingDlg::Lf_updateTempature()
{
	int i = 0;
	CString sdata = _T("");

	for (i = 0; i < MAX_TEMP_SENSOR; i++)
	{
		if (m_pApp->m_pTemp2xxx->m_bPortOpen2 == TRUE)
		{
			sdata.Format(_T("%.1f"), lpInspWorkInfo->m_fTempReadVal[i]);
			m_pSttTempInfo[i]->SetWindowText(sdata);
		}
		else
		{
			m_pSttTempInfo[i]->SetWindowText(_T("N/C"));
		}
	}
}

void CHseAgingDlg::Lf_saveTempMinMax()
{
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		if (lpInspWorkInfo->m_nAgingOperatingMode[rack] != AGING_RUNNING)
			continue;

		// Min Value Save
		if (lpInspWorkInfo->m_fOpeAgingTempMin[rack] > lpInspWorkInfo->m_fTempReadVal[rack])
		{
			lpInspWorkInfo->m_fOpeAgingTempMin[rack] = lpInspWorkInfo->m_fTempReadVal[rack];
		}

		// Max Value Save
		if (lpInspWorkInfo->m_fOpeAgingTempMax[rack] < lpInspWorkInfo->m_fTempReadVal[rack])
		{
			lpInspWorkInfo->m_fOpeAgingTempMax[rack] = lpInspWorkInfo->m_fTempReadVal[rack];
		}

		// AVG 계산을 위한 Data 누적. AVG 변수에 측정값을 누적하고 Complete 시점에 Count 값으로 나눈다
		lpInspWorkInfo->m_fOpeAgingTempAvg[rack] = lpInspWorkInfo->m_fOpeAgingTempAvg[rack] + lpInspWorkInfo->m_fTempReadVal[rack];
		lpInspWorkInfo->m_nAgingTempMeasCount[rack] = lpInspWorkInfo->m_nAgingTempMeasCount[rack] + 1;
	}
}

void CHseAgingDlg::Lf_parseSDR100Packet(char* szpacket)
{
	CString sdata;
	char szCmd[5] = { 0, };
	char szRet[5] = { 0, };
	CHseAgingDlg* pDlg = (CHseAgingDlg*)AfxGetMainWnd();

	if (szpacket[0] == _STX_)
	{
		int addr = 0;
		sscanf_s(&szpacket[1], "%02d", &addr);

		if (addr == TEMPSDR100_ADDR) // 온도 레코더일 때
		{
			memcpy(szCmd, &szpacket[3], 3);
			memcpy(szRet, &szpacket[7], 2);

		if (!strcmp(szRet, "OK"))
		{
			lpInspWorkInfo->m_nConnectInfo[CONNECT_TEMP] = 5;

			if (!strcmp(szCmd, "RSD")) // 받는 패킷이 RSD
			{
				for (int tp = 1; tp < 7; tp++)
				{
					int nVal;
					sscanf_s(&szpacket[10 + (tp - 1) * 5], "%04X", &nVal);

					if ((float)(nVal / 10.0) < 10)
					{
						continue;
					}

					if (nVal > 1000)	nVal = 0;
					lpInspWorkInfo->m_fTempReadVal[tp - 1] = (float)(nVal / 10.0);
					//lpInspWorkInfo->m_fTempReadVal[tp - 1] = 45+tp;

					//CString sTemp;
					//sTemp.Format(_T("%.1f°C"), lpInspWorkInfo->m_fTempReadVal[1]);  // tp=3 번째 → 인덱스 2
					//GetDlgItem(IDC_STT_TEMP_SENSOR2)->SetWindowText(sTemp);
				}
			}
		}
		}
		else if (addr == TEMPST590_ADDR2) // 온도 컨트롤러일 때(ADDR - 2)
		{
			memcpy(szCmd, &szpacket[3], 3);
			memcpy(szRet, &szpacket[7], 2);

			if (!strcmp(szRet, "OK"))
			{
				lpInspWorkInfo->m_nConnectInfo[CONNECT_TEMP] = 5;

				/*if (!strcmp(szCmd, "RSD") || !strcmp(szCmd, "RRD"))*/
				if (!strcmp(szCmd, "RSD"))
				{
					for (int tp = 1; tp < 4; tp++)
					{
						int nVal;
						sscanf_s(&szpacket[10 + (tp - 1) * 5], "%04X", &nVal);

						if (nVal > 1000)	nVal = 0;

						// 온도 저장
						lpInspWorkInfo->m_fTempReadValST590_2[tp - 1] = (float)(nVal / 10.0);
					}

					// 세 번째 온도를 IDC_STT_TEMP_SENSOR2에 표시
					/*CString sTemp;
					sTemp.Format(_T("%.1fC + %.1fC"), lpInspWorkInfo->m_fTempReadValST590_2_SET[0], lpInspWorkInfo->m_fTempReadValST590_2[0]);
					GetDlgItem(IDC_STT_TEMP_SENSOR2)->SetWindowText(sTemp);*/
				}
				else if (!strcmp(szCmd, "RRD"))
				{
					for (int tp = 1; tp < 4; tp++)
					{
						int nVal;
						sscanf_s(&szpacket[10 + (tp - 1) * 5], "%04X", &nVal);
						if (nVal > 1000) nVal = 0;

						lpInspWorkInfo->m_fTempReadValST590_2_SET[tp - 1] = (float)(nVal / 10.0);
					}
				}
			}
		}
		else if (addr == TEMPST590_ADDR3) // 온도 컨트롤러일 때(ADDR - 3)
		{
			memcpy(szCmd, &szpacket[3], 3);
			memcpy(szRet, &szpacket[7], 2);

			if (!strcmp(szRet, "OK"))
			{
				lpInspWorkInfo->m_nConnectInfo[CONNECT_TEMP] = 5;

				//if (!strcmp(szCmd, "RSD") || !strcmp(szCmd, "RRD"))
				if (!strcmp(szCmd, "RSD"))
				{
					for (int tp = 1; tp < 4; tp++)
					{
						int nVal;
						sscanf_s(&szpacket[10 + (tp - 1) * 5], "%04X", &nVal);

						if (nVal > 1000)	nVal = 0;

						// 온도 저장
						lpInspWorkInfo->m_fTempReadValST590_3[tp - 1] = (float)(nVal / 10.0);
					}

					// 세 번째 온도를 IDC_STT_TEMP_SENSOR3에 표시
					/*CString sTemp;
					sTemp.Format(_T("%.1fC + %.1fC"), lpInspWorkInfo->m_fTempReadValST590_3_SET[0], lpInspWorkInfo->m_fTempReadValST590_3[0]);
					GetDlgItem(IDC_STT_TEMP_SENSOR3)->SetWindowText(sTemp);*/
				}
				else if (!strcmp(szCmd, "RRD"))
				{
					for (int tp = 1; tp < 4; tp++)
					{
						int nVal;
						sscanf_s(&szpacket[10 + (tp - 1) * 5], "%04X", &nVal);
						if (nVal > 1000)	nVal = 0;
						lpInspWorkInfo->m_fTempReadValST590_3_SET[tp - 1] = (float)(nVal / 10.0);
					}
				}
			}
		}
		else if (addr == TEMPST590_ADDR4) // 온도 컨트롤러일 때(ADDR - 4)
		{
			memcpy(szCmd, &szpacket[3], 3);
			memcpy(szRet, &szpacket[7], 2);

			if (!strcmp(szRet, "OK"))
			{
				lpInspWorkInfo->m_nConnectInfo[CONNECT_TEMP] = 5;

				//if (!strcmp(szCmd, "RSD") || !strcmp(szCmd, "RRD"))
				if (!strcmp(szCmd, "RSD")) // 셋팅값 저장
				{
					for (int tp = 1; tp < 4; tp++)
					{
						int nVal;
						sscanf_s(&szpacket[10 + (tp - 1) * 5], "%04X", &nVal);

						if (nVal > 1000)	nVal = 0;

						// 온도 저장
						lpInspWorkInfo->m_fTempReadValST590_4[tp - 1] = (float)(nVal / 10.0);
					}

					// 세 번째 온도를 IDC_STT_TEMP_SENSOR4에 표시
					/*CString sTemp;
					sTemp.Format(_T("%.1fC + %.1fC"), lpInspWorkInfo->m_fTempReadValST590_4_SET[0], lpInspWorkInfo->m_fTempReadValST590_4[0]);
					GetDlgItem(IDC_STT_TEMP_SENSOR4)->SetWindowText(sTemp);*/
				}
				else if (!strcmp(szCmd, "RRD")) // 현재 값 저장
				{
					for (int tp = 1; tp < 4; tp++)
					{
						int nVal;
						sscanf_s(&szpacket[10 + (tp - 1) * 5], "%04X", &nVal);
						if (nVal > 1000)	nVal = 0;
						lpInspWorkInfo->m_fTempReadValST590_4_SET[tp - 1] = (float)(nVal / 10.0);
					}
				}
			}
		}
	}
}

CString CHseAgingDlg::Lf_getLimitErrorString(int rack, int layer, int ch)
{
	CString retString;

	// Model 설정 Limit 정보를 가져온다.
	CString sModelName;
	float modelSetValue;

	m_cmbMaModelRack1.GetWindowText(sModelName);

	// Error 정보에 따라 Error String 을 생성한다.
	if (lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][ch] == ERR_INFO_VCC)
	{
		if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_HIGH)
		{
			Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("VCC_LIMIT_VOLT_HIGH"), &modelSetValue);
			retString.Format(_T("VCC HIGH Limit (Set:%.2fV, Meas:%.2fV)"), modelSetValue, (float)((float)lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][ch] / 100.f));
		}
		if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_LOW)
		{
			Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("VCC_LIMIT_VOLT_LOW"), &modelSetValue);
			retString.Format(_T("VCC LOW Limit (Set:%.2fV, Meas:%.2fV)"), modelSetValue, (float)((float)lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][ch] / 100.f));
		}
	}
	if (lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][ch] == ERR_INFO_ICC)
	{
		if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_HIGH)
		{
			Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("VCC_LIMIT_CURR_HIGH"), &modelSetValue);
			retString.Format(_T("ICC HIGH Limit (Set:%.2fA, Meas:%.2fA)"), modelSetValue, (float)((float)lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][ch] / 100.f));
		}
		if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_LOW)
		{
			Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("VCC_LIMIT_CURR_LOW"), &modelSetValue);
			retString.Format(_T("ICC LOW Limit (Set:%.2fA, Meas:%.2fA)"), modelSetValue, (float)((float)lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][ch] / 100.f));
		}
	}
	if (lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][ch] == ERR_INFO_VBL)
	{
		if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_HIGH)
		{
			Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("VBL_LIMIT_VOLT_HIGH"), &modelSetValue);
			retString.Format(_T("VBL HIGH Limit (Set:%.2fV, Meas:%.2fV)"), modelSetValue, (float)((float)lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][ch] / 100.f));
		}
		if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_LOW)
		{
			Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("VBL_LIMIT_VOLT_LOW"), &modelSetValue);
			retString.Format(_T("VBL LOW Limit (Set:%.2fV, Meas:%.2fV)"), modelSetValue, (float)((float)lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][ch] / 100.f));
		}
	}
	if (lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][ch] == ERR_INFO_IBL)
	{
		if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_HIGH)
		{
			Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("VBL_LIMIT_CURR_HIGH"), &modelSetValue);
			retString.Format(_T("IBL HIGH Limit (Set:%.2fA, Meas:%.2fA)"), modelSetValue, (float)((float)lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][ch] / 100.f));
		}
		if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_LOW)
		{
			Read_ModelFile(sModelName, _T("MODEL_INFO"), _T("VBL_LIMIT_CURR_LOW"), &modelSetValue);
			retString.Format(_T("IBL LOW Limit (Set:%.2fA, Meas:%.2fA)"), modelSetValue, (float)((float)lpInspWorkInfo->m_ast_AgingChErrorValue[rack][layer][ch] / 100.f));
		}
	}
	if (lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][ch] == ERR_INFO_TEMP)
	{
		retString.Format(_T("Temp Error. Rack = [%d], Layer =[%d], Ch = [%d]"), rack + 1, layer + 1, ch + 1);
	}

	return retString;
}

int CHseAgingDlg::Lf_getAlarmChannel(int layer, int ch)
{
	int retChannel = 0;

	for (int i = 0; i < MAX_LAYER; i++)
	{
		if (ch < 8)
		{
			retChannel = ch + (i * 8);
		}
		else
		{
			retChannel = ch + 32 * (i * 8);
		}
	}
// 
// 	if (layer == LAYER_1)
// 	{
// 		if (ch < 8)
// 			retChannel = ch;
// 		else
// 			retChannel = ch + 32;
// 	}
// 
// 	if (layer == LAYER_2)
// 	{
// 		if (ch < 8)
// 			retChannel = ch + 8;
// 		else
// 			retChannel = ch + 40;
// 	}
// 
// 	if (layer == LAYER_3)
// 	{
// 		if (ch < 8)
// 			retChannel = ch + 16;
// 		else
// 			retChannel = ch + 48;
// 	}
// 
// 	if (layer == LAYER_4)
// 	{
// 		if (ch < 8)
// 			retChannel = ch + 24;
// 		else
// 			retChannel = ch + 56;
// 	}
// 
// 	if (layer == LAYER_5)
// 	{
// 		if (ch < 8)
// 			retChannel = ch + 32;
// 		else
// 			retChannel = ch + 64;
// 	}

	return retChannel + 1;
}

void CHseAgingDlg::Lf_checkPowerLimitAlarm()
{
	CString limitAlarm = _T("");
	CString sLog, sdata, rackInfo, errString;
	int ch = 0;
	static bool tempErrorLogged[MAX_RACK] = { false };

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		for (int layer = 0; layer < MAX_LAYER; layer++)
		{
			// Channel 상태 Update. /Normal/Error
			for (ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
			{

				if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == TEMP_LOW)
				{
					if (lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][ch] == ERR_INFO_TEMP)
					{
						sLog.Format(_T("Temp Error. Rack = [%d], Layer =[%d], Ch = [%d]"), rack+1, layer+1, ch+1);
						Lf_writeRackMLog(rack, sLog);

						rackInfo.Format(_T("RACK[%d] CH[%d] - "), rack + 1, Lf_getAlarmChannel(layer, ch));
						errString = Lf_getLimitErrorString(rack, layer, ch);
						//Lf_writeRackMLog(rack, errString);

						sLog.Format(_T("<TEMP> TEMP ERROR NG. %s %s"), rackInfo, errString);
						m_pApp->Gf_writeMLog(sLog);

						//limitAlarm.Append(sdata);
						if (!tempErrorLogged[rack])
						{
							sdata.Format(_T("TEMP ERROR (RACK%d)"), rack+1);
							limitAlarm.Append(sdata);
							tempErrorLogged[rack] = true;
						}

						m_pApp->Gf_writeAlarmLog(rack, layer, ch, sdata);
						m_pApp->Gf_writeAlarmLog_RackOnly(rack, layer, ch, errString);

						//if (lpInspWorkInfo->m_sMesPanelID[rack][layer][ch].GetLength() != 0)
						//{
						//	//////////////////////////////////////////////////////////////////////////////////////////////////
						//	CString skey = _T(""), stime = _T("");
						//	CTime time = CTime::GetCurrentTime();
						//	stime.Format(_T("%02d:%02d:%02d"), time.GetHour(), time.GetMinute(), time.GetSecond());
						//	skey.Format(_T("RACK%d_LAYER%d_CH%d"), rack + 1, layer + 1, ch + 1);
						//	Write_SummaryInfo(_T("FAIL_MESSAGE"), skey, errString);
						//	Write_SummaryInfo(_T("FAIL_TIME"), skey, stime);
						//	//////////////////////////////////////////////////////////////////////////////////////////////////
						//}

						lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] = LIMIT_NONE;
						lpInspWorkInfo->m_ast_AgingChErrorType[rack][layer][ch] = ERR_INFO_NONE;
						//lpInspWorkInfo->m_ast_AgingTempError[rack][layer][ch] = TRUE; // Temp Error Agn_out 없애는 거
						continue;
					}
				}
				if (lpInspWorkInfo->m_nChErrorStatusOld[rack][layer][ch] == lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch])
					continue;

				if (lpInspWorkInfo->m_ast_AgingChErrorResult[rack][layer][ch] == LIMIT_NONE)
					continue;

				if (lpInspWorkInfo->m_PidTestError[rack] == true)
					continue;
				
				//////////////////////////////////////////////////////////////////////////////////////////////////
				// Power Limit 정보를 생성한다.
				//rackInfo.Format(_T("RACK[%d] LAYER[%d] CH[%d] - "), rack + 1, layer + 1, ch + 1);
				rackInfo.Format(_T("RACK[%d] CH[%d] - "), rack + 1, Lf_getAlarmChannel(layer, ch));
				errString = Lf_getLimitErrorString(rack, layer, ch);

				// UI Rack LOG에 기록한다.
				Lf_writeRackMLog(rack, errString);

				// MLog 에 상세 내용을 기록한다.
				sLog.Format(_T("<POWER> Power Limit NG. %s %s"), rackInfo, errString);
				m_pApp->Gf_writeMLog(sLog);

				//////////////////////////////////////////////////////////////////////////////////////////////////
				// Alarm을 발생 시키고, Log를 기록한다.
				sdata.Format(_T("%s  %s\r\n"), rackInfo, errString);
				limitAlarm.Append(sdata);

				// Alarm Log 를 기록한다.
				m_pApp->Gf_writeAlarmLog(rack, layer, ch, errString);
				m_pApp->Gf_writeAlarmLog_RackOnly(rack, layer, ch, errString);

				// 2024-05-30 PDH. PID가 없는 CH은 Limit NG 정보 Summary Log 에 기록되지 않도록 한다.
				if (lpInspWorkInfo->m_sMesPanelID[rack][layer][ch].GetLength() != 0)
				{
					//////////////////////////////////////////////////////////////////////////////////////////////////
					CString skey = _T(""), stime = _T("");
					CTime time = CTime::GetCurrentTime();
					stime.Format(_T("%02d:%02d:%02d"), time.GetHour(), time.GetMinute(), time.GetSecond());
					skey.Format(_T("RACK%d_LAYER%d_CH%d"), rack + 1, layer + 1, ch + 1);
					Write_SummaryInfo(_T("FAIL_MESSAGE"), skey, errString);
					Write_SummaryInfo(_T("FAIL_TIME"), skey, stime);
					//////////////////////////////////////////////////////////////////////////////////////////////////
				}

				//m_pApp->Gf_gmesSendHost(HOST_UNDO, rack, layer, ch);

				 //Tower Lamp Error
				lpInspWorkInfo->m_nAgingOperatingMode[rack] = AGING_ERROR;
				lpInspWorkInfo->m_AgingErrorRack = rack;
				m_pApp->pCommand->Gf_dio_setDIOWriteOutput(9, 1);
			}
		}
	}

	// Limit Alarm 메세지가 있고 Alarm 발생 상태가 아니면 Limit Alarm 을 발생시킨다.
	if (limitAlarm.GetLength() != 0)
	{
		if (lpInspWorkInfo->m_bAlarmOccur != TRUE)
		{
			lpInspWorkInfo->m_bAlarmOccur = TRUE;
			lpInspWorkInfo->m_sAlarmMessage.Format(_T("%s"), limitAlarm);
		}
	}

	memcpy(lpInspWorkInfo->m_nChErrorStatusOld, lpInspWorkInfo->m_ast_AgingChErrorResult, sizeof(lpInspWorkInfo->m_nChErrorStatusOld));
}

void CHseAgingDlg::Lf_getFirmawareVersion()
{
	Lf_updateFirmwareMatching();
}

//void CHseAgingDlg::Lf_updateFirmwareMatching()
//{
//#if 1
//	// 2025-01-17 PDH. 가장 빠른 날짜의 F/W 버전을 표시하기 위한 알고리즘으로 수정
//	for (int rack = 0; rack < MAX_RACK; rack++)
//	{
//		CString fwVersion, curVersion, preVersion;
//		CTime backupDate = 0;
//		BOOL bVerifyResult = TRUE;
//		for (int layer = 0; layer < MAX_LAYER; layer++)
//		{
//			// FW 버전이 Read 되어있지 않을 경우 Skip 한다.
//			if(lpInspWorkInfo->m_sMainFWVersion[rack][layer].GetLength() == 0)
//				continue;
//
//			// CString 시간 문자를 CTime 형으로 변경한다.
//			CTime layerDate;
//			CString strDate = lpInspWorkInfo->m_sMainFWVersion[rack][layer];
//			CString strYear, strMonth, strDay;
//			AfxExtractSubString(strYear, strDate, 0, '-');
//			AfxExtractSubString(strMonth, strDate, 1, '-');
//			AfxExtractSubString(strDay, strDate, 2, '-');
//			CTime tmDate(_ttoi(strYear), _ttoi(strMonth), _ttoi(strDay), 0, 0, 0);
//			layerDate = tmDate;
//
//			// 가장 마지막 FW 버전 정보를 표시한다.
//			if (layerDate > backupDate)
//			{
//				backupDate = layerDate;
//				fwVersion.Format(_T("%s"), lpInspWorkInfo->m_sMainFWVersion[rack][layer].GetString());
//			}
//
//			// 모든 FW 버전의 값이 같은지 비교한다.
//			if (preVersion.GetLength() == 0)
//			{
//				// 이전 Version 비교값이 없을 경우에 값을 넣는다.
//				preVersion = lpInspWorkInfo->m_sMainFWVersion[rack][layer];
//			}
//			else
//			{
//				if (preVersion != lpInspWorkInfo->m_sMainFWVersion[rack][layer])
//				{
//					bVerifyResult = FALSE;
//				}
//			}
//		}
//		lpInspWorkInfo->m_nFwVerifyResult[rack] = bVerifyResult;
//
//		// Firmware Version 변경되었으면 업데이트 한다.
//		m_pSttFWVersion[rack]->GetWindowText(curVersion);
//		if (fwVersion != curVersion)
//		{
//			m_pSttFWVersion[rack]->SetWindowText(fwVersion);
//			
//		}
//
//		// RACK의 모든 Layer F/W 버전이 같은지 확인한다.
//		m_pSttFWVersion[rack]->Invalidate(FALSE);
//		if (lpInspWorkInfo->m_nFwVerifyResult[rack] == FALSE)
//		{
//			CString sdata;
//			sdata.Format(_T("FirmWare Not Matching (RACK%d)"), rack + 1);
//			m_pApp->Gf_ShowMessageBox(sdata);
//		}
//	}
//#else
//	CString sMcuOld[MAX_RACK];
//
//	for (int rack = 0; rack < MAX_RACK; rack++)
//	{
//		for (int layer = 0; layer < MAX_LAYER; layer++)
//		{
//			if (lpInspWorkInfo->m_nMainEthConnect[rack][layer] == 0)
//				continue;
//
//			if (lpInspWorkInfo->m_sMainFWVersion[rack][layer].IsEmpty() == FALSE)
//				sMcuOld[layer].Format(_T("%s"), lpInspWorkInfo->m_sMainFWVersion[rack][layer]);
//		}
//	}
//
//	for (int rack = 0; rack < MAX_RACK; rack++)
//	{
//		for (int layer = 0; layer < MAX_LAYER; layer++)
//		{
//			if (lpInspWorkInfo->m_nMainEthConnect[rack][layer] == 0)
//				continue;
//
//			if (lpInspWorkInfo->m_sMainFWVersion[rack][layer] != sMcuOld[layer])
//				m_bMcuFwComapre[rack] = FALSE;
//			else
//				sMcuOld[rack] = lpInspWorkInfo->m_sMainFWVersion[rack][layer];
//		}
//		m_pSttFWVersion[rack]->SetWindowText(sMcuOld[rack].Left(16));
//	}
//#endif
//}

void CHseAgingDlg::Lf_updateFirmwareMatching()
{
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		CString fwVersion, curVersion, preVersion;
		CTime backupDate = 0;
		BOOL bVerifyResult = TRUE;

		for (int layer = 0; layer < MAX_LAYER; layer++)
		{
			if (lpInspWorkInfo->m_sMainFWVersion[rack][layer].GetLength() == 0)
				continue;

			// 날짜 파싱
			CString strDate = lpInspWorkInfo->m_sMainFWVersion[rack][layer];
			CString strYear, strMonth, strDay;
			AfxExtractSubString(strYear, strDate, 0, _T('-'));
			AfxExtractSubString(strMonth, strDate, 1, _T('-'));
			AfxExtractSubString(strDay, strDate, 2, _T('-'));
			CTime layerDate(_ttoi(strYear), _ttoi(strMonth), _ttoi(strDay), 0, 0, 0);

			if (layerDate > backupDate)
			{
				backupDate = layerDate;
				fwVersion = lpInspWorkInfo->m_sMainFWVersion[rack][layer];
			}

			if (preVersion.IsEmpty())
				preVersion = lpInspWorkInfo->m_sMainFWVersion[rack][layer];
			else if (preVersion != lpInspWorkInfo->m_sMainFWVersion[rack][layer])
				bVerifyResult = FALSE;
		}

		lpInspWorkInfo->m_nFwVerifyResult[rack] = bVerifyResult;

		// 표시 업데이트
		m_pSttFWVersion[rack]->GetWindowText(curVersion);
		if (fwVersion != curVersion)
			m_pSttFWVersion[rack]->SetWindowText(fwVersion);

		m_pSttFWVersion[rack]->Invalidate(FALSE);

		// ★ 알림 로직: 불일치 시 1회만 알림, 일치하면 플래그 리셋
		if (bVerifyResult == FALSE)
		{
			if (m_bFwMismatchNotified[rack] == FALSE)
			{
				CString sdata;
				sdata.Format(_T("FirmWare Not Matching (RACK%d)"), rack + 1);
				m_pApp->Gf_ShowMessageBox(sdata);
				m_bFwMismatchNotified[rack] = TRUE; // 이제 알림했음
			}
		}
		else
		{
			// 일치하면 다음 번 불일치에서 다시 알리도록 플래그 초기화
			m_bFwMismatchNotified[rack] = FALSE;
		}
	}
}

BOOL CHseAgingDlg::Lf_checkAgingIDLEMode()
{
	CString sdata;
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		if (lpInspWorkInfo->m_nAgingOperatingMode[rack] != AGING_IDLE)
		{ 
			sdata.Format(_T("'RACK-%d' is not IDLE mode. Please stop Aging !!"), rack + 1);
			m_pApp->Gf_ShowMessageBox(sdata);
			return FALSE;
		}
	}

	return TRUE;
}

void CHseAgingDlg::Lf_toggleChUseUnuse(int rack, int layer, int ch)
{
	if (lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] == CHANNEL_USE)
		lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] = CHANNEL_UNUSE;
	else
		lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] = CHANNEL_USE;

	m_pSttRackState[rack][layer][ch]->Invalidate(FALSE);
}

//void CHseAgingDlg::Lf_setChannelUseUnuse(int rack)
//{
//	CString ip;
//	BOOL setInfo[16];
//
//	m_pBtnChUseUnuseSet[rack]->EnableWindow(FALSE);
//
//	for (int layer = 0; layer < MAX_LAYER; layer++)
//	{
//		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
//		{
//			// ch == 7일 때만 Use, 나머지는 Unuse (0-based index)
//			setInfo[ch] = (ch == 8) ? CHANNEL_USE : CHANNEL_UNUSE;
//
//			// 옵션: 내부 상태도 같이 변경 (lpInspWorkInfo도 반영)
//			lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] = setInfo[ch];
//		}
//
//		ip.Format(_T("192.168.10.%d"), (rack * 5) + layer + 1);
//		m_pApp->pCommand->Gf_setChannelUseUnuse(ip, setInfo);
//	}
//
//	m_pBtnChUseUnuseSet[rack]->EnableWindow(TRUE);
//}

void CHseAgingDlg::Lf_setChannelUseUnuse(int rack)
{
	//CString ip;
	//BOOL setInfo[16];

	//m_pBtnChUseUnuseSet[rack]->EnableWindow(FALSE);

	//for (int layer = 0; layer < MAX_LAYER; layer++)
	//{
	//	for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
	//	{
	//		// 모든 채널을 사용 상태로 설정
	//		setInfo[ch] = CHANNEL_USE;

	//		// 내부 상태도 같이 반영
	//		lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch] = CHANNEL_USE;
	//	}

	//	ip.Format(_T("192.168.10.%d"), (rack * 5) + layer + 1);
	//	m_pApp->pCommand->Gf_setChannelUseUnuse(ip, setInfo);
	//}

	//m_pBtnChUseUnuseSet[rack]->EnableWindow(TRUE);
	CString ip;
	BOOL setInfo[16];

	m_pBtnChUseUnuseSet[rack]->EnableWindow(FALSE);

	for (int layer = 0; layer < MAX_LAYER; layer++)
	{
		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
		{
			setInfo[ch] = lpInspWorkInfo->m_ast_ChUseUnuse[rack][layer][ch];
		}
		ip.Format(_T("192.168.10.%d"), (rack * 5) + layer + 1);
		m_pApp->pCommand->Gf_setChannelUseUnuse(ip, setInfo);
	}

	m_pBtnChUseUnuseSet[rack]->EnableWindow(TRUE);
}

void CHseAgingDlg::Lf_channelUseButtonShowHide(int rack)
{
	if (m_pChkChSelect[rack]->GetCheck() == TRUE)
	{
		m_pBtnAgingStart[rack]->ShowWindow(SW_HIDE);
		m_pBtnAgingStop[rack]->ShowWindow(SW_HIDE);
		m_pBtnAgingFusing[rack]->ShowWindow(SW_HIDE);
		m_pBtnChUseUnuseSet[rack]->ShowWindow(SW_SHOWNORMAL);
	}
	else
	{
		m_pBtnAgingStart[rack]->ShowWindow(SW_SHOWNORMAL);
		m_pBtnAgingStop[rack]->ShowWindow(SW_SHOWNORMAL);
		m_pBtnAgingFusing[rack]->ShowWindow(SW_SHOWNORMAL);
		m_pBtnChUseUnuseSet[rack]->ShowWindow(SW_HIDE);
	}
}


BOOL CHseAgingDlg::Lf_checkCableOpen(int rack)
{
	CString sLog, sdata, sMessage;
	
	// Cable Open Check 옵션이 OFF 되어있으면 TRUE Return 한다.
	if (lpModelInfo->m_nFuncCableOpen == FALSE)
		return TRUE;

	CCableOpen cable_dlg;
	cable_dlg.m_nRackNo = rack;
	cable_dlg.DoModal();

	// Cable Open 은 모든 채널에 패널이 걸려있지 않을 수 있기에 NG 일 경우에도 무조건 TRUE로 Return 한다.
	return TRUE;
}

void CHseAgingDlg::Lf_checkDoorOpenClose()
{
	CString strMsg;
	BOOL bInvalidate = FALSE;

	for (int i = 0; i < DOOR_MAX; i++)
	{
		if (lpInspWorkInfo->m_nDioInputData[i + DIO_IN_DOOR1] != lpInspWorkInfo->m_nDoorOpenClose[i])
		{
			lpInspWorkInfo->m_nDoorOpenClose[i] = lpInspWorkInfo->m_nDioInputData[i + DIO_IN_DOOR1];
			bInvalidate = TRUE;

			// DOOR Open/Close 상태값을 UI에 표시
			if (lpInspWorkInfo->m_nDoorOpenClose[i] == DOOR_OPEN)
				strMsg.Format(_T("DOOR%d OPEN"), i + 1);
			else
				strMsg.Format(_T("DOOR%d CLOSE"), i + 1);

			m_pDoorState[i]->SetWindowText(strMsg);
		}
	}

	// Data 변경사항 있을 경우에만 UI Update 진행한다.
	if (bInvalidate == TRUE)
	{
		GetDlgItem(IDC_STT_MA_DOOR1)->Invalidate(FALSE);
		GetDlgItem(IDC_STT_MA_DOOR2)->Invalidate(FALSE);
		GetDlgItem(IDC_STT_MA_DOOR3)->Invalidate(FALSE);
		GetDlgItem(IDC_STT_MA_DOOR4)->Invalidate(FALSE);
		GetDlgItem(IDC_STT_MA_DOOR5)->Invalidate(FALSE);
		GetDlgItem(IDC_STT_MA_DOOR6)->Invalidate(FALSE);
	}
}

//void CHseAgingDlg::Lf_checkBcrRackIDInput()
//{
//	// Key In Data에 RACKID가 검출되면 해당 RACK PID 입력창으로 이동시킨다.
//	int selRackId = -1;
//	CString rack1_id, rack2_id, rack3_id, rack4_id, rack5_id, rack6_id;
//	Read_SysIniFile(_T("SYSTEM"), _T("RACK1_BCR_ID"), &rack1_id);
//	Read_SysIniFile(_T("SYSTEM"), _T("RACK2_BCR_ID"), &rack2_id);
//	Read_SysIniFile(_T("SYSTEM"), _T("RACK3_BCR_ID"), &rack3_id);
//	Read_SysIniFile(_T("SYSTEM"), _T("RACK4_BCR_ID"), &rack4_id);
//	Read_SysIniFile(_T("SYSTEM"), _T("RACK5_BCR_ID"), &rack5_id);
//	Read_SysIniFile(_T("SYSTEM"), _T("RACK6_BCR_ID"), &rack6_id);
//
//	if (rack1_id.GetLength() != 0)
//	{
//		if (m_nMainKeyInData.Find(rack1_id) != -1)		selRackId = RACK_1;
//	}
//	if (rack2_id.GetLength() != 0)
//	{
//		if (m_nMainKeyInData.Find(rack2_id) != -1)		selRackId = RACK_2;
//	}
//	if (rack3_id.GetLength() != 0)
//	{
//		if (m_nMainKeyInData.Find(rack3_id) != -1)		selRackId = RACK_3;
//	}
//	if (rack4_id.GetLength() != 0)
//	{
//		if (m_nMainKeyInData.Find(rack4_id) != -1)		selRackId = RACK_4;
//	}
//	if (rack5_id.GetLength() != 0)
//	{
//		if (m_nMainKeyInData.Find(rack5_id) != -1)		selRackId = RACK_5;
//	}
//	if (rack6_id.GetLength() != 0)
//	{
//		if (m_nMainKeyInData.Find(rack6_id) != -1)		selRackId = RACK_6;
//	}
//
//	if (selRackId != -1)
//	{
//		m_nMainKeyInData.Empty();
//
//		AfxGetApp()->GetMainWnd()->SendMessage(WM_BCR_RACK_ID_INPUT, (WPARAM)selRackId, NULL);
//	}
//}

void CHseAgingDlg::Lf_checkBcrRackIDInput()
{

	int Value = 0;

	// "RACK03CH02"에서 "3"을 추출하기 위해 필요한 인덱스를 계산
	int rackIndex = 5; // "RACK" 다음에 오는 숫자의 시작 인덱스
	CString Right_Data = m_nSendKeyInData.Right(6);
	CString Left_Data = Right_Data.Left(2);

	// 숫자를 문자로 가져오기
	if (m_nSendKeyInData.GetLength() > rackIndex)
	{
		CString numberStr;
		numberStr.Format(_T("%c"), m_nSendKeyInData[rackIndex]); // '3'를 가져옴

		// CString을 int로 변환
		Value = _ttoi(numberStr); // 숫자로 변환
	}

	lpInspWorkInfo->m_ChID = m_nSendKeyInData.Right(2);
	lpInspWorkInfo->m_RackID = Left_Data;

	m_nUseKeyInData = true;
	lpInspWorkInfo->m_SendRackID = true;

	//OnBnClickedBtnMaPidInput();

		AfxGetApp()->GetMainWnd()->SendMessage(WM_BCR_RACK_ID_INPUT, (WPARAM)Value-1, NULL);
}

void CHseAgingDlg::Lf_writeSensingLog()
{
	// Sensing Log Write 기능으 OFF 이면 Retrun 시킨다.
	/*if (lpSystemInfo->m_nSensingLogInterval == 0)
		return;*/

	if (lpSystemInfo->m_nSensingLogInterval != 0)
	{
		CTime time = CTime::GetCurrentTime();
		int currenthour = time.GetHour();
		int currentminute = time.GetMinute();

		if (currenthour != m_nTempLogWriteHour ||
			(currentminute - m_nTempLogWriteMin) >= lpSystemInfo->m_nSensingLogInterval)
		{
			//m_nSensingLogWriteHour = currenthour;
			//m_nSensingLogWriteMin = currentminute;

			//// Sensing Log 기록한다.
			//FILE* fp;
			//char filepath[128] = { 0 };
			//char buff[256] = { 0 };
			//char dataline[1024] = { 0 };

			//sprintf_s(filepath, "./Logs/SensingLog/SensingLog_%04d%02d%02d.csv", time.GetYear(), time.GetMonth(), time.GetDay());
			//fopen_s(&fp, filepath, "r+");
			//if (fp == NULL)
			//{
			//	if ((_access("./Logs/SensingLog", 0)) == -1)
			//		_mkdir("./Logs/SensingLog");

			//	delayMs(1);
			//	fopen_s(&fp, filepath, "a+");
			//	if (fp == NULL)
			//	{
			//		if ((_access(filepath, 2)) != -1)
			//		{
			//			delayMs(1);
			//			fopen_s(&fp, filepath, "a+");
			//			if (fp == NULL)
			//			{
			//				return;
			//			}
			//		}
			//	}
			//	sprintf_s(buff, "TIME,RACK,LAYER,CH,PID,VCC,ICC,VBL,IBL,TEMP\n");
			//	fprintf(fp, "%s", buff);
			//}

			//fseek(fp, 0L, SEEK_END);

			//char timeKey[100] = { 0, };
			//char panelID[100] = { 0, };
			//sprintf_s(timeKey, "%02d:%02d:%02d", time.GetHour(), time.GetMinute(), time.GetSecond());
			//for (int rack = 0; rack < MAX_RACK; rack++)
			//{
			//	for (int layer = 0; layer < MAX_LAYER; layer++)
			//	{
			//		for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
			//		{
			//			sprintf_s(panelID, "%S", lpInspWorkInfo->m_sMesPanelID[rack][layer][ch].GetString());

			//			sprintf_s(buff, "%s,%d,%d,%d,%s,%.2fV,%.2fA,%.2fV,%.2fV,%.1f",
			//				timeKey,
			//				rack + 1,
			//				layer + 1,
			//				ch + 1,
			//				panelID,
			//				(float)lpInspWorkInfo->m_nMeasVCC[rack][layer][ch] / 100.0f,
			//				(float)lpInspWorkInfo->m_nMeasICC[rack][layer][ch] / 100.0f,
			//				(float)lpInspWorkInfo->m_nMeasVBL[rack][layer][ch] / 100.0f,
			//				(float)lpInspWorkInfo->m_nMeasIBL[rack][layer][ch] / 100.0f,
			//				(lpInspWorkInfo->m_fTempReadVal[rack])
			//			);

			//			char* pos = dataline;
			//			sprintf_s(dataline, "%s\n", buff);
			//			fprintf(fp, "%s", pos);

			//			// Summary 및 실처리 전송을 위한 Min/Max/Avg 값을 저장한다.
			//			Lf_savePowerMeasureMinMax(rack, layer, ch);
			//		}
			//	}
			//	lpInspWorkInfo->m_nAgingPowerMeasCount[rack] = lpInspWorkInfo->m_nAgingPowerMeasCount[rack] + 1;
			//}

			//fclose(fp);
	}

	// 시간을 계산하여 Sensing Log Write Interval 시간이 되면 로그를 기록한다.
	/*CTime time = CTime::GetCurrentTime();
	int currenthour = time.GetHour();
	int currentminute = time.GetMinute();*/
	//if ((time.GetMinute() - m_nSensingLogWriteMin) < lpSystemInfo->m_nSensingLogInterval) // m_nSensingLogWriteMin : 현재시간, m_nSensingLogInterval : 설정시간
	//{
	//	return;
	//}
	//if (currenthour != m_nSensingLogWriteHour ||
	//	(currentminute - m_nSensingLogWriteMin) >= lpSystemInfo->m_nSensingLogInterval)
	//{
	//	m_nSensingLogWriteHour = currenthour;
	//	m_nSensingLogWriteMin = currentminute;

	//	// Sensing Log 기록한다.
	//	FILE* fp;
	//	char filepath[128] = { 0 };
	//	char buff[256] = { 0 };
	//	char dataline[1024] = { 0 };

	//	sprintf_s(filepath, "./Logs/SensingLog/SensingLog_%04d%02d%02d.csv", time.GetYear(), time.GetMonth(), time.GetDay());
	//	fopen_s(&fp, filepath, "r+");
	//	if (fp == NULL)
	//	{
	//		if ((_access("./Logs/SensingLog", 0)) == -1)
	//			_mkdir("./Logs/SensingLog");

	//		delayMs(1);
	//		fopen_s(&fp, filepath, "a+");
	//		if (fp == NULL)
	//		{
	//			if ((_access(filepath, 2)) != -1)
	//			{
	//				delayMs(1);
	//				fopen_s(&fp, filepath, "a+");
	//				if (fp == NULL)
	//				{
	//					return;
	//				}
	//			}
	//		}
	//		sprintf_s(buff, "TIME,RACK,LAYER,CH,PID,VCC,ICC,VBL,IBL,TEMP\n");
	//		fprintf(fp, "%s", buff);
	//	}

	//	fseek(fp, 0L, SEEK_END);

	//	char timeKey[100] = { 0, };
	//	char panelID[100] = { 0, };
	//	sprintf_s(timeKey, "%02d:%02d:%02d", time.GetHour(), time.GetMinute(), time.GetSecond());
	//	for (int rack = 0; rack < MAX_RACK; rack++)
	//	{
	//		for (int layer = 0; layer < MAX_LAYER; layer++)
	//		{
	//			for (int ch = 0; ch < MAX_LAYER_CHANNEL; ch++)
	//			{
	//				sprintf_s(panelID, "%S", lpInspWorkInfo->m_sMesPanelID[rack][layer][ch].GetString());

	//				sprintf_s(buff, "%s,%d,%d,%d,%s,%.2fV,%.2fA,%.2fV,%.2fV,%.1f",
	//					timeKey,
	//					rack + 1,
	//					layer + 1,
	//					ch + 1,
	//					panelID,
	//					(float)lpInspWorkInfo->m_nMeasVCC[rack][layer][ch] / 100.0f,
	//					(float)lpInspWorkInfo->m_nMeasICC[rack][layer][ch] / 100.0f,
	//					(float)lpInspWorkInfo->m_nMeasVBL[rack][layer][ch] / 100.0f,
	//					(float)lpInspWorkInfo->m_nMeasIBL[rack][layer][ch] / 100.0f,
	//					(lpInspWorkInfo->m_fTempReadVal[rack])
	//				);

	//				char* pos = dataline;
	//				sprintf_s(dataline, "%s\n", buff);
	//				fprintf(fp, "%s", pos);

	//				// Summary 및 실처리 전송을 위한 Min/Max/Avg 값을 저장한다.
	//				Lf_savePowerMeasureMinMax(rack, layer, ch);
	//			}
	//		}
	//		lpInspWorkInfo->m_nAgingPowerMeasCount[rack] = lpInspWorkInfo->m_nAgingPowerMeasCount[rack] + 1;
	//	}

	//	fclose(fp);
	}
}

void CHseAgingDlg::Lf_checkComplete5MinOver()
{
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		if (m_nAgingStart[rack] == FALSE)
			continue;

		if (lpInspWorkInfo->m_nAgingEndWaitTime[rack] == 0)
			continue;

		if (lpInspWorkInfo->m_nAgingOperatingMode[rack] != AGING_RUNNING)
			continue;

		int setSec, runSec;
		setSec = lpInspWorkInfo->m_nAgingSetTime[rack];
		runSec = lpInspWorkInfo->m_nAgingRunTime[rack];

		// Run 시간이 Set 시간보다 5분 경과했을 경우 Alarm 발생
		if ((runSec - setSec) > lpInspWorkInfo->m_nAgingEndWaitTime[rack])
		{
			if (lpInspWorkInfo->m_bAlarmOccur != TRUE)
			{
				lpInspWorkInfo->m_bAlarmOccur = TRUE;
				lpInspWorkInfo->m_sAlarmMessage.Format(_T("RACK[%d] AGING Completed %d minutes have passed."), rack+1, lpInspWorkInfo->m_nAgingEndWaitTime[rack]);
			}
		}
	}
}

void CHseAgingDlg::Lf_savePowerMeasureMinMax(int rack, int layer, int ch)
{
	float measVCC = (float)lpInspWorkInfo->m_nMeasVCC[rack][layer][ch] / 100.0f;
	float measICC = (float)lpInspWorkInfo->m_nMeasICC[rack][layer][ch] / 100.0f;
	float measVBL = (float)lpInspWorkInfo->m_nMeasVBL[rack][layer][ch] / 100.0f;
	float measIBL = (float)lpInspWorkInfo->m_nMeasIBL[rack][layer][ch] / 100.0f;


	// VCC Min Value Save
	if (lpInspWorkInfo->m_fOpeAgingVccMin[rack][layer][ch] == 0)
	{
		lpInspWorkInfo->m_fOpeAgingVccMin[rack][layer][ch] = measVCC;
	}
	else
	{
		if (lpInspWorkInfo->m_fOpeAgingVccMin[rack][layer][ch] > measVCC)
		{
			lpInspWorkInfo->m_fOpeAgingVccMin[rack][layer][ch] = measVCC;
		}
	}

	// VCC Max Value Save
	if (lpInspWorkInfo->m_fOpeAgingVccMax[rack][layer][ch] < measVCC)
	{
		lpInspWorkInfo->m_fOpeAgingVccMax[rack][layer][ch] = measVCC;
	}

	// ICC Min Value Save
	if (lpInspWorkInfo->m_fOpeAgingIccMin[rack][layer][ch] == 0)
	{
		lpInspWorkInfo->m_fOpeAgingIccMin[rack][layer][ch] = measICC;
	}
	else
	{
		if (lpInspWorkInfo->m_fOpeAgingIccMin[rack][layer][ch] > measICC)
		{
			lpInspWorkInfo->m_fOpeAgingIccMin[rack][layer][ch] = measICC;
		}
	}

	// ICC Max Value Save
	if (lpInspWorkInfo->m_fOpeAgingIccMax[rack][layer][ch] < measICC)
	{
		lpInspWorkInfo->m_fOpeAgingIccMax[rack][layer][ch] = measICC;
	}

	// VBL Min Value Save
	if (lpInspWorkInfo->m_fOpeAgingVblMin[rack][layer][ch] == 0)
	{
		lpInspWorkInfo->m_fOpeAgingVblMin[rack][layer][ch] = measVBL;
	}
	else
	{
		if (lpInspWorkInfo->m_fOpeAgingVblMin[rack][layer][ch] > measVBL)
		{
			lpInspWorkInfo->m_fOpeAgingVblMin[rack][layer][ch] = measVBL;
		}
	}

	// VBL Max Value Save
	if (lpInspWorkInfo->m_fOpeAgingVblMax[rack][layer][ch] < measVBL)
	{
		lpInspWorkInfo->m_fOpeAgingVblMax[rack][layer][ch] = measVBL;
	}

	// IBL Min Value Save
	if (lpInspWorkInfo->m_fOpeAgingIblMin[rack][layer][ch] == 0)
	{
		lpInspWorkInfo->m_fOpeAgingIblMin[rack][layer][ch] = measIBL;
	}
	else
	{
		if (lpInspWorkInfo->m_fOpeAgingIblMin[rack][layer][ch] > measIBL)
		{
			lpInspWorkInfo->m_fOpeAgingIblMin[rack][layer][ch] = measIBL;
		}
	}

	// IBL Max Value Save
	if (lpInspWorkInfo->m_fOpeAgingIblMax[rack][layer][ch] < measIBL)
	{
		lpInspWorkInfo->m_fOpeAgingIblMax[rack][layer][ch] = measIBL;
	}


	// AVG 계산을 위한 Data 누적. AVG 변수에 측정값을 누적하고 Complete 시점에 Count 값으로 나눈다
	/*lpInspWorkInfo->m_fOpeAgingVccAvg[rack][layer][ch] = lpInspWorkInfo->m_fOpeAgingVccAvg[rack][layer][ch] + measVCC;
	lpInspWorkInfo->m_fOpeAgingIccAvg[rack][layer][ch] = lpInspWorkInfo->m_fOpeAgingIccAvg[rack][layer][ch] + measICC;
	lpInspWorkInfo->m_fOpeAgingVblAvg[rack][layer][ch] = lpInspWorkInfo->m_fOpeAgingVblAvg[rack][layer][ch] + measVBL;
	lpInspWorkInfo->m_fOpeAgingIblAvg[rack][layer][ch] = lpInspWorkInfo->m_fOpeAgingIblAvg[rack][layer][ch] + measIBL;*/
}

void CHseAgingDlg::Lf_flickerCompleteRackNumber()
{
	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		//if (lpInspWorkInfo->m_nAgingOperatingMode[rack] == AGING_COMPLETE)
		if(lpInspWorkInfo->m_nAgingStatusS[rack] == 1)
		{
			Lf_updateTowerLamp();
			if (m_pSttRackNo[rack]->IsWindowVisible() == TRUE)
			{
				m_pSttRackNo[rack]->ShowWindow(SW_HIDE);
			}
			else
			{
				m_pSttRackNo[rack]->ShowWindow(SW_SHOWNORMAL);
			}
		}
		else
		{
			if (m_pSttRackNo[rack]->IsWindowVisible() == FALSE)
			{
				m_pSttRackNo[rack]->ShowWindow(SW_SHOWNORMAL);
			}
		}
	}
}

void CHseAgingDlg::OnBnClickedButtonDoor1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//Lf_setDoorOnOff(RACK_2);
	lpInspWorkInfo->TempTest += 1;

}


void CHseAgingDlg::OnBnClickedButtonDoor2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//Lf_setDoorOnOff(RACK_1);
	lpInspWorkInfo->TempTest -= 1;
}


void CHseAgingDlg::OnBnClickedButtonDoor3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//Lf_setDoorOnOff(RACK_3);
	//m_pApp->Gf_gmesSendHost(HOST_ERCP, NULL, NULL, NULL);

	m_dlgErcpTest.DoModal();

	//Lf_rmsErcpSet();
}


void CHseAgingDlg::OnBnClickedButtonDoor4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//Lf_setDoorOnOff(RACK_4);
	Lf_rmsErcpSet();
}


void CHseAgingDlg::OnBnClickedButtonDoor5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setDoorOnOff(RACK_5);
}


void CHseAgingDlg::OnBnClickedButtonDoor6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	Lf_setDoorOnOff(RACK_6);
}

void CHseAgingDlg::Lf_AgingProgressLog()
{
	CString Aging_Count, Aging_Ng_Count, Aging_Month_Count, Aging_Month_Ng_Count;
	Aging_Count.Format(_T("%d"), lpSystemInfo->m_Aging_Count);
	GetDlgItem(IDC_STT_MA_AGING_COUNT)->SetWindowText(Aging_Count);
	Aging_Ng_Count.Format(_T("%d"), lpSystemInfo->m_Aging_Ng_Count);
	GetDlgItem(IDC_STT_MA_AGING_NG_COUNT)->SetWindowText(Aging_Ng_Count);
	Aging_Month_Count.Format(_T("%d"), lpSystemInfo->m_Aging_Month_Count);
	GetDlgItem(IDC_STT_MA_MONTH_AGING_COUNT)->SetWindowText(Aging_Month_Count);
	Aging_Month_Ng_Count.Format(_T("%d"), lpSystemInfo->m_Aging_Month_Ng_Count);
	GetDlgItem(IDC_STT_MA_MONTH_AGING_NG_COUNT)->SetWindowText(Aging_Month_Ng_Count);

	for (int i = 0; i < MAX_RACK; i++)
	{
		if (m_nAgingStart[i] == TRUE)
		{
			m_nProgressGauge[i] = m_pCtrMaProgress[i]->GetPos();

			// 현재 진행률에 따른 구간 번호 계산
			int nSection = m_nProgressGauge[i] / 20;  // 0~5까지

			// 새로운 구간에 진입했는지 확인
			if (nSection != m_nLastLoggerdProgress[i] && nSection >= 1 && nSection <= 5)
			{
				CString sLog;
				sLog.Format(_T("<MESSAGE> Aging Progress Rate - [%d%%] (RACK %d)"),
					m_nProgressGauge[i], (i + 1));

				m_pApp->Gf_writeMLog(sLog);

				m_nLastLoggerdProgress[i] = nSection;

				//m_pApp->Gf_gmesSendHost(HOST_RMSO, NULL, NULL, NULL); // RMS
			}
		}
	}
}

void CHseAgingDlg::Lf_rmsErcpSet()
{
	CString ErcpDataSet, setPW, ErcpMessageSet;
	CString rackKeys[] = {
	_T("RACK1_BCR_ID"),
	_T("RACK2_BCR_ID"),
	_T("RACK3_BCR_ID"),
	_T("RACK4_BCR_ID"),
	_T("RACK5_BCR_ID"),
	_T("RACK6_BCR_ID")
	};

	//ErcpDataSet.Format(_T("MODEL_NB#1^"));																				ErcpMessageSet += ErcpDataSet;
	//ErcpDataSet.Format(_T("%s_D10001#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sEqpName);					ErcpMessageSet += ErcpDataSet; // EQP
	//ErcpDataSet.Format(_T("%s_D10002#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sLastModelName[0]);		ErcpMessageSet += ErcpDataSet;	// LastModelName[1]
	//ErcpDataSet.Format(_T("%s_D10003#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sLastModelName[1]);		ErcpMessageSet += ErcpDataSet;
	//ErcpDataSet.Format(_T("%s_D10004#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sLastModelName[2]);		ErcpMessageSet += ErcpDataSet;
	//ErcpDataSet.Format(_T("%s_D10005#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sLastModelName[3]);		ErcpMessageSet += ErcpDataSet;
	//ErcpDataSet.Format(_T("%s_D10006#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sLastModelName[4]);		ErcpMessageSet += ErcpDataSet;
	//ErcpDataSet.Format(_T("%s_D10007#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sLastModelName[5]);		ErcpMessageSet += ErcpDataSet;
	//ErcpDataSet.Format(_T("%s_D10008#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_nTempRecorderPort);		ErcpMessageSet += ErcpDataSet; // TEMPRECORDERPORT
	//Read_SysIniFile(_T("SYSTEM"), _T("PASSWORD"), &setPW);																ErcpMessageSet += ErcpDataSet;
	//ErcpDataSet.Format(_T("%s_D10009#%s^"), lpSystemInfo->m_sEqpName.Right(6), setPW);									ErcpMessageSet += ErcpDataSet; // PASSWORD
	//ErcpDataSet.Format(_T("%s_D10010#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sChamberNo);				ErcpMessageSet += ErcpDataSet; // CHAMBER NUMBER
	//for (int i = 0; i < 6; ++i)
	//{
	//	CString rackVal;
	//	// INI 파일에서 각 RACK 값을 읽음
	//	Read_SysIniFile(_T("SYSTEM"), rackKeys[i], &rackVal);

	//	// 기존 문자열에 누적 추가 (Format 대신 Append)
	//	CString temp;
	//	temp.Format(_T(" %s=%s"), rackKeys[i], rackVal); // RACK1_BCR_ID
	//	//ErcpDataSet += temp;
	//	ErcpMessageSet += temp;
	//}

	//ErcpDataSet.Format(_T("%s_D10011#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_fRefreshAgingStatusTime);		ErcpMessageSet += ErcpDataSet; // REFRESH_AGING_STATUS_TIME
	//ErcpDataSet.Format(_T("%s_D10012#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_fRefreshPowerMeasureTime);		ErcpMessageSet += ErcpDataSet; // REFRESH_POWER_MEASURE_TIME
	//ErcpDataSet.Format(_T("%s_D10013#%i^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_nTempLogInterval);				ErcpMessageSet += ErcpDataSet; // TEMP_LOG_INTERVAL
	//ErcpDataSet.Format(_T("%s_D10014#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sMesServicePort);				ErcpMessageSet += ErcpDataSet; // MES_SERVICE_PORT
	//ErcpDataSet.Format(_T("%s_D10015#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sMesNetWork);					ErcpMessageSet += ErcpDataSet; // MES_NETWORK
	//ErcpDataSet.Format(_T("%s_D10016#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sMesDaemonPort);					ErcpMessageSet += ErcpDataSet; // MES_DAEMON_PORT
	//ErcpDataSet.Format(_T("%s_D10017#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sMesLocalSubject);				ErcpMessageSet += ErcpDataSet; // MES_LOCAL_SUBJECT
	//ErcpDataSet.Format(_T("%s_D10018#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sMesRemoteSubject);				ErcpMessageSet += ErcpDataSet; // MES_REMOTE_SUBJECT
	//ErcpDataSet.Format(_T("%s_D10019#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sEasServicePort);				ErcpMessageSet += ErcpDataSet; // EAS_SERVICE_PORT
	//ErcpDataSet.Format(_T("%s_D10020#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sEasNetWork);					ErcpMessageSet += ErcpDataSet; // EAS_NETWORK
	//ErcpDataSet.Format(_T("%s_D10021#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sEasDaemonPort);					ErcpMessageSet += ErcpDataSet; // EAS_DAEMON_PORT
	//ErcpDataSet.Format(_T("%s_D10022#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sEasLocalSubject);				ErcpMessageSet += ErcpDataSet; // EAS_LOCAL_SUBJECT
	//ErcpDataSet.Format(_T("%s_D10023#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sEasRemoteSubject);				ErcpMessageSet += ErcpDataSet; // EAS_REMOTE_SUBJECT
	//ErcpDataSet.Format(_T("%s_D10024#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sRmsServicePort);				ErcpMessageSet += ErcpDataSet; // RMS_SERVICE_PORT
	//ErcpDataSet.Format(_T("%s_D10025#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sRmsNetWork);					ErcpMessageSet += ErcpDataSet; // RMS_NETWORK
	//ErcpDataSet.Format(_T("%s_D10026#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sRmsDaemonPort);					ErcpMessageSet += ErcpDataSet; // RMS_DAEMON_PORT
	//ErcpDataSet.Format(_T("%s_D10027#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sRmsLocalSubject);				ErcpMessageSet += ErcpDataSet; // RMS_LOCAL_SUBJECT
	//ErcpDataSet.Format(_T("%s_D10028#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sRmsRemoteSubject);				ErcpMessageSet += ErcpDataSet; // RMS_REMOTE_SUBJECT
	//ErcpDataSet.Format(_T("%s_D10029#%s^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_sRmsEqp);						ErcpMessageSet += ErcpDataSet; // RMS_EQP
	//ErcpDataSet.Format(_T("%s_D10030#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_Aging_Count);					ErcpMessageSet += ErcpDataSet; // AGING_COUNT
	//ErcpDataSet.Format(_T("%s_D10031#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpSystemInfo->m_Aging_Ng_Count);					ErcpMessageSet += ErcpDataSet; // AGING_NG_COUNT

	//ErcpDataSet.Format(_T("%s_D10032#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nDimmingSel);						ErcpMessageSet += ErcpDataSet; // DIMMING_SEL
	//ErcpDataSet.Format(_T("%s_D10033#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPwmFreq);						ErcpMessageSet += ErcpDataSet; // PWM_FREQ
	//ErcpDataSet.Format(_T("%s_D10034#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPwmDuty);						ErcpMessageSet += ErcpDataSet; // PWM_DUTY
	//ErcpDataSet.Format(_T("%s_D10035#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVbrVolt);						ErcpMessageSet += ErcpDataSet; // VBR_VOLT
	//ErcpDataSet.Format(_T("%s_D10036#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nFuncCableOpen);					ErcpMessageSet += ErcpDataSet; // CABLE_OPEN
	//ErcpDataSet.Format(_T("%s_D10037#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq1);					ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ1
	//ErcpDataSet.Format(_T("%s_D10038#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq2);					ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ2
	//ErcpDataSet.Format(_T("%s_D10039#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq3);					ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ3
	//ErcpDataSet.Format(_T("%s_D10040#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq4);					ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ4
	//ErcpDataSet.Format(_T("%s_D10041#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq5);					ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ5
	//ErcpDataSet.Format(_T("%s_D10042#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq6);					ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ6
	//ErcpDataSet.Format(_T("%s_D10043#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq7);					ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ7
	//ErcpDataSet.Format(_T("%s_D10044#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq8);					ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ8
	//ErcpDataSet.Format(_T("%s_D10045#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq9);					ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ9
	//ErcpDataSet.Format(_T("%s_D10046#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq10);					ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ10
	//ErcpDataSet.Format(_T("%s_D10047#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay1);					ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY1
	//ErcpDataSet.Format(_T("%s_D10048#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay2);					ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY2
	//ErcpDataSet.Format(_T("%s_D10049#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay3);					ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY3
	//ErcpDataSet.Format(_T("%s_D10050#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay4);					ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY4
	//ErcpDataSet.Format(_T("%s_D10051#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay5);					ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY5
	//ErcpDataSet.Format(_T("%s_D10052#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay6);					ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY6
	//ErcpDataSet.Format(_T("%s_D10053#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay7);					ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY7
	//ErcpDataSet.Format(_T("%s_D10054#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay8);					ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY8
	//ErcpDataSet.Format(_T("%s_D10055#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay9);					ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY9
	//ErcpDataSet.Format(_T("%s_D10056#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq1);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ1
	//ErcpDataSet.Format(_T("%s_D10057#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq2);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ2
	//ErcpDataSet.Format(_T("%s_D10058#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq3);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ3
	//ErcpDataSet.Format(_T("%s_D10059#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq4);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ4
	//ErcpDataSet.Format(_T("%s_D10060#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq5);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ5
	//ErcpDataSet.Format(_T("%s_D10061#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq6);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ6
	//ErcpDataSet.Format(_T("%s_D10062#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq7);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ7
	//ErcpDataSet.Format(_T("%s_D10063#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq8);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ8
	//ErcpDataSet.Format(_T("%s_D10064#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq9);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ9
	//ErcpDataSet.Format(_T("%s_D10065#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq10);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ10
	//ErcpDataSet.Format(_T("%s_D10066#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay1);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY1
	//ErcpDataSet.Format(_T("%s_D10067#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay2);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY2
	//ErcpDataSet.Format(_T("%s_D10068#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay3);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY3
	//ErcpDataSet.Format(_T("%s_D10069#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay4);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY4
	//ErcpDataSet.Format(_T("%s_D10070#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay5);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY5
	//ErcpDataSet.Format(_T("%s_D10071#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay6);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY6
	//ErcpDataSet.Format(_T("%s_D10072#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay7);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY7
	//ErcpDataSet.Format(_T("%s_D10073#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay8);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY8
	//ErcpDataSet.Format(_T("%s_D10074#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay9);					ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY9

	//ErcpDataSet.Format(_T("%s_D10075#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccVolt);						ErcpMessageSet += ErcpDataSet; // VCC_VOLT
	//ErcpDataSet.Format(_T("%s_D10076#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccVoltOffset);					ErcpMessageSet += ErcpDataSet; // VCC_VOLT_OFFSET
	//ErcpDataSet.Format(_T("%s_D10077#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccLimitVoltLow);				ErcpMessageSet += ErcpDataSet; // VCC_LIMIT_VOLT_LOW
	//ErcpDataSet.Format(_T("%s_D10078#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccLimitVoltHigh);				ErcpMessageSet += ErcpDataSet; // VCC_LIMIT_VOLT_HIGH
	//ErcpDataSet.Format(_T("%s_D10079#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccLimitCurrLow);				ErcpMessageSet += ErcpDataSet; // VCC_LIMIT_CURR_LOW
	//ErcpDataSet.Format(_T("%s_D10080#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccLimitCurrHigh);				ErcpMessageSet += ErcpDataSet; // VCC_LIMIT_CURR_HIGH
	//ErcpDataSet.Format(_T("%s_D10081#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblVolt);						ErcpMessageSet += ErcpDataSet; // VBL_VOLT
	//ErcpDataSet.Format(_T("%s_D10082#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblVoltOffset); 					ErcpMessageSet += ErcpDataSet;  // VBL_VOLT_OFFSET
	//ErcpDataSet.Format(_T("%s_D10083#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblLimitVoltLow);				ErcpMessageSet += ErcpDataSet; // VBL_LIMIT_VOLT_LOW
	//ErcpDataSet.Format(_T("%s_D10084#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblLimitVoltHigh);				ErcpMessageSet += ErcpDataSet; // VBL_LIMIT_VOLT_HIGH
	//ErcpDataSet.Format(_T("%s_D10085#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblLimitCurrLow);				ErcpMessageSet += ErcpDataSet; // VBL_LIMIT_CURR_LOW
	//ErcpDataSet.Format(_T("%s_D10086#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblLimitCurrHigh);				ErcpMessageSet += ErcpDataSet; // VBL_LIMIT_CURR_HIGH
	//ErcpDataSet.Format(_T("%s_D10087#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nAgingTimeHH);					ErcpMessageSet += ErcpDataSet; // AGING_TIME_HH
	//ErcpDataSet.Format(_T("%s_D10088#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nAgingTimeMM);					ErcpMessageSet += ErcpDataSet; // AGING_TIME_MM
	//ErcpDataSet.Format(_T("%s_D10089#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nAgingTimeMinute);				ErcpMessageSet += ErcpDataSet; // AGING_TIME_MINUTE
	//ErcpDataSet.Format(_T("%s_D10090#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nAgingEndWaitTime);				ErcpMessageSet += ErcpDataSet; // AGING_END_WAIT_TIME
	//ErcpDataSet.Format(_T("%s_D10091#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nOpeTemperatureUse);				ErcpMessageSet += ErcpDataSet; // TEMPERATURE_USE
	//ErcpDataSet.Format(_T("%s_D10092#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nOpeTemperatureMin);				ErcpMessageSet += ErcpDataSet; // TEMPERATURE_MIN
	//ErcpDataSet.Format(_T("%s_D10093#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nOpeTemperatureMax);				ErcpMessageSet += ErcpDataSet; // TEMPERATURE_MAX
	//ErcpDataSet.Format(_T("%s_D10094#%d"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nOpeDoorUse);						ErcpMessageSet += ErcpDataSet; // DOOR_USE

	try {
		ErcpDataSet.Format(_T("MODEL_NB#1^"));																									ErcpMessageSet += ErcpDataSet; // MODEL_NUMBER
		ErcpDataSet.Format(_T("%s_DIMMING_SEL_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nDimmingSel);					ErcpMessageSet += ErcpDataSet; // DIMMING SEL
		ErcpDataSet.Format(_T("%s_PWM_FREQ_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPwmFreq);						ErcpMessageSet += ErcpDataSet; // PWM_FREQ
		ErcpDataSet.Format(_T("%s_PWM_DUTY_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPwmDuty);						ErcpMessageSet += ErcpDataSet; // PWM_DUTY
		ErcpDataSet.Format(_T("%s_VBR_VOLT_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVbrVolt);						ErcpMessageSet += ErcpDataSet; // VBR_VOLT
		ErcpDataSet.Format(_T("%s_CABLE_OPEN_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nFuncCableOpen);				ErcpMessageSet += ErcpDataSet; // CABLE_OPEN
		ErcpDataSet.Format(_T("%s_POWER_ON_SEQ1_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq1);				ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ1
		ErcpDataSet.Format(_T("%s_POWER_ON_SEQ2_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq2);				ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ2
		ErcpDataSet.Format(_T("%s_POWER_ON_SEQ3_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq3);				ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ3
		ErcpDataSet.Format(_T("%s_POWER_ON_SEQ4_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq4);				ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ4
		ErcpDataSet.Format(_T("%s_POWER_ON_SEQ5_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq5);				ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ5
		ErcpDataSet.Format(_T("%s_POWER_ON_SEQ6_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq6);				ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ6
		ErcpDataSet.Format(_T("%s_POWER_ON_SEQ7_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq7);				ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ7
		ErcpDataSet.Format(_T("%s_POWER_ON_SEQ8_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq8);				ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ8
		ErcpDataSet.Format(_T("%s_POWER_ON_SEQ9_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq9);				ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ9
		ErcpDataSet.Format(_T("%s_POWER_ON_SEQ10_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnSeq10);			ErcpMessageSet += ErcpDataSet; // POWER_ON_SEQ10
		ErcpDataSet.Format(_T("%s_POWER_ON_DELAY1_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay1);			ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY1
		ErcpDataSet.Format(_T("%s_POWER_ON_DELAY2_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay2);			ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY2
		ErcpDataSet.Format(_T("%s_POWER_ON_DELAY3_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay3);			ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY3
		ErcpDataSet.Format(_T("%s_POWER_ON_DELAY4_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay4);			ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY4
		ErcpDataSet.Format(_T("%s_POWER_ON_DELAY5_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay5);			ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY5
		ErcpDataSet.Format(_T("%s_POWER_ON_DELAY6_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay6);			ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY6
		ErcpDataSet.Format(_T("%s_POWER_ON_DELAY7_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay7);			ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY7
		ErcpDataSet.Format(_T("%s_POWER_ON_DELAY8_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay8);			ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY8
		ErcpDataSet.Format(_T("%s_POWER_ON_DELAY9_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOnDelay9);			ErcpMessageSet += ErcpDataSet; // POWER_ON_DELAY9


		ErcpDataSet.Format(_T("%s_POWER_OFF_SEQ1_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq1);			ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ1
		ErcpDataSet.Format(_T("%s_POWER_OFF_SEQ2_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq2);			ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ2
		ErcpDataSet.Format(_T("%s_POWER_OFF_SEQ3_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq3);			ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ3
		ErcpDataSet.Format(_T("%s_POWER_OFF_SEQ4_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq4);			ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ4
		ErcpDataSet.Format(_T("%s_POWER_OFF_SEQ5_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq5);			ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ5
		ErcpDataSet.Format(_T("%s_POWER_OFF_SEQ6_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq6);			ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ6
		ErcpDataSet.Format(_T("%s_POWER_OFF_SEQ7_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq7);			ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ7
		ErcpDataSet.Format(_T("%s_POWER_OFF_SEQ8_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq8);			ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ8
		ErcpDataSet.Format(_T("%s_POWER_OFF_SEQ9_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq9);			ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ9
		ErcpDataSet.Format(_T("%s_POWER_OFF_SEQ10_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffSeq10);			ErcpMessageSet += ErcpDataSet; // POWER_OFF_SEQ10
		ErcpDataSet.Format(_T("%s_POWER_OFF_DELAY1_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay1);		ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY1
		ErcpDataSet.Format(_T("%s_POWER_OFF_DELAY2_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay2);		ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY2
		ErcpDataSet.Format(_T("%s_POWER_OFF_DELAY3_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay3);		ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY3
		ErcpDataSet.Format(_T("%s_POWER_OFF_DELAY4_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay4);		ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY4
		ErcpDataSet.Format(_T("%s_POWER_OFF_DELAY5_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay5);		ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY5
		ErcpDataSet.Format(_T("%s_POWER_OFF_DELAY6_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay6);		ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY6
		ErcpDataSet.Format(_T("%s_POWER_OFF_DELAY7_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay7);		ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY7
		ErcpDataSet.Format(_T("%s_POWER_OFF_DELAY8_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay8);		ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY8
		ErcpDataSet.Format(_T("%s_POWER_OFF_DELAY9_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nPowerOffDelay9);		ErcpMessageSet += ErcpDataSet; // POWER_OFF_DELAY9
		ErcpDataSet.Format(_T("%s_VCC_VOLT_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccVolt);						ErcpMessageSet += ErcpDataSet; // VCC_VOLT
		ErcpDataSet.Format(_T("%s_VCC_VOLT_OFFSET_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccVoltOffset);			ErcpMessageSet += ErcpDataSet; // VCC_VOLT_OFFSET
		ErcpDataSet.Format(_T("%s_VCC_LIMIT_VOLT_LOW_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccLimitVoltLow);		ErcpMessageSet += ErcpDataSet; // VCC_LIMIT_VOLT_LOW
		ErcpDataSet.Format(_T("%s_VCC_LIMIT_VOLT_HIGH_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccLimitVoltHigh);	ErcpMessageSet += ErcpDataSet; // VCC_LIMIT_VOLT_HIGH
		ErcpDataSet.Format(_T("%s_VCC_LIMIT_CURR_LOW_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccLimitCurrLow);		ErcpMessageSet += ErcpDataSet; // VCC_LIMIT_CURR_LOW
		ErcpDataSet.Format(_T("%s_VCC_LIMIT_CURR_HIGH_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVccLimitCurrHigh);	ErcpMessageSet += ErcpDataSet; // VCC_LIMIT_CURR_HIGH
		ErcpDataSet.Format(_T("%s_VBL_VOLT_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblVolt);						ErcpMessageSet += ErcpDataSet; // VBL_VOLT
		ErcpDataSet.Format(_T("%s_VBL_OFFSET_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblVoltOffset);				ErcpMessageSet += ErcpDataSet; // VBL_VOLT_OFFSET
		ErcpDataSet.Format(_T("%s_VBL_LIMIT_VOLT_LOW_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblLimitVoltLow);		ErcpMessageSet += ErcpDataSet; // VBL_LIMIT_VOLT_LOW
		ErcpDataSet.Format(_T("%s_VBL_LIMIT_VOLT_HIGH_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblLimitVoltHigh);	ErcpMessageSet += ErcpDataSet; // VBL_LIMIT_VOLT_HIGH
		ErcpDataSet.Format(_T("%s_VBL_LIMIT_CURR_LOW_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblLimitCurrLow);		ErcpMessageSet += ErcpDataSet; // VBL_LIMIT_CURR_LOW
		ErcpDataSet.Format(_T("%s_VBL_LIMIT_CURR_HIGH_MODEL_INFO#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_fVblLimitCurrHigh);	ErcpMessageSet += ErcpDataSet; // VBL_LIMIT_CURR_HIGH
		ErcpDataSet.Format(_T("%s_AGING_TIME_HH_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nAgingTimeHH);				ErcpMessageSet += ErcpDataSet; // AGING_TIME_HH
		ErcpDataSet.Format(_T("%s_AGING_TIME_MM_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nAgingTimeMM);				ErcpMessageSet += ErcpDataSet; // AGING_TIME_MM
		ErcpDataSet.Format(_T("%s_AGING_TIME_MINUTE_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nAgingTimeMinute);		ErcpMessageSet += ErcpDataSet; // AGING_TIME_MINUTE
		ErcpDataSet.Format(_T("%s_AGING_END_WAIT_TIME_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nAgingEndWaitTime);	ErcpMessageSet += ErcpDataSet; // AGING_END_WAIT_TIM
		ErcpDataSet.Format(_T("%s_TEMPERATURE_USE_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nOpeTemperatureUse);		ErcpMessageSet += ErcpDataSet; // TEMPERATURE_USE
		ErcpDataSet.Format(_T("%s_TEMPERATURE_MIN_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nOpeTemperatureMin);		ErcpMessageSet += ErcpDataSet; // TEMPERATURE_MIN
		ErcpDataSet.Format(_T("%s_TEMPERATURE_MAX_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nOpeTemperatureMax);		ErcpMessageSet += ErcpDataSet; // TEMPERATURE_MAX
		ErcpDataSet.Format(_T("%s_DOOR_USE_MODEL_INFO#%d^"), lpSystemInfo->m_sEqpName.Right(6), lpModelInfo->m_nOpeDoorUse);					ErcpMessageSet += ErcpDataSet; // DOOR_USE
		ErcpDataSet.Format(_T("%s_TEMP_ZONE_S1#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpInspWorkInfo->m_fTempReadVal[0]);					ErcpMessageSet += ErcpDataSet; // 1ZONE 온도 (S1)
		ErcpDataSet.Format(_T("%s_TEMP_ZONE_S2#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpInspWorkInfo->m_fTempReadVal[1]);					ErcpMessageSet += ErcpDataSet; // 1ZONE 온도 (S2)
		ErcpDataSet.Format(_T("%s_TEMP_ZONE_S3#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpInspWorkInfo->m_fTempReadVal[2]);					ErcpMessageSet += ErcpDataSet; // 2ZONE 온도 (S3)
		ErcpDataSet.Format(_T("%s_TEMP_ZONE_S4#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpInspWorkInfo->m_fTempReadVal[3]);					ErcpMessageSet += ErcpDataSet; // 2ZONE 온도 (S4)
		ErcpDataSet.Format(_T("%s_TEMP_ZONE_S5#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpInspWorkInfo->m_fTempReadVal[4]);					ErcpMessageSet += ErcpDataSet; // 3ZONE 온도 (S5)
		ErcpDataSet.Format(_T("%s_TEMP_ZONE_S6#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpInspWorkInfo->m_fTempReadVal[5]);					ErcpMessageSet += ErcpDataSet; // 3ZONE 온도 (S6)
		ErcpDataSet.Format(_T("%s_TEMP_ZONE_SET1#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpInspWorkInfo->m_fTempReadValST590_2[0]);			ErcpMessageSet += ErcpDataSet; // 1ZONE 메인 컨트롤러 세팅값
		ErcpDataSet.Format(_T("%s_TEMP_ZONE_SET2#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpInspWorkInfo->m_fTempReadValST590_2[1]);			ErcpMessageSet += ErcpDataSet; // 2ZONE 메인 컨트롤러 세팅값
		ErcpDataSet.Format(_T("%s_TEMP_ZONE_SET3#%f^"), lpSystemInfo->m_sEqpName.Right(6), lpInspWorkInfo->m_fTempReadValST590_2[2]);			ErcpMessageSet += ErcpDataSet; // 3ZONE 메인 컨트롤러 세팅값

		m_pApp->pCimNet->SetERCPInfo(ErcpMessageSet);

		m_pApp->Gf_gmesSendHost(HOST_ERCP, NULL, NULL, NULL);
	}
	catch (const std::exception& ex)
	{
		CString errmsg;
		errmsg.Format(_T("ERCP MESSAGE SEND ERROR"), ex.what());
		AfxMessageBox(errmsg, MB_ICONERROR);
	}
}

void CHseAgingDlg::OnBnClickedPause1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
