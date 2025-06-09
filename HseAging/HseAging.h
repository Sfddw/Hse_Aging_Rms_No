
// HseAging.h: PROJECT_NAME 애플리케이션에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.
#include "Command.h"
#include "CIMNetCommApp.h"
#include "Temp2xxx.h"

// CHseAgingApp:
// 이 클래스의 구현에 대해서는 HseAging.cpp을(를) 참조하세요.
//

class CHseAgingApp : public CWinApp
{
public:
	CHseAgingApp();

// 재정의입니다.
public:
	virtual BOOL InitInstance();

// 구현입니다.

	DECLARE_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
public:
	LPMODELINFO				GetModelInfo();
	LPSYSTEMINFO			GetSystemInfo();
	LPINSPWORKINFO			GetInspWorkInfo();

	CUDPSocket*			m_pUDPSocket;
	CCommand*			pCommand;
	SUMMARYINFO			m_summaryInfo[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	CCimNetCommApi*		pCimNet;
	CTemp2xxx*			m_pTemp2xxx;

	void Gf_SoftwareStartLog();
	void Gf_writeMLog(CString sLogData);
	void Gf_writeAlarmLog(int rack, int layer, int ch, CString strError);
	BOOL Gf_ShowMessageBox(CString errMessage);

	void Gf_setGradientStatic01(CGradientStatic* pGStt, CFont* pFont, BOOL bSplit = FALSE);
	void Gf_setGradientStatic02(CGradientStatic* pGStt, CFont* pFont, BOOL bSplit = FALSE, int txt_align = TEXT_ALIGN_CENTER);
	void Gf_setGradientStatic03(CGradientStatic* pGStt, CFont* pFont, BOOL bSplit = FALSE);
	void Gf_setGradientStatic04(CGradientStatic* pGStt, CFont* pFont, BOOL bSplit = FALSE);
	void Gf_setGradientStatic05(CGradientStatic* pGStt, CFont* pFont, BOOL bSplit = FALSE);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void Gf_InitialSystemInfo();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void Lf_IPStringToRackInfo(CString ip, int* rack, int* layer);
	void Lf_initCreateUdpSocket();
	BOOL procWaitRecvACK(int rack, int layer, int cmd, DWORD waitTime);
	BOOL procWaitRecvACK_DIO(int cmd, DWORD waitTime);
	BOOL procParseMeasurePower(int rack, int layer, CString packet);
	BOOL procParseCableOpenCheck(int rack, int layer, CString packet);
	BOOL procParseAgingStatus(int rack, int layer, CString packet);
	BOOL procParseFWVersion(int rack, int layer, CString packet);
	BOOL procParseGotoBootSection(int rack, int layer, CString packet);
	BOOL udp_sendPacketUDP(CString ip, int nCommand, int nSize, char* pdata, int recvACK, int waitTime = 1000);
	BOOL udp_sendPacketUDPRack(int rack, int nCommand, int nSize, char* pdata, int recvACK, int waitTime = 1000);
	void udp_processPacket(CString strPacket);

	void procDioParseBoardInitial(CString packet);
	void procDioParseInputRead(CString packet);
	void procDioParseFWVersion(CString packet);
	void procDioParseGotoBootSection(CString packet);
	void udp_processDioPacket(CString strPacket);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void Gf_LoadSystemData();
	void Gf_loadModelData(CString modelName);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void Gf_sumWriteSummaryLog(int rack, int layer, int channel);
	void Gf_sumInitSummaryInfo(int rack);
	void Gf_sumSetSummaryInfo(int rack, int layer, int ch, int sumIndex, CString sdata);
	void Gf_sumSetStartTime(int rack);
	void Gf_sumSetEndTime(int rack);
	void Gf_sumSetFailedInfo(int rack, int layer, int ch, CString failMessage);
	void Gf_sumSetPowerMeasureInfo(int group);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	BOOL	Gf_gmesInitServer(BOOL nServerType);
	BOOL	Gf_gmesConnect(int nServerType);
	void	Gf_gmesSetValueAgcm(int rack, int layer, int ch);
	void	Gf_gmesShowLocalErrorMsg();
	CString Gf_gmesGetUserID();
	CString Gf_gmesGetUserName();
	CString Gf_gmesGetRTNCD();
	CString Gf_gmesGetPlanInfo();
	BOOL	Gf_gmesSendHost(int hostCMD, int rack, int layer, int ch);
	void	Lf_gmesSetValueAPDR(int rack, int layer, int ch);
	void	Lf_setGmesValueAGN_INSP(int rack, int layer, int ch);
	void	Lf_setGmesValueDSPM(int rack, int layer, int ch);
	void	Lf_setGmesValueDMIN(int rack);
	void	Lf_setGmesValueDMOU(int rack);

	void	Lf_setGmesValueUNDO(int rack, int layer, int ch);

	BOOL	Gf_gmesSendHost_PCHK(int hostCMD, CString PID);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	void	Gf_clearAgingStatusError();
	BOOL	Gf_initTempRecorder();

	CString m_sSoftwareVersion;
	int m_nActiveGroup;
	BOOL m_bUserIdAdmin;
	CString m_sUserID;
	CString m_sUserName;
	CString m_strLocalSubjectIP;
	BOOL m_bIsGmesConnect;
	BOOL m_bIsEasConnect;
	int m_nAckCmdPG[MAX_RACK][MAX_LAYER];
	int m_nAckCmdDIO;

	BOOL m_bIsSendEAYT;

	int m_nDownloadCountUp;
	int m_nDownloadReadyAckCount;


protected:
	LPMODELINFO				lpModelInfo;
	LPSYSTEMINFO			lpSystemInfo;
	LPINSPWORKINFO			lpInspWorkInfo;

	void Lf_InitGlobalVariable();
	void Lf_initCreateFolder();


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

};

extern CHseAgingApp theApp;
extern CHseAgingApp* m_pApp;