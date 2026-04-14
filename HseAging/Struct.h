#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct _ST_MODELINFO_
{
	int		m_nModelNumber;
	int		r_nModelNumber[6];

	int		m_nTimingHorTotal;
	int		r_nTimingHorTotal[6];
	int		m_nTimingHorActive;
	int		r_nTimingHorActive[6];
	int		m_nTimingHorWidth;
	int		r_nTimingHorWidth[6];
	int		m_nTimingHorBP;
	int		r_nTimingHorBP[6];
	int		m_nTimingHorFP;
	int		r_nTimingHorFP[6];
	int		m_nTimingVerTotal;
	int		r_nTimingVerTotal[6];
	int		m_nTimingVerActive;
	int		r_nTimingVerActive[6];
	int		m_nTimingVerWidth;
	int		r_nTimingVerWidth[6];
	int		m_nTimingVerBP;
	int		r_nTimingVerBP[6];
	int		m_nTimingVerFP;
	int		r_nTimingVerFP[6];
	float	m_fTimingMainClock;
	float	r_fTimingMainClock[6];

	int		m_nLcmSignalType;
	int		r_nLcmSignalType[6];
	int		m_nLcmPixelType;
	int		r_nLcmPixelType[6];
	int		m_nLcmOddEven;
	int		r_nLcmOddEven[6];
	int		m_nLcmSignalBit;
	int		r_nLcmSignalBit[6];
	int		m_nLcmBitSwap;
	int		r_nLcmBitSwap[6];
	int		m_nLcmLvdsRsSel;
	int		r_nLcmLvdsRsSel[6];

	int		m_nDimmingSel;
	int		r_nDimmingSel[6];
	int		m_nPwmFreq;
	int		r_nPwmFreq[6];
	int		m_nPwmDuty;
	int		r_nPwmDuty[6];
	float	m_fVbrVolt;
	float	r_fVbrVolt[6];

	int		m_nFuncCableOpen;
	int		r_nFuncCableOpen[6];

	int		m_nPowerOnSeq1;
	int		r_nPowerOnSeq1[6];
	int		m_nPowerOnSeq2;
	int		r_nPowerOnSeq2[6];
	int		m_nPowerOnSeq3;
	int		r_nPowerOnSeq3[6];
	int		m_nPowerOnSeq4;
	int		r_nPowerOnSeq4[6];
	int		m_nPowerOnSeq5;
	int		r_nPowerOnSeq5[6];
	int		m_nPowerOnSeq6;
	int		r_nPowerOnSeq6[6];
	int		m_nPowerOnSeq7;
	int		r_nPowerOnSeq7[6];
	int		m_nPowerOnSeq8;
	int		r_nPowerOnSeq8[6];
	int		m_nPowerOnSeq9;
	int		r_nPowerOnSeq9[6];
	int		m_nPowerOnSeq10;
	int		r_nPowerOnSeq10[6];
	int		m_nPowerOnSeq11;
	int		r_nPowerOnSeq11[6];
	int		m_nPowerOnDelay1;
	int		r_nPowerOnDelay1[6];
	int		m_nPowerOnDelay2;
	int		r_nPowerOnDelay2[6];
	int		m_nPowerOnDelay3;
	int		r_nPowerOnDelay3[6];
	int		m_nPowerOnDelay4;
	int		r_nPowerOnDelay4[6];
	int		m_nPowerOnDelay5;
	int		r_nPowerOnDelay5[6];
	int		m_nPowerOnDelay6;
	int		r_nPowerOnDelay6[6];
	int		m_nPowerOnDelay7;
	int		r_nPowerOnDelay7[6];
	int		m_nPowerOnDelay8;
	int		r_nPowerOnDelay8[6];
	int		m_nPowerOnDelay9;
	int		r_nPowerOnDelay9[6];
	int		m_nPowerOnDelay10;
	int		r_nPowerOnDelay10[6];
	int		m_nPowerOffSeq1;
	int		r_nPowerOffSeq1[6];
	int		m_nPowerOffSeq2;
	int		r_nPowerOffSeq2[6];
	int		m_nPowerOffSeq3;
	int		r_nPowerOffSeq3[6];
	int		m_nPowerOffSeq4;
	int		r_nPowerOffSeq4[6];
	int		m_nPowerOffSeq5;
	int		r_nPowerOffSeq5[6];
	int		m_nPowerOffSeq6;
	int		r_nPowerOffSeq6[6];
	int		m_nPowerOffSeq7;
	int		r_nPowerOffSeq7[6];
	int		m_nPowerOffSeq8;
	int		r_nPowerOffSeq8[6];
	int		m_nPowerOffSeq9;
	int		r_nPowerOffSeq9[6];
	int		m_nPowerOffSeq10;
	int		r_nPowerOffSeq10[6];
	int		m_nPowerOffSeq11;
	int		r_nPowerOffSeq11[6];
	int		m_nPowerOffDelay1;
	int		r_nPowerOffDelay1[6];
	int		m_nPowerOffDelay2;
	int		r_nPowerOffDelay2[6];
	int		m_nPowerOffDelay3;
	int		r_nPowerOffDelay3[6];
	int		m_nPowerOffDelay4;
	int		r_nPowerOffDelay4[6];
	int		m_nPowerOffDelay5;
	int		r_nPowerOffDelay5[6];
	int		m_nPowerOffDelay6;
	int		r_nPowerOffDelay6[6];
	int		m_nPowerOffDelay7;
	int		r_nPowerOffDelay7[6];
	int		m_nPowerOffDelay8;
	int		r_nPowerOffDelay8[6];
	int		m_nPowerOffDelay9;
	int		r_nPowerOffDelay9[6];
	int		m_nPowerOffDelay10;
	int		r_nPowerOffDelay10[6];

	int		m_nPowerOffDelay;
	int		r_nPowerOffDelay[6];

	float	m_fVccVolt;
	float	r_fVccVolt[6];
	float	m_fVccVoltOffset;
	float	r_fVccVoltOffset[6];
	float	m_fVccLimitVoltLow;
	float	r_fVccLimitVoltLow[6];
	float	m_fVccLimitVoltHigh;
	float	r_fVccLimitVoltHigh[6];
	float	m_fVccLimitCurrLow;
	float	r_fVccLimitCurrLow[6];
	float	m_fVccLimitCurrHigh;
	float	r_fVccLimitCurrHigh[6];

	float	m_fVblVolt;
	float	r_fVblVolt[6];
	float	m_fVblVoltOffset;
	float	r_fVblVoltOffset[6];
	float	m_fVblLimitVoltLow;
	float	r_fVblLimitVoltLow[6];
	float	m_fVblLimitVoltHigh;
	float	r_fVblLimitVoltHigh[6];
	float	m_fVblLimitCurrLow;
	float	r_fVblLimitCurrLow[6];
	float	m_fVblLimitCurrHigh;
	float	r_fVblLimitCurrHigh[6];

	int		m_nAgingTimeHH;
	int		r_nAgingTimeHH[6];
	int		m_nAgingTimeMM;
	int		r_nAgingTimeMM[6];
	int		m_nAgingTimeMinute;
	int		r_nAgingTimeMinute[6];
	int		m_nAgingEndWaitTime;
	int		r_nAgingEndWaitTime[6];

	int		m_nOpeTemperatureUse;
	int		r_nOpeTemperatureUse[6];
	int		m_nOpeTemperatureMin;
	int		r_nOpeTemperatureMin[6];
	int		m_nOpeTemperatureMax;
	int		r_nOpeTemperatureMax[6];
	int		m_nOpeDoorUse;
	int		r_nOpeDoorUse[6];

	CString m_sPanelIDCode;

}MODELINFO, *LPMODELINFO;


