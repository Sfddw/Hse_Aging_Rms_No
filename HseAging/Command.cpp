// CCommand.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "HseAging.h"
#include "Command.h"


// CCommand
IMPLEMENT_DYNAMIC(CCommand, CWnd)

CCommand::CCommand()
{
	lpSystemInfo	= m_pApp->GetSystemInfo();
	lpModelInfo		= m_pApp->GetModelInfo();
	lpInspWorkInfo	= m_pApp->GetInspWorkInfo();
}

CCommand::~CCommand()
{
}


BEGIN_MESSAGE_MAP(CCommand, CWnd)
END_MESSAGE_MAP()

int CCommand::getPowerOffAckWaitTime()
{
	int waitTime=0;

	return waitTime=1000;
}

int CCommand::getPwrSeqIndex(int cmbIndex)
{
	return cmbIndex;
}

CString CCommand::makeSystemFusingData()
{
	CString makePacket;
	CString stBuff = _T("");
	int nInterface=0, nBitSwap=0, nPolarity=0;
	float mClk=0.0,m_fvol=0.0;

#if 0
	if(lpModelInfo->m_nLcmSignalType == PIXEL_SINGLE)		nInterface=0;
	else if(lpModelInfo->m_nLcmSignalType == PIXEL_DUAL)	nInterface=1;
	else if(lpModelInfo->m_nLcmSignalType == PIXEL_QUAD)	nInterface=2;

	nBitSwap	= (lpModelInfo->m_nLcmBitSwap << 5);
	nPolarity = 0;
	if(lpModelInfo->m_nLcmSignalType == PIXEL_SINGLE)		mClk = lpModelInfo->m_fTimingMainClock;
	if(lpModelInfo->m_nLcmSignalType == PIXEL_DUAL)			mClk = lpModelInfo->m_fTimingMainClock / 2.0f;
	if(lpModelInfo->m_nLcmSignalType == PIXEL_QUAD)			mClk = lpModelInfo->m_fTimingMainClock / 4.0f;

	stBuff.Format(_T("INFO"));															makePacket.Append(stBuff);	// LCD Info				: INFO
	stBuff.Format(_T("%02X"), (nInterface | nBitSwap));									makePacket.Append(stBuff);	// Info MODE
	stBuff.Format(_T("%01X"), nPolarity);												makePacket.Append(stBuff);	// LCD Info Polarity	: 0
	stBuff.Format(_T("%04d"), lpModelInfo->m_nTimingHorActive);							makePacket.Append(stBuff);	// LCD Info H-Active	: 3088
	stBuff.Format(_T("%04d"), lpModelInfo->m_nTimingVerActive);							makePacket.Append(stBuff);	// LCD Info V-Active	: 1728
	stBuff.Format(_T("%06.2f"), mClk);													makePacket.Append(stBuff);	// LCD Info M-CLK		: 86.72
	stBuff.Format(_T("%04d"), lpModelInfo->m_nTimingHorWidth);							makePacket.Append(stBuff);	// LCD Info H-Width		: 32
	stBuff.Format(_T("%04d"), lpModelInfo->m_nTimingHorFP);								makePacket.Append(stBuff);	// LCD Info H-FrontPorch: 48
	stBuff.Format(_T("%04d"), lpModelInfo->m_nTimingHorBP);								makePacket.Append(stBuff);	// LCD Info H-BackPorch	: 80
	stBuff.Format(_T("%04d"), lpModelInfo->m_nTimingVerWidth);							makePacket.Append(stBuff);	// LCD Info V-Width		: 10
	stBuff.Format(_T("%04d"), lpModelInfo->m_nTimingVerFP);								makePacket.Append(stBuff);	// LCD Info V-FrontPorch: 4
	stBuff.Format(_T("%04d"), lpModelInfo->m_nTimingVerBP);								makePacket.Append(stBuff);	// LCD Info V-BackPorch	: 38

	stBuff.Format(_T("%01d"), lpModelInfo->m_nLcmSignalType);							makePacket.Append(stBuff);	// Signal Type			: LVDS
	stBuff.Format(_T("%01d"), lpModelInfo->m_nLcmSignalBit);							makePacket.Append(stBuff);	// Signal Bit			: 8Bit
	stBuff.Format(_T("%01d"), lpModelInfo->m_nLcmPixelType);							makePacket.Append(stBuff);	// Pixel Type			: QUAD	(0:Single, 2:Dual, 3:Quad)
#endif
	// Timing 사용하진 않지만 Timing 정보 채우도록 한다. Parsing 정보 확인되면 모델 세팅된 정보를 전송할 것.
	stBuff.Format(_T("50HC500DQN-VKXL1-914X-D*******99INFO03038402160074.2506002500250010004000400"));	makePacket.Append(stBuff);

	stBuff.Format(_T("%04d"), lpModelInfo->m_nAgingTimeMinute);							makePacket.Append(stBuff);	// Total Time
	stBuff.Format(_T("%03d"), 0)							;							makePacket.Append(stBuff);	// Next Pattern Time
	stBuff.Format(_T("%03d"), 0);														makePacket.Append(stBuff);	// Start Time
	stBuff.Format(_T("%03d"), 0);														makePacket.Append(stBuff);	// Auto On Time
	stBuff.Format(_T("%03d"), 0);														makePacket.Append(stBuff);	// Auto Off Time
	stBuff.Format(_T("%02d"), 0);														makePacket.Append(stBuff);	// NG Count
	stBuff.Format(_T("%01d"), 0);														makePacket.Append(stBuff);	// Current Check On/Off
	stBuff.Format(_T("%02d"), 0);														makePacket.Append(stBuff);	// Current Low Limit Count
	stBuff.Format(_T("%03d"), 0);														makePacket.Append(stBuff);	// OffOn Time When NG
	stBuff.Format(_T("%02d"), 0);														makePacket.Append(stBuff);	// Unchanged Current Turn Count
	stBuff.Format(_T("%04d"), 0);														makePacket.Append(stBuff);	// Unchanged Current Value
	stBuff.Format(_T("%01d"), 0);														makePacket.Append(stBuff);	// Monitoring Pattern OnOff
	stBuff.Format(_T("%03d"), 0);														makePacket.Append(stBuff);	// Monitoring Pattern Time
	stBuff.Format(_T("%01d"), lpModelInfo->m_nLcmSignalType);							makePacket.Append(stBuff);	// Signal Type
	stBuff.Format(_T("%01d"), lpModelInfo->m_nLcmSignalBit);							makePacket.Append(stBuff);	// Signal Bit
	stBuff.Format(_T("%01d"), lpModelInfo->m_nLcmPixelType);							makePacket.Append(stBuff);	// Pixel Type
	stBuff.Format(_T("%01d"), lpModelInfo->m_nLcmLvdsRsSel);							makePacket.Append(stBuff);	// LVDS Select
	stBuff.Format(_T("%01d"), 0);														makePacket.Append(stBuff);	// Bit Select
	stBuff.Format(_T("%01d"), 0);														makePacket.Append(stBuff);	// Vby1 Data Format
	stBuff.Format(_T("%01d"), 0);														makePacket.Append(stBuff);	// Vby1 Data PreEmphasis
	stBuff.Format(_T("%01d"), 0);														makePacket.Append(stBuff);	// Vby1 AGP Mode
	stBuff.Format(_T("%02d"), 0);														makePacket.Append(stBuff);	// User Gender Check
	stBuff.Format(_T("%01d"), lpModelInfo->m_nFuncCableOpen);							makePacket.Append(stBuff);	// User Cable Check
	stBuff.Format(_T("%01d"), 0);														makePacket.Append(stBuff);	// AGP OnOff
	stBuff.Format(_T("%01d"), 0);														makePacket.Append(stBuff);	// HDCP OnOff
	stBuff.Format(_T("%01d"), 0);														makePacket.Append(stBuff);	// Interface Mode
	stBuff.Format(_T("%03d"), (int)(lpModelInfo->m_fVccVolt * 10 + 0.5));				makePacket.Append(stBuff);	// Power VCC
	stBuff.Format(_T("%03d"), 0);														makePacket.Append(stBuff);	// Power VDD
	stBuff.Format(_T("%04d"), (int)(lpModelInfo->m_fVblVolt * 10 + 0.5));				makePacket.Append(stBuff);	// Power VBL
	stBuff.Format(_T("%03d"), (int)(lpModelInfo->m_fVbrVolt * 10 + 0.5));				makePacket.Append(stBuff);	// Power VBR
	stBuff.Format(_T("%03d"), (int)(lpModelInfo->m_fVccVoltOffset * 10 + 0.5));			makePacket.Append(stBuff);	// Power VCC Offset
	stBuff.Format(_T("%03d"), 0);														makePacket.Append(stBuff);	// Power VDD Offset
	stBuff.Format(_T("%03d"), (int)(lpModelInfo->m_fVblVoltOffset * 10 + 0.5));			makePacket.Append(stBuff);	// Power VBL Offset
	stBuff.Format(_T("%03d"), 0);														makePacket.Append(stBuff);	// Power VBR Offset

	stBuff.Format(_T("%01d"), lpModelInfo->m_nDimmingSel);								makePacket.Append(stBuff);	// Dimming Select
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPwmFreq);									makePacket.Append(stBuff);	// PWM Freq
	stBuff.Format(_T("%03d"), lpModelInfo->m_nPwmDuty);									makePacket.Append(stBuff);	// PWM Duty

	stBuff.Format(_T("%04d"), (int)(lpModelInfo->m_fVccLimitVoltLow * 100 + 0.5));		makePacket.Append(stBuff);	// Limit VCC Low
	stBuff.Format(_T("%04d"), (int)(lpModelInfo->m_fVccLimitVoltHigh * 100 + 0.5));		makePacket.Append(stBuff);	// Limit VCC High
	stBuff.Format(_T("%04d"), 0);														makePacket.Append(stBuff);	// Limit VDD Low
	stBuff.Format(_T("%04d"), 0);														makePacket.Append(stBuff);	// Limit VDD High
	stBuff.Format(_T("%04d"), (int)(lpModelInfo->m_fVblLimitVoltLow * 100 + 0.5));		makePacket.Append(stBuff);	// Limit VBL Low
	stBuff.Format(_T("%04d"), (int)(lpModelInfo->m_fVblLimitVoltHigh * 100 + 0.5));		makePacket.Append(stBuff);	// Limit VBL High
	stBuff.Format(_T("%04d"), (int)(lpModelInfo->m_fVccLimitCurrLow * 100 + 0.5));		makePacket.Append(stBuff);	// Limit ICC Low
	stBuff.Format(_T("%04d"), (int)(lpModelInfo->m_fVccLimitCurrHigh * 100 + 0.5));		makePacket.Append(stBuff);	// Limit ICC High
	stBuff.Format(_T("%04d"), 0);														makePacket.Append(stBuff);	// Limit IDD Low
	stBuff.Format(_T("%04d"), 0);														makePacket.Append(stBuff);	// Limit IDD High
	stBuff.Format(_T("%04d"), (int)(lpModelInfo->m_fVblLimitCurrLow * 100 + 0.5));		makePacket.Append(stBuff);	// Limit IBL Low
	stBuff.Format(_T("%04d"), (int)(lpModelInfo->m_fVblLimitCurrHigh * 100 + 0.5));		makePacket.Append(stBuff);	// Limit IBL High

	stBuff.Format(_T("%02d"), 0);														makePacket.Append(stBuff);	// PWM Min
	stBuff.Format(_T("%02d"), 0);														makePacket.Append(stBuff);	// String Of Edge CH1
	stBuff.Format(_T("%02d"), 0);														makePacket.Append(stBuff);	// String Of Edge CH2
	stBuff.Format(_T("%02X"), 0);														makePacket.Append(stBuff);	// Model No BLU
	
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq1));				makePacket.Append(stBuff);	// Power ON SEQ 1
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq2));				makePacket.Append(stBuff);	// Power ON SEQ 2
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq3));				makePacket.Append(stBuff);	// Power ON SEQ 3
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq4));				makePacket.Append(stBuff);	// Power ON SEQ 4
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq5));				makePacket.Append(stBuff);	// Power ON SEQ 5
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq6));				makePacket.Append(stBuff);;	// Power ON SEQ 6
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq7));				makePacket.Append(stBuff);	// Power ON SEQ 7
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq8));				makePacket.Append(stBuff);	// Power ON SEQ 8
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq9));				makePacket.Append(stBuff);	// Power ON SEQ 9
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq10));			makePacket.Append(stBuff);;	// Power ON SEQ 10
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOnSeq11));			makePacket.Append(stBuff);;	// Power ON SEQ 11

	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOnDelay1);							makePacket.Append(stBuff);	// Power ON Delay 1
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOnDelay2);							makePacket.Append(stBuff);	// Power ON Delay 2
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOnDelay3);							makePacket.Append(stBuff);	// Power ON Delay 3
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOnDelay4);							makePacket.Append(stBuff);	// Power ON Delay 4
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOnDelay5);							makePacket.Append(stBuff);	// Power ON Delay 5
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOnDelay6);							makePacket.Append(stBuff);	// Power ON Delay 6
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOnDelay7);							makePacket.Append(stBuff);	// Power ON Delay 7
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOnDelay8);							makePacket.Append(stBuff);	// Power ON Delay 8
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOnDelay9);							makePacket.Append(stBuff);	// Power ON Delay 9
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOnDelay10);							makePacket.Append(stBuff);	// Power ON Delay 10

	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq1));			makePacket.Append(stBuff);	// Power OFF SEQ 1
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq2));			makePacket.Append(stBuff);	// Power OFF SEQ 2
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq3));			makePacket.Append(stBuff);	// Power OFF SEQ 3
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq4));			makePacket.Append(stBuff);	// Power OFF SEQ 4
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq5));			makePacket.Append(stBuff);	// Power OFF SEQ 5
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq6));			makePacket.Append(stBuff);;	// Power OFF SEQ 6
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq7));			makePacket.Append(stBuff);	// Power OFF SEQ 7
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq8));			makePacket.Append(stBuff);	// Power OFF SEQ 8
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq9));			makePacket.Append(stBuff);	// Power OFF SEQ 9
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq10));			makePacket.Append(stBuff);;	// Power OFF SEQ 10
	stBuff.Format(_T("%02d"), getPwrSeqIndex(lpModelInfo->m_nPowerOffSeq11));			makePacket.Append(stBuff);;	// Power OFF SEQ 11

	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOffDelay1);							makePacket.Append(stBuff);	// Power OFF Delay 1
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOffDelay2);							makePacket.Append(stBuff);	// Power OFF Delay 2
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOffDelay3);							makePacket.Append(stBuff);	// Power OFF Delay 3
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOffDelay4);							makePacket.Append(stBuff);	// Power OFF Delay 4
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOffDelay5);							makePacket.Append(stBuff);	// Power OFF Delay 5
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOffDelay6);							makePacket.Append(stBuff);	// Power OFF Delay 6
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOffDelay7);							makePacket.Append(stBuff);	// Power OFF Delay 7
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOffDelay8);							makePacket.Append(stBuff);	// Power OFF Delay 8
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOffDelay9);							makePacket.Append(stBuff);	// Power OFF Delay 9
	stBuff.Format(_T("%04d"), lpModelInfo->m_nPowerOffDelay10);							makePacket.Append(stBuff);	// Power OFF Delay 10

	return makePacket;
}


