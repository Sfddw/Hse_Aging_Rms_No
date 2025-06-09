#pragma once


#define RTN_OK						0
#define RTN_MSG_NOT_SEND			1
#define RTN_RCV_TIMEOUT				5

#define ECS_WAIT_TIME				5000	// 10Sec

#define	ECS_MODE_EAYT				0x01
#define	ECS_MODE_UCHK				0x02
#define	ECS_MODE_EDTI				0x03
#define	ECS_MODE_PCHK				0x04
#define	ECS_MODE_IISQ				0x05
#define	ECS_MODE_EICR				0x06
#define	ECS_MODE_BICD				0x07
#define	ECS_MODE_SCLR				0x08
#define	ECS_MODE_EESR				0x09
#define	ECS_MODE_EEER				0x0A
#define	ECS_MODE_EQCC				0x0B
#define	ECS_MODE_FLDR				0x0C
#define	ECS_MODE_EPCR				0x0D
#define ECS_MODE_AGN_IN				0x0E
#define ECS_MODE_EICR_ABNORMAL		0x0F
#define	ECS_MODE_MSET				0x10
#define	ECS_MODE_MILT				0x11
#define ECS_MODE_APDR				0x12
#define ECS_MODE_WDCR				0x13
#define	ECS_MODE_RPLT				0x14
#define	ECS_MODE_LPIR				0x15
#define	ECS_MODE_RPRQ				0x16
#define	ECS_MODE_SCRA				0x17
#define ECS_MODE_BDCR				0x18
#define ECS_MODE_AGN_INSP			0x19
#define ECS_MODE_INSP_INFO			0x1A
#define ECS_MODE_PINF				0x1B
#define ECS_MODE_AGN_CTR			0x1C
#define ECS_MODE_APTR				0x1D
#define	ECS_MODE_SCRP				0x1E
#define ECS_MODE_PPSET				0x1F
#define ECS_MODE_POIR				0x20
#define ECS_MODE_AGCM				0x21
#define ECS_MODE_AGN_OUT			0x22
#define ECS_MODE_EWOQ				0x23
#define ECS_MODE_EWCH				0x24
#define ECS_MODE_EPIQ				0x25
#define ECS_MODE_SSIR				0x26
#define ECS_MODE_DSPM				0x27
#define ECS_MODE_DMIN				0x28
#define ECS_MODE_DMOU				0x29
#define ECS_MODE_ERCP				0x2A

#define UTC_ZONE_VIETNAM			0x00
#define UTC_ZONE_CHINA				0x01
#define UTC_ZONE_KOREA				0x02

class CCimNetCommApi
{
public:
	CCimNetCommApi(void);
	~CCimNetCommApi(void);

	ICallGmesClass *gmes;
	ICallEASClass *eas;

	CLSID			clsid_ECS;
	HRESULT			hr;

	// ----------------------------------------
	
	BOOL Init(int nServerType);
	BOOL ConnectTibRv(int nServerType);
	BOOL CloseTibRv(int nServerType);
	INT64 GetbytesSent(CString strIPadress);
	INT64 GetbytesReceived(CString strIPadress);
	BOOL MessageSend(int nMode);
	BOOL MessageReceive();

	BOOL GetFieldData(CString* pszSource, CString sToken, int nMode=0);// TCHAR* wszToken
	CString GetHostSendMessage();
	CString GetHostRecvMessage();

	void getLocalSubjectIPAddress();

	// ----------------------------------------
	// ---------------------------------------
	// ----------------------------------------
	BOOL EAYT ();
	BOOL UCHK ();
	BOOL EDTI ();
	BOOL PCHK (int ipa_mode=FALSE, int ipa_value=FALSE);

	BOOL PCHK_B(CString PID);
	// ----------------------------------------

	BOOL IISQ ();
	BOOL EICR (int stationMode);
	BOOL RPLT (int station);
	BOOL RPRQ();
	BOOL SCRA(int station);
	BOOL SCRP(int station);
	BOOL MSET ();