typedef struct _ST_SYSTEMINFO_{
	// STATION 정보 설정 //
	CString m_sChamberNo;
	CString m_sEqpName;
	int m_nMesIDType;

	CString	m_sLastModelName[MAX_RACK];			// 마지막 M/C 모델명
	int m_sLastTimeOut[MAX_RACK];
	CString m_sChannelID[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	int m_nTempRecorderPort;
	int m_nTempControllerPort;

	// MES 정보 설정 //
	CString m_sMesServicePort;
	CString m_sMesNetWork;
	CString m_sMesDaemonPort;
	CString m_sMesLocalSubject;
	CString m_sMesRemoteSubject;

	// EAS 정보 설정 //
	int		m_nEasUse;
	CString m_sEasServicePort;
	CString m_sEasNetWork;
	CString m_sEasDaemonPort;
	CString m_sEasLocalSubject;
	CString m_sEasRemoteSubject;

	// RMS 정보 설정 //
	int		m_sRmsUse;
	CString m_sRmsServicePort;
	CString m_sRmsNetWork;
	CString m_sRmsDaemonPort;
	CString m_sRmsLocalSubject;
	CString m_sRmsRemoteSubject;
	CString m_sRmsEqp;

	// SW VERSION 설정 //
	CString m_SwVersion;

	// Aging Count 설정 //
	int m_Aging_Count;
	int m_Aging_Ng_Count;
	int m_Aging_Month_Count;
	int m_Aging_Month_Ng_Count;

	// Refresh Time 설정
	float m_fRefreshAgingStatusTime;
	float m_fRefreshPowerMeasureTime;
	int m_nTempLogInterval;
	int m_nSensingLogInterval;

}SYSTEMINFO, *LPSYSTEMINFO;


typedef struct _ST_INSPWORKINFO_
{
	int m_nConnectInfo[CONNECT_MAX];

	int m_nMainEthConnect[MAX_RACK][MAX_LAYER];
	float m_fTempReadVal[MAX_TEMP_SENSOR];

	int TempTest = 0;
	CString Ercp_Test_Message;
	int Msg_Test;

	float m_fTempReadValST590_2[MAX_TEMP_SENSOR];
	float m_fTempReadValST590_3[MAX_TEMP_SENSOR];
	float m_fTempReadValST590_4[MAX_TEMP_SENSOR];

	float m_fTempReadValST590_2_SET[MAX_TEMP_SENSOR];
	float m_fTempReadValST590_3_SET[MAX_TEMP_SENSOR];
	float m_fTempReadValST590_4_SET[MAX_TEMP_SENSOR];

	CString m_sMainFWVersion[MAX_RACK][MAX_LAYER];
	BOOL m_nFwVerifyResult[MAX_RACK];
	BOOL m_nFwVerEmpyt[MAX_RACK];

	ULONGLONG m_nAgingStartTick[MAX_RACK];

	int m_nAgingResumeOffsetSec[MAX_RACK];

	int	m_nAgingStatus[MAX_RACK];
	int m_nAgingSetTime[MAX_RACK];
	int m_nAgingRunTime[MAX_RACK];
	int m_nAgingOperatingMode[MAX_RACK];
	int m_nAgingDoorOpenTime[MAX_RACK];

	int m_nAgingTempMatchTime[MAX_RACK];
	int m_nAgingInYN[MAX_RACK];

	int m_nAgingEndWaitTime[MAX_RACK];
	int m_nOpeDoorUse[MAX_RACK];
	int m_nOpeTemperatureUse[MAX_RACK];
	int m_nOpeTemperatureMin[MAX_RACK];
	int m_nOpeTemperatureMax[MAX_RACK];
	float m_fOpeVccSetting[MAX_RACK];
	float m_fOpeVccSetMin[MAX_RACK];
	float m_fOpeVccSetMax[MAX_RACK];
	float m_fOpeIccSetMin[MAX_RACK];
	float m_fOpeIccSetMax[MAX_RACK];
	float m_fOpeVblSetting[MAX_RACK];
	float m_fOpeVblSetMin[MAX_RACK];
	float m_fOpeVblSetMax[MAX_RACK];
	float m_fOpeIblSetMin[MAX_RACK];
	float m_fOpeIblSetMax[MAX_RACK];

	int m_nTempSt590_01;
	int m_nTempSt590_02;
	int m_nTempSt590_03;

	int m_nAgingTempMeasCount[MAX_RACK];
	/*int m_nAgingPowerMeasCount[MAX_RACK];*/
	int m_nAgingPowerMeasCount[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingTempMin[MAX_RACK];
	float m_fOpeAgingTempMax[MAX_RACK];
	float m_fOpeAgingTempAvg[MAX_RACK];
	float m_fOpeAgingVccMin[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingVccMax[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingVccAvg[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingIccMin[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingIccMax[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingIccAvg[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingVblMin[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingVblMax[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingVblAvg[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingIblMin[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingIblMax[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	float m_fOpeAgingIblAvg[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];


	BOOL m_nRecvACK_Rack[MAX_RACK][MAX_LAYER];

	CString m_sRackID[MAX_RACK];
	CString m_sMesPanelID[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	CString m_sMesPchkRtnPID[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	BOOL m_bMesChannelUse[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];

	CString m_sMesBcrChID[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];

	BOOL m_nMesDspmOK[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];

	int m_nMeasVCC[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	int m_nMeasICC[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	int m_nMeasVBL[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	int m_nMeasIBL[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];

	BOOL m_ast_AgingLayerError[MAX_RACK][MAX_LAYER];
	BOOL m_ast_AgingStartStop[MAX_RACK][MAX_LAYER];
	BOOL m_ast_AgingChOnOff[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	BOOL m_ast_ChUseUnuse[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	BOOL m_ast_CableOpenCheck[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	BOOL m_ast_AgingChErrorResult[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	BOOL m_ast_AgingChErrorType[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	int  m_ast_AgingChErrorValue[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];

	BOOL m_ast_AgingTempError[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];

	int m_nChMainUiStatusOld[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	int m_nChErrorStatusOld[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];

	int m_nDioNeedInitial;
	int m_nDioOutputData;
	int m_nDioOutputMode;
	int m_nDioInputData[20];
	int m_nTowerLampStatus;
	int m_nDoorOpenClose[DOOR_MAX];

	int m_nAgingStatusS[6]; // Door Open, Close 
	int m_nLampColor; // Lamp
	int m_nAgingOutStatus[6] = { 0,0,0,0,0,0 };

	CString m_sDioFWVersion;

	BOOL m_bAlarmOccur;
	CString m_sAlarmMessage;

	BOOL m_SendRackID;

	CString m_RackID;
	CString m_ChID;
	CString m_LayerID;
	CString m_Layer_ChID;
	CString m_nPid;

	CString m_StopRackID = _T("13"); // pid 점등 초기값 설정

	int m_CableRackId;
	int m_CableRackLayer;
	int m_CableRackCh;

	BOOL m_PidFlag = false;

	BOOL m_nAgnOutYn[6];
	BOOL m_nAgnIn = FALSE;
	BOOL m_nAgnRack = FALSE;

	CString m_nAgnInStartTime[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	CString m_nAgnInStartPid[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	int m_AgnInStartRack;
	int m_AgnInStartLayer;
	int m_AgnInStartChannel;

	int m_AgingErrorRack;
	CString m_AgingErrorMsg;
	BOOL m_PidTestError[MAX_RACK] = { false, false, false, false, false, false };

}INSPWORKINFO, * LPINSPWORKINFO;

enum
{
	SUM_SW_VER = 0,
	SUM_FW_VER,
	SUM_MODEL,
	SUM_EQP_NAME,
	SUM_PID,
	SUM_RACK,
	SUM_LAYER,
	SUM_CHANNEL,
	SUM_USER_ID,
	SUM_AGING_TIME,
	SUM_START_TIME,
	SUM_END_TIME,
	SUM_RESULT,
	SUM_FAILED_MESSAGE,
	SUM_FAILED_MESSAGE_TIME,
	SUM_MEAS_VCC,
	SUM_MEAS_ICC,
	SUM_MEAS_VBL,
	SUM_MEAS_IBL,
	SUM_TEMP_MIN,
	SUM_TEMP_MAX,
	SUM_TEMP_AVG,
	SUM_VCC_MIN,
	SUM_VCC_MAX,
	SUM_VCC_AVG,
	SUM_ICC_MIN,
	SUM_ICC_MAX,
	SUM_ICC_AVG,
	SUM_VBL_MIN,
	SUM_VBL_MAX,
	SUM_VBL_AVG,
	SUM_IBL_MIN,
	SUM_IBL_MAX,
	SUM_IBL_AVG,
	SUM_INFO_MAX
};

typedef struct _ST_SUMMARY_
{
	CString	m_sumData[SUM_INFO_MAX];

} SUMMARYINFO, * LPSUMMARYINFO;