BOOL CCommand::Gf_setFusingSystemInfo(int rack)
{
	BOOL bRet=FALSE;
	int nSize = 0;
	char szPacket[2048]={0,};
	CString fusingData;
	fusingData = makeSystemFusingData();
	sprintf_s(szPacket, "%S", fusingData.GetBuffer(0));

	nSize = (int)strlen(szPacket);

	bRet = m_pApp->udp_sendPacketUDPRack(rack, CMD_SET_FUSING, nSize, szPacket, ACK, 2000);

	return bRet;
}

BOOL CCommand::Gf_getCableOpenCheck(int rack)
{
	BOOL bRet = FALSE;

	bRet = m_pApp->udp_sendPacketUDPRack(rack, CMD_SET_CABLE_OPEN_CHECK, NULL, NULL, ACK, 2000);

	return bRet;
}


BOOL CCommand::Gf_setChannelUseUnuse(CString ip, int* ch_use)
{
	BOOL bRet = FALSE;
	int nSize = 0;
	char szPacket[2048] = { 0, };

	sprintf_s(szPacket, "%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d"
		, ch_use[0]
		, ch_use[1]
		, ch_use[2]
		, ch_use[3]
		, ch_use[4]
		, ch_use[5]
		, ch_use[6]
		, ch_use[7]
		, ch_use[8]
		, ch_use[9]
		, ch_use[10]
		, ch_use[11]
		, ch_use[12]
		, ch_use[13]
		, ch_use[14]
		, ch_use[15]
	);

	nSize = (int)strlen(szPacket);
	bRet = m_pApp->udp_sendPacketUDP(ip, CMD_SET_CHANNEL_USE_UNUSE, nSize, szPacket, ACK);

	return bRet;
}