	BOOL EICR_Abnormal ();
	BOOL APDR ();			// 2011-08-18 PDH. APDR Message 추가.
	BOOL MILT ();			// 2013.03.28. KSM. MILT Message 추가.
	BOOL WDCR ();			// 2014-11-20 PDH. WDCR Message 추가.
	BOOL FLDR ();

	// ----------------------------------------
	BOOL EWOQ();
	BOOL EWCH();
	BOOL EPIQ();
	BOOL EPCR();

	// ---------------------------------------
	BOOL BICD (char * pszBuff);
	BOOL SCLR ();
	BOOL EESR ();
	BOOL EEER ();
	BOOL EQCC ();
	BOOL AGCM ();
	BOOL AGN_IN ();
	BOOL AGN_OUT ();
	BOOL AGN_INSP ();
	BOOL LPIR();
	BOOL BDCR();
	BOOL INSP_INFO();
	BOOL PINF();
	BOOL AGN_CTR();
	BOOL APTR();
	BOOL POIR();
	BOOL SSIR();

	BOOL DSPM();
	BOOL DMIN();
	BOOL DMOU();

	BOOL UNDO(int rack, int layer, int ch);

	// ---------------------------------------
	BOOL ERCP();

	// ---------------------------------------
	void SetLocalTest(int nServerType);

	// ---------------------------------------
	void MakeClientTimeString();
	void SetMesHostInterface();
	void SetEasHostInterface();
	void SetLocalTimeZone(int timeZone);
	void SetLocalTimeData(CString strTime);
	void SetMachineName(CString strBuff);
	void SetUserId (CString strBuff);
	void SetComment (CString strBuff);
	void SetAgingChangeTime (CString strBuff);
	void SetAptrAgingTime(CString strBuff);
	void SetRemark (CString strBuff);
	void SetNGPortOut (CString strBuff);
	void SetRwkCode(CString strBuff);
	void SetFLDRFileName(CString strBuff);
	void SetPanelID(CString strBuff);
	void SetChannelID (CString strBuff);
	void SetBLID(CString strBuff);
	void SetSerialNumber(CString strBuff);
	void SetModelName(CString strBuff);
	void SetPalletID(CString strBuff);
	void SetPF(CString strBuff);
	void SetDefectPattern(CString strBuff);
	void SetPvcomAdjustValue(CString strBuff);
	void SetPvcomAdjustDropValue(CString strBuff);
	void SetPatternInfo(CString strBuff);
	void SetEdidStatus(CString strBuff);
	void SetOverHaulFlag(CString strBuff);
	void SetBaExiFlag(CString strBuff);
	void SetBuyerSerialNo(CString strBuff);
	void SetVthValue(CString strBuff);
	void SetBDInfo(CString strBuff);
	void SetWDRInfo(CString strBuff);
	void SetWDREnd(CString strBuff);
	void SetAPDInfo(CString strBuff);
	void SetDefectCommentCode(CString strBuff);
	void SetFullYN(CString strBuff);
	void SetGibCode(CString strBuff);
	void SetToOper(CString strBuff);
	CString GetToOper();
	CString GetComment();
	void SetRepairCD(CString strBuff);
	void SetRespDept(CString strBuff);
	void SetMaterialInfo(CString strBuff);
	void SetSSFlag(CString strBuff);
	void SetFromOper(CString strBuff);
	void SetTACT(CString strBuff);
	void SetAutoRespDeptFlag(CString strBuff);
	void SetAgingLevelInfo(CString strBuff);
	void SetPOIRProcessCode(CString strBuff);
	void SetActFlag(CString strBuff);
	void SetWorkOrder(CString strBuff);
	void SetPFactory(CString strBuff);
	void SetCategory(CString strBuff);
	void SetWorkerInfo(CString strBuff);
	void SetDurableID(CString strBuff);
	void SetSlotNo(CString strBuff);