BOOL CCommand::Gf_setPowerSequenceOnOff(int rack, BOOL onoff, int bAck)
{
	BOOL ret=FALSE;
	int length=0;
	char szPacket[128]={0,};

	CString sLog;
	if(onoff==ON)				sLog.Format(_T("<TEST> 'RACK-%c' POWER ON"), rack + 'A');
	else if (onoff == OFF)		sLog.Format(_T("<TEST> 'RACK-%c' POWER OFF"), rack + 'A');
	
	sprintf_s(szPacket, "%01d", onoff);
	length = (int)strlen(szPacket);

	int waitTime;
	waitTime = getPowerOffAckWaitTime();
	ret = m_pApp->udp_sendPacketUDPRack(rack, CMD_SET_POWER_SEQUENCE_ONOFF, length, szPacket, bAck, waitTime);

	// Power Off 동작 시 Error 정보는 Clear 시킨다.
	m_pApp->Gf_clearAgingStatusError();

	return ret;
}

BOOL CCommand::Gf_getAllPowerMeasure(int rack)
{
	BOOL ret=FALSE;

	ret = m_pApp->udp_sendPacketUDPRack(rack, CMD_GET_POWER_MEASURE, 0, NULL, NACK);
	return ret;
}

BOOL CCommand::Gf_getPowerMeasureAllGroup()
{
	BOOL ret=FALSE;

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		ret = m_pApp->udp_sendPacketUDPRack(rack, CMD_GET_POWER_MEASURE, 0, NULL, NACK);
	}
	return ret;
}

BOOL CCommand::Gf_getAgingStatusAllGroup(int* skipGroup)
{
	BOOL ret=FALSE;

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		if (skipGroup[rack] == TRUE)
			continue;

		ret = m_pApp->udp_sendPacketUDPRack(rack, CMD_GET_AGING_STATUS, 0, NULL, NACK);
	}
	return ret;
}

BOOL CCommand::Gf_setAgingSTART(int rack)
{
	BOOL ret = FALSE;
	int nSize = 0;
	char szPacket[16]={0,};

	sprintf_s(szPacket, "%01d", AGING_START);
	nSize = (int)strlen(szPacket);
	
	return m_pApp->udp_sendPacketUDPRack(rack, CMD_SET_AGING_START_STOP, nSize, szPacket, ACK);
}

BOOL CCommand::Gf_setAgingSTOP(int rack)
{
	BOOL ret=FALSE;
	int nSize = 0;
	char szPacket[16]={0,};

	sprintf_s(szPacket, "%01d", AGING_STOP);
	nSize = (int)strlen(szPacket);

	ret = m_pApp->udp_sendPacketUDPRack(rack, CMD_SET_AGING_START_STOP, nSize, szPacket, ACK);

	return ret;
}

BOOL CCommand::Gf_getMainBoardFwVersion(int rack)
{
	BOOL ret = FALSE;

	ret = m_pApp->udp_sendPacketUDPRack(rack, CMD_GET_FW_VERSION, 0, NULL, NACK);

	return ret;
}