	// ---------------------------------------
	CString GetRwkCode();
	CString GetPF();
	CString GetMachineName();



	CString m_strRemoteSubject;
	CString m_strLocalSubject;
	CString m_strLocalSubjectIP;
	CString m_strLocalSubjectMesF;
	CString m_strLocalSubjectEasF;

protected:
	BOOL	m_bIsGmesLocalTestMode;
	BOOL	m_bIsEasLocalTestMode;


	

	
	CString m_strNetwork;
	CString m_strServicePort;
	CString m_strDaemon;
	CString m_strLocalIP;

	CString m_strRemoteSubjectEAS;
	CString m_strLocalSubjectEAS;
	CString m_strNetworkEAS;
	CString m_strServicePortEAS;
	CString m_strDaemonEAS;
	CString m_strLocalIPEAS;

	CString m_strNgComment;

	CString m_strClientDate;
	CString m_strClientOldDate;
	CString m_strClientNewDate;

	CString m_strMachineName;
	CString m_strUserID;
	CString m_strComment;
	CString m_strAgingChangeTime;
	CString m_strAptrAgingTime;
	CString m_strRemark;
	CString m_strNGPortOut;
	CString m_strRwkCode;
	CString m_strSSFlag;
	CString m_strFLDRFileName;
	CString m_strPanelID;
	CString m_strFrom_Oper;
	CString m_strBLID;
	CString m_strSerialNumber;
	CString m_strModelName;
	CString m_strPalletID;
	CString m_strPF;
	CString m_strDefectPattern;
	CString m_strPvcomAdjustValue;
	CString m_strPvcomAdjustDropValue;
	CString m_strPatternInfo;
	CString m_strEdidStatus;
	CString m_strOverHaulFlag;
	CString m_strExpectedRwk;
	CString m_strBaExiFlag;
	CString m_strBuyerSerialNo;
	CString m_strVthValue;
	CString m_strBDInfo;
	CString m_strWDRInfo;
	CString m_strWDREnd;
	CString m_strAPDInfo;
	CString m_strFullYN;
	CString m_strGibCode;
	CString m_strToOper;
	CString m_strRepairCD;
	CString m_strRespDept;
	CString m_strMaterialInfo;
	CString m_strTactTime;
	CString m_strACT_FLAG;
	CString m_strOffrsCnt;
	CString m_strRepairTypeCD;
	CString m_strAutoRespDeptFlag;
	CString m_strAgingLevelInfo;
	CString m_strPOIRProcessCode;
	CString m_strActFlag;
	CString m_strChannelID;
	CString m_strWorkOrder;
	CString m_strPFactory;
	CString m_strCategory;
	CString m_strWorkerInfo;
	CString m_strDurableID;
	CString m_strSlotNo;

	CString m_strHostSendMessage;
	CString m_strHostRecvMessage;
	CString m_strEAYT;
	CString m_strUCHK;
	CString m_strEDTI;
	CString m_strFLDR;
	CString m_strPCHK;
	CString m_strEICR;
	CString m_strRPLT;
	CString m_strMSET;
	CString m_strAGCM;
	CString m_strAGN_IN;
	CString m_strAGN_OUT;
	CString m_strAGN_INSP;
	CString m_strAPDR;
	CString m_strWDCR;
	CString m_strLPIR;
	CString m_strRPRQ;
	CString m_strSCRA;
	CString m_strSCRP;
	CString m_strBDCR;
	CString m_strINSP_INFO;
	CString m_strPINF;
	CString m_strAGN_CTR;
	CString m_strAPTR;
	CString m_strPPSET;
	CString m_strPOIR;
	CString m_strEWOQ;
	CString m_strEWCH;
	CString m_strEPIQ;
	CString m_strEPCR;
	CString m_strSSIR;
	CString m_strDSPM;
	CString m_strDMIN;
	CString m_strDMOU;
	CString m_strERCP;

	CString m_strDefectComCode;

};