BOOL CCommand::Gf_getMainBoardFwVersionAll()
{
	BOOL ret=FALSE;

	for (int rack = 0; rack < MAX_RACK; rack++)
	{
		ret = m_pApp->udp_sendPacketUDPRack(rack, CMD_GET_FW_VERSION, 0, NULL, NACK);
	}
	return ret;
}

BOOL CCommand::Gf_setGoToBootSection(int rack)
{
	BOOL ret=FALSE;

	ret = m_pApp->udp_sendPacketUDPRack(rack, CMD_GOTOBOOT_DSECTION, 0, NULL, ACK);

	return ret;
}

BOOL CCommand::Gf_setDownloadData(int rack, int nSize, char* packet)
{
	BOOL ret=FALSE;

	ret = m_pApp->udp_sendPacketUDPRack(rack, CMD_BOOTCODE_DOWNLOAD, nSize, packet, ACK, 5000);

	return ret;
}

BOOL CCommand::Gf_setDownloadComplete(int rack)
{
	BOOL ret=FALSE;

	ret = m_pApp->udp_sendPacketUDPRack(rack, CMD_BOOTCODE_COMPLETE, 0, NULL, ACK);

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCommand::Gf_dio_setDIOBoardInitial()
{
	BOOL ret = FALSE;

	ret = m_pApp->udp_sendPacketUDP(IP_ADDRESS_DIO, CMD_DIO_BOARD_INITIAL, NULL, NULL, NACK);

	return ret;
}

BOOL CCommand::Gf_dio_setDIOWriteOutput(int color, int mode)
{
	BOOL ret = FALSE;
	int nSize = 0;
	char szPacket[2048] = { 0, };
	sprintf_s(szPacket, "%02X%01d", color, mode);

	nSize = (int)strlen(szPacket);
	ret = m_pApp->udp_sendPacketUDP(IP_ADDRESS_DIO, CMD_DIO_OUTPUT, nSize, szPacket, ACK);

	return ret;
}

BOOL CCommand::Gf_dio_getDIOReadStatus()
{
	BOOL ret = FALSE;

	ret = m_pApp->udp_sendPacketUDP(IP_ADDRESS_DIO, CMD_DIO_INPUT, NULL, NULL, NACK);

	return ret;

}

BOOL CCommand::Gf_dio_getDIOBoardFwVersion()
{
	BOOL ret = FALSE;

	ret = m_pApp->udp_sendPacketUDP(IP_ADDRESS_DIO, CMD_GET_FW_VERSION, NULL, NULL, ACK);

	return ret;
}

BOOL CCommand::Gf_dio_setGoToBootSection()
{
	BOOL ret = FALSE;

	ret = m_pApp->udp_sendPacketUDP(IP_ADDRESS_DIO, CMD_GOTOBOOT_DSECTION, 0, NULL, ACK);

	return ret;
}

BOOL CCommand::Gf_dio_setDownloadData(int nSize, char* packet)
{
	BOOL ret = FALSE;

	ret = m_pApp->udp_sendPacketUDP(IP_ADDRESS_DIO, CMD_BOOTCODE_DOWNLOAD, nSize, packet, ACK);

	return ret;
}

BOOL CCommand::Gf_dio_setDownloadComplete()
{
	BOOL ret = FALSE;

	ret = m_pApp->udp_sendPacketUDP(IP_ADDRESS_DIO, CMD_BOOTCODE_COMPLETE, 0, NULL, ACK);

	return ret;
}
