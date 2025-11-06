#include "pch.h"
#include "HseAging.h"

#include "atlbase.h"
#include "atlcom.h"
#include <string>

#include "CIMNetCommApp.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Message Receive Class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
const UINT SINK_ID = 1;

CString m_sReceiveMessage;
BOOL	m_nMessageReceiveFlag;


static _ATL_FUNC_INFO HandleTibRvMsgEvent = { CC_STDCALL, VT_EMPTY, 1, { VT_BSTR } };
static _ATL_FUNC_INFO HandleTibRvStateEvent = { CC_STDCALL, VT_EMPTY, 1, { VT_BSTR} };


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


CCimNetCommApi::CCimNetCommApi(void)
{
	m_nMessageReceiveFlag	= 0;
	m_bIsGmesLocalTestMode		= FALSE;
	m_bIsEasLocalTestMode		= FALSE;
}


CCimNetCommApi::~CCimNetCommApi(void)
{
}

BOOL CCimNetCommApi::ConnectTibRv(int nServerType)
{
	CString sLog;

	if(nServerType == SERVER_MES)
	{

		VARIANT_BOOL resultConnect = gmes->Connect();

		if(resultConnect == VARIANT_TRUE){

			m_pApp->Gf_writeMLog(_T("<MES> MES Server Connection Succeeded"));

			m_pApp->m_bIsGmesConnect = TRUE;
			return TRUE;
		}
		else
		{
			m_pApp->Gf_writeMLog(_T("<MES> MES Server Connection Fail"));
			m_pApp->m_bIsGmesConnect = FALSE;
		}
	}

	else if(nServerType == SERVER_EAS)
	{

		VARIANT_BOOL resultConnect = eas->Connect();

		if(resultConnect == VARIANT_TRUE)
		{
			m_pApp->Gf_writeMLog(_T("<EAS> EAS Server Connection Succeeded"));
			m_pApp->m_bIsEasConnect = TRUE;
			return TRUE;
		}
		else
		{
			m_pApp->Gf_writeMLog(_T("<EAS> EAS Server Connection Fail"));
			m_pApp->m_bIsEasConnect = FALSE;
		}
	}

	else if (nServerType == SERVER_RMS)
	{
		VARIANT_BOOL resultConnect = rms->Connect();

		if (resultConnect == VARIANT_TRUE)
		{
			m_pApp->Gf_writeMLog(_T("<RMS> RMS Server Connection Succeeded"));
			m_pApp->m_blsRmsConnect = TRUE;
			return TRUE;
		}
		else
		{
			m_pApp->Gf_writeMLog(_T("<RMS> RMS Server Connection Fail"));
			m_pApp->m_blsRmsConnect = FALSE;
		}
	}

	return FALSE;
}

BOOL CCimNetCommApi::CloseTibRv(int nServerType)
{
	if(nServerType == SERVER_MES){

		VARIANT_BOOL resultDisConnect = gmes->Terminate();
		
		if(resultDisConnect == VARIANT_TRUE)
			return TRUE;
	}

	else if(nServerType == SERVER_EAS){

		VARIANT_BOOL resultDisConnect = eas->Terminate();
	
		if(resultDisConnect == VARIANT_TRUE)
			return TRUE;
	}

	else if (nServerType == SERVER_RMS)
	{
		VARIANT_BOOL resultDisConnect = rms->Terminate();

		if (resultDisConnect == VARIANT_TRUE)
			return TRUE;
	}

	return FALSE;
}

INT64 CCimNetCommApi::GetbytesSent(CString strIPadress)
{
	INT64 bytesSent = gmes->getbytesSent((_bstr_t)strIPadress);

	return bytesSent;
}

INT64 CCimNetCommApi::GetbytesReceived(CString strIPadress)
{
	INT64 bytesReceived = gmes->getbytesReceived((_bstr_t)strIPadress);

	return bytesReceived;
}

void CCimNetCommApi::getLocalSubjectIPAddress()

{	// Add 'ws2_32.lib' to your linker options

	WSADATA WSAData;
	CString sLog;
	m_strLocalSubjectIP = (_T(""));
	m_strLocalSubject = (_T(""));
	m_strLocalSubjectMesF = (_T(""));
	m_strLocalSubjectEasF = (_T(""));

	// Initialize winsock dll
	if(::WSAStartup(MAKEWORD(1, 0), &WSAData))
	{
		m_pApp->Gf_writeMLog(_T("<MES>  Winsock dll Initialize Fail"));
	}

	// Get local host name
	char szHostName[128] = "";

	if(::gethostname(szHostName, sizeof(szHostName)))
	{
		m_pApp->Gf_writeMLog(_T("<MES> Get Local Host Name Error"));
	}

	// Get local IP addresses
	struct sockaddr_in SocketAddress;
	struct hostent     *pHost        = 0;

	pHost = ::gethostbyname(szHostName);
	if(!pHost)
	{
		m_pApp->Gf_writeMLog(_T("<MES> Get Local IP Addresses Error"));
	}

	char aszIPAddresses[20]; // maximum of ten IP addresses
	for(int iCnt = 0; ((pHost->h_addr_list[iCnt]) && (iCnt < 10)); ++iCnt)
	{
		memcpy(&SocketAddress.sin_addr, pHost->h_addr_list[iCnt], pHost->h_length);
		sprintf_s(aszIPAddresses, "%s", inet_ntoa(SocketAddress.sin_addr));

		//if(aszIPAddresses[0] == '1' && aszIPAddresses[1] == '9' && aszIPAddresses[2] =='2' && aszIPAddresses[3] == '.'){
		if (aszIPAddresses[0] == '1' && aszIPAddresses[1] == '0' && aszIPAddresses[2] == '.') {
			
			m_strLocalSubjectIP.Format(_T("%S"), aszIPAddresses);
			m_pApp->m_strLocalSubjectIP = m_strLocalSubjectIP;

			sLog.Format(_T("<MES> STATION Local IP Addresses = %s"), m_strLocalSubjectIP);
			m_pApp->Gf_writeMLog(sLog);
		}
	}

	// Cleanup
	WSACleanup();

}

BOOL CCimNetCommApi::Init(int nServerType)
{
	CString sLog;

	// Host 접속 정보를 가져온다.
	if(nServerType == SERVER_MES){
		
		CoInitialize(NULL);
		
		if (DEBUG_GMES_TEST_SERVER != TRUE)
			getLocalSubjectIPAddress();

		SetMesHostInterface();
		
#if (DEBUG_GMES_TEST_SERVER==1)
		m_strLocalSubjectMesF = (_T("EQP.TEST"));
#else
		if(m_strLocalSubjectIP.GetLength() == 0)
			m_strLocalSubjectMesF.Format(_T("%s"), m_strLocalSubject);
		else
			m_strLocalSubjectMesF.Format(_T("%s.%s"), m_strLocalSubject, m_strLocalSubjectIP);
#endif


		HRESULT mesHr = CoCreateInstance(CLSID_DllGmes, NULL, CLSCTX_INPROC_SERVER, IID_ICallGmesClass, reinterpret_cast<void**>(&gmes));
		
		if (SUCCEEDED(mesHr)){

			gmes->SetTimeOut(5);

			VARIANT_BOOL resultIni = gmes->Init(
				(_bstr_t)m_strServicePort,
				(_bstr_t)m_strNetwork,
				(_bstr_t)m_strDaemon,
				(_bstr_t)m_strRemoteSubject,
				(_bstr_t)m_strLocalSubjectMesF
				);

			if (resultIni == VARIANT_TRUE)
			{
				return TRUE;
			}
				

		}

		m_pApp->Gf_writeMLog(_T("<MES> MES TIB INIT Fail"));
		return FALSE;
	}
	
	else if(nServerType == SERVER_EAS){
		
		SetEasHostInterface();
		
#if (DEBUG_GMES_TEST_SERVER==1)
		m_strLocalSubjectEasF = (_T("EQP.TEST"));
#else
		if(m_strLocalSubjectIP.GetLength() == 0)
			m_strLocalSubjectEasF.Format(_T("%s"), m_strLocalSubjectEAS);
		else
			m_strLocalSubjectEasF.Format(_T("%s.%s"), m_strLocalSubjectEAS, m_strLocalSubjectIP);
#endif

		HRESULT easHr = CoCreateInstance(CLSID_DllEas, NULL, CLSCTX_INPROC_SERVER, IID_ICallEASClass, reinterpret_cast<void**>(&eas));

		if (SUCCEEDED(easHr)) {

			eas->SetTimeOut(5);

			VARIANT_BOOL resultIni = eas->Init(
				(_bstr_t)m_strServicePortEAS,
				(_bstr_t)m_strNetworkEAS,
				(_bstr_t)m_strDaemonEAS,
				(_bstr_t)m_strRemoteSubjectEAS,
				(_bstr_t)m_strLocalSubjectEasF
				);
			
			if(resultIni == VARIANT_TRUE)
				return TRUE;

		}

		m_pApp->Gf_writeMLog(_T("<EAS> EAS TIB INIT Fail"));
		return FALSE;
	}

	else if (nServerType == SERVER_RMS)
	{
		SetRmsHostInterface();
#if (DEBUG_GMES_TEST_SERVER==1)
		m_strLocalSubjectRMS = (_T("EQP.TEST"));
#else
		if (m_strLocalSubjectIP.GetLength() == 0)
			m_strLocalSubjectRmsF.Format(_T("%s"), m_strLocalSubjectRMS);
		else
			m_strLocalSubjectRmsF.Format(_T("%s,%s"), m_strLocalSubjectRMS, m_strLocalSubjectIP);
#endif

		HRESULT rmsHr = CoCreateInstance(CLSID_DllRms, NULL, CLSCTX_INPROC_SERVER, IID_ICallRMSClass, reinterpret_cast<void**>(&rms));

		if (SUCCEEDED(rmsHr)) {
			rms->SetTimeOut(5);

			VARIANT_BOOL resultIni = rms->Init(
				(_bstr_t)m_strServicePortRMS,
				(_bstr_t)m_strNetworkRMS,
				(_bstr_t)m_strDaemonRMS,
				(_bstr_t)m_strRemoteSubjectRMS,
				(_bstr_t)m_strLocalSubjectRmsF
			);

			if (resultIni == VARIANT_TRUE)
				return TRUE;
		}

		m_pApp->Gf_writeMLog(_T("<RMS> RMS TIB INIT Fail"));
		return FALSE;
	}

	return FALSE;
}

BOOL CCimNetCommApi::MessageSend (int nMode)	// Event
{
	_bstr_t bstBuff = (LPSTR)(LPCTSTR) m_strRemoteSubject;
	BSTR m_remoteTemp = bstBuff.copy ();
	CString sLog = _T("");
	//	_bstr_t m_remoteTemp = bstBuff.copy ();

	CString strBuff (_T (""));

	switch (nMode)
	{
	case ECS_MODE_EAYT:
		m_strHostSendMessage = m_strEAYT;
		break;
	case ECS_MODE_UCHK:
		m_strHostSendMessage = m_strUCHK;
		break;
	case ECS_MODE_EDTI:
		m_strHostSendMessage = m_strEDTI;
		break;
	case ECS_MODE_FLDR:
		m_strHostSendMessage = m_strFLDR;
		break;
	case ECS_MODE_PCHK:
		m_strHostSendMessage = m_strPCHK;
		break;
	case ECS_MODE_EICR:
		m_strHostSendMessage = m_strEICR;
		break;
	case ECS_MODE_RPLT:
		m_strHostSendMessage = m_strRPLT;
		break;
	case ECS_MODE_MSET:
		m_strHostSendMessage = m_strMSET;
		break;
	case ECS_MODE_AGCM:
		m_strHostSendMessage = m_strAGCM;
		break;
	case ECS_MODE_AGN_IN:
		m_strHostSendMessage = m_strAGN_IN;
		break;
	case ECS_MODE_AGN_OUT:
		m_strHostSendMessage = m_strAGN_OUT;
		break;
	case ECS_MODE_AGN_INSP:
		m_strHostSendMessage = m_strAGN_INSP;
		break;
	case ECS_MODE_APDR:
		m_strHostSendMessage = m_strAPDR;
		break;
	case ECS_MODE_WDCR:
		m_strHostSendMessage = m_strWDCR;
		break;
	case ECS_MODE_LPIR:
		m_strHostSendMessage = m_strLPIR;
		break;
	case ECS_MODE_RPRQ:
		m_strHostSendMessage = m_strRPRQ;
		break;
	case ECS_MODE_SCRA:
		m_strHostSendMessage = m_strSCRA;
		break;
	case ECS_MODE_SCRP:
		m_strHostSendMessage = m_strSCRP;
		break;
	case ECS_MODE_BDCR:
		m_strHostSendMessage = m_strBDCR;
		break;
	case ECS_MODE_INSP_INFO:
		m_strHostSendMessage = m_strINSP_INFO;
		break;
	case ECS_MODE_PINF:
		m_strHostSendMessage = m_strPINF;
		break;
	case ECS_MODE_AGN_CTR:
		m_strHostSendMessage = m_strAGN_CTR;
		break;
	case ECS_MODE_APTR:
		m_strHostSendMessage = m_strAPTR;
		break;
	case ECS_MODE_PPSET:
		m_strHostSendMessage = m_strPPSET;
		break;
	case ECS_MODE_POIR:
		m_strHostSendMessage = m_strPOIR;
		break;
	case ECS_MODE_EWOQ:
		m_strHostSendMessage = m_strEWOQ;
		break;
	case ECS_MODE_EWCH:
		m_strHostSendMessage = m_strEWCH;
		break;
	case ECS_MODE_EPIQ:
		m_strHostSendMessage = m_strEPIQ;
		break;
	case ECS_MODE_EPCR:
		m_strHostSendMessage = m_strEPCR;
		break;
	case ECS_MODE_SSIR:
		m_strHostSendMessage = m_strSSIR;
		break;
	case ECS_MODE_DSPM:
		m_strHostSendMessage = m_strDSPM;
		break;
	case ECS_MODE_DMIN:
		m_strHostSendMessage = m_strDMIN;
		break;
	case ECS_MODE_DMOU:
		m_strHostSendMessage = m_strDMOU;
		break;
	case ECS_MODE_RMSO:
		m_strHostSendMessage = m_strRMSO;
		break;
	default:
		return RTN_MSG_NOT_SEND;	// 통신 NG
	}

	sLog.Format(_T("<HOST_R> %s"), m_strHostSendMessage);
	m_pApp->Gf_writeMLog(sLog);

	m_strHostRecvMessage.Format(_T("EMPTY"));

	Sleep (10);

	if(nMode != ECS_MODE_APDR && nMode != ECS_MODE_RMSO)
	{
		if (m_pApp->m_bIsGmesConnect == FALSE)
			return RTN_MSG_NOT_SEND;

		VARIANT_BOOL bRetCode = gmes->SendTibMessage((_bstr_t)m_strHostSendMessage);

		do{
			if(gmes->GetreceivedDataFlag() == VARIANT_TRUE){
				m_sReceiveMessage = (LPCTSTR)gmes->GetReceiveData();
				break;
			}
			if(bRetCode == VARIANT_FALSE){
				
				m_pApp->Gf_writeMLog(_T("<HOST_S> Did not send a MES Message. Retry !!!"));
				break;
			}

		}while(1);
		

		if(bRetCode == VARIANT_FALSE){

			bRetCode = gmes->SendTibMessage((_bstr_t)m_strHostSendMessage);

			sLog.Format(_T("<HOST_S> %s"), m_strHostSendMessage);
			m_pApp->Gf_writeMLog(sLog);
			
			do{
				if(gmes->GetreceivedDataFlag() == VARIANT_TRUE){
					m_sReceiveMessage = (LPCTSTR)gmes->GetReceiveData();
					break;
				}
				if(bRetCode == VARIANT_FALSE){

					AfxMessageBox (_T ("Did not send a message !!!"));
					return RTN_MSG_NOT_SEND;	// 통신 NG
				}

			}while(1);
		}


	}
	else if (nMode == ECS_MODE_RMSO) // RMS 온도값 메시지
	{
		if (m_pApp->m_bIsGmesConnect == FALSE)
			return RTN_MSG_NOT_SEND;

		VARIANT_BOOL bRetCode = rms->SendTibMessage((_bstr_t)m_strHostSendMessage);

		do {
			if (rms->GetreceivedDataFlag() == VARIANT_TRUE) {
				m_sReceiveMessage = (LPCTSTR)rms->GetReceiveData();
				break;
			}
			if (bRetCode == VARIANT_FALSE)
			{
				m_pApp->Gf_writeMLog(_T("<HOST_S> Did not send a RMS Message. Retry !!! (RMS)"));
				break;
			}
		} while (1);

		if (bRetCode == VARIANT_FALSE) {
			bRetCode = rms->SendTibMessage((_bstr_t)m_strHostSendMessage);

			sLog.Format(_T("<HOST_S> %s"), m_strHostSendMessage);
			m_pApp->Gf_writeMLog(sLog);

			do {
				if (rms->GetreceivedDataFlag() == VARIANT_TRUE) {
					m_sReceiveMessage = (LPCTSTR)rms->GetReceiveData();
					break;
				}
				if (bRetCode == VARIANT_FALSE) {
					AfxMessageBox(_T("Did not send a message !!! (RMS)"));
					return RTN_MSG_NOT_SEND;   // 통신 NG 
				}
			} while (1);
		}
	}
	else
	{
		if (m_pApp->m_bIsEasConnect == FALSE)
			return RTN_MSG_NOT_SEND;

		VARIANT_BOOL bRetCode = eas->SendTibMessage((_bstr_t)m_strHostSendMessage);

		do{
			if(eas->GetreceivedDataFlag() == VARIANT_TRUE)
			{
				m_sReceiveMessage = (LPCTSTR)eas->GetReceiveData();
				break;
			}
			if(bRetCode == VARIANT_FALSE)
			{
				m_pApp->Gf_writeMLog(_T("<HOST_S> Did not send a EAS Message. Retry !!!"));
				break;
			}

		}while(1);

		if(bRetCode == VARIANT_FALSE){

			bRetCode = eas->SendTibMessage((_bstr_t)m_strHostSendMessage);

			sLog.Format(_T("<HOST_S> %s"), m_strHostSendMessage);
			m_pApp->Gf_writeMLog(sLog);

			do{
				if(eas->GetreceivedDataFlag() == VARIANT_TRUE){
					m_sReceiveMessage = (LPCTSTR)eas->GetReceiveData();
					break;
				}
				if(bRetCode == VARIANT_FALSE){

					AfxMessageBox (_T ("Did not send a message !!!"));
					return RTN_MSG_NOT_SEND;	// 통신 NG
				}

			}while(1);
		}
	}

	//if (nMode == ECS_MODE_RMSO)
	//{
	//	if (m_pApp->m_bIsGmesConnect == FALSE)
	//		return RTN_MSG_NOT_SEND;

	//	VARIANT_BOOL bRetCode = rms->SendTibMessage((_bstr_t)m_strHostSendMessage);

	//	do {
	//		if (rms->GetreceivedDataFlag() == VARIANT_TRUE) {
	//			m_sReceiveMessage = (LPCTSTR)rms->GetReceiveData();
	//			break;
	//		}
	//		if (bRetCode == VARIANT_FALSE)
	//		{
	//			m_pApp->Gf_writeMLog(_T("<HOST_S> Did not send a RMS Message. Retry !!!"));
	//			break;
	//		}
	//	} while (1);

	//	if (bRetCode == VARIANT_FALSE) {
	//		bRetCode = rms->SendTibMessage((_bstr_t)m_strHostSendMessage);

	//		sLog.Format(_T("<HOST_S> %s"), m_strHostSendMessage);
	//		m_pApp->Gf_writeMLog(sLog);

	//		do {
	//			if (rms->GetreceivedDataFlag() == VARIANT_TRUE) {
	//				m_sReceiveMessage = (LPCTSTR)rms->GetReceiveData();
	//				break;
	//			}
	//			if (bRetCode == VARIANT_FALSE) {
	//				AfxMessageBox(_T("Did not send a message !!! (RMS)"));
	//				return RTN_MSG_NOT_SEND;   // 통신 NG 
	//			}
	//		} while (1);
	//	}
	//}

	m_strHostRecvMessage = m_sReceiveMessage;

	return RTN_OK;		// normal...
}

BOOL CCimNetCommApi::MessageReceive() 
{
	if(m_sReceiveMessage.IsEmpty() == TRUE)
		return FALSE;

	m_strHostRecvMessage.Format(_T("%s"), m_sReceiveMessage);
	m_nMessageReceiveFlag = 1;

	return TRUE;
}

void CCimNetCommApi::MakeClientTimeString ()
{
	CString strBuff;
	CTime time = CTime::GetCurrentTime ();

	strBuff.Format(_T("%04d%02d%02d%02d%02d%02d"),
		time.GetYear (),
		time.GetMonth (),
		time.GetDay (),
		time.GetHour (),
		time.GetMinute (),
		time.GetSecond ()
		);

	m_strClientDate.Format (_T("%s"), strBuff);
}

BOOL CCimNetCommApi::GetFieldData (CString* pstrReturn, CString sToke, int nMode)
{
 	char * pszBuff = new char [4096];
 	ZeroMemory (pszBuff, 4096);

	m_strNgComment.Format (_T("0"));
	CString strBuff;
 	if (0 == nMode)
 	{
 		strBuff.Format (m_strHostRecvMessage);
 		pstrReturn->Empty();
 	}
 	else
 	{
 		strBuff.Format(_T("%s"), *pstrReturn);
 	}
 
	int nPos = strBuff.Find(sToke, 0);
 
	if (0 > nPos)
	{
		// no data
		delete[] pszBuff;
		return TRUE;
	}

	char temp [2] = {0,};
	if ((sToke == _T("PF=")) || (sToke == _T("COMP_CNT=")) || (sToke == _T("COMP_INTERLOCK_CNT=")))
		sToke.Replace(_T("="), _T(""));

	int nStartPos = nPos + (int)sToke.GetLength() + 1;
	int nEndPos= 0;

	switch (strBuff.GetAt (nStartPos))
	{
		case '0':
		{
			if (!sToke.Compare(_T("DEPOSITION_GROUP")))
			{
				nEndPos = strBuff.Find (' ', nStartPos);

				if (nEndPos <= 0)
				{
					nEndPos = strBuff.GetLength ();
				}
				else
				{
				}

				pstrReturn->Format(_T("%s"), strBuff.Mid (nStartPos, nEndPos-nStartPos));
				break;
			}

			if (!sToke.Compare(_T("RTN_CD")))
			{
				m_strNgComment.Format (_T("0"));	// normal 로 셋팅.

				if (pstrReturn->GetLength() <= 0)
				{
					pstrReturn->Format(_T("%s"), m_strNgComment);
				}

				delete [] pszBuff;
				return FALSE;
			}
			
			if (!sToke.Compare(_T("OAGING_TIME")))
			{
				nEndPos = strBuff.Find (' ', nStartPos);

				if (nEndPos <= 0)
				{
					nEndPos = strBuff.GetLength ();
				}
				pstrReturn->Format(_T("%s"), strBuff.Mid (nStartPos, nEndPos-nStartPos));
			}

			if (!sToke.Compare(_T("USER_ID")))
			{
				nEndPos = strBuff.Find(' ', nStartPos);

				if (nEndPos <= 0)
				{
					nEndPos = strBuff.GetLength();
				}
				pstrReturn->Format(_T("%s"), strBuff.Mid(nStartPos, nEndPos - nStartPos));
			}

			if (pstrReturn->GetLength() <= 0)
			{
				pstrReturn->Format(_T("%s"), m_strNgComment);
			}

			delete [] pszBuff;
			return FALSE;

		} break;

		case '[':
		{
			// [ 로 묶여진 error, error 내용안에 space 가 포함될수 있음.

			nEndPos = strBuff.Find (']', nStartPos);
			if (0 >= nEndPos)
			{
				nEndPos = strBuff.GetLength ();
			}
			else
			{
			}

			pstrReturn->Format(_T("%s"), strBuff.Mid (nStartPos+1, (nEndPos-nStartPos)-1));
		} break;

 		default :
 		{
 			// [ ] 가 없는 error messgae... 
 			// 에러내용 안에 space 가 포함될수 없으므로 space 를 token 으로 다시 data 검색..
 
 			nEndPos = strBuff.Find (' ', nStartPos);

			if (nEndPos <= 0)
			{
				nEndPos = strBuff.GetLength ();
			}
			else
			{
			}

 			pstrReturn->Format(_T("%s"), strBuff.Mid (nStartPos, nEndPos-nStartPos));
 		} break;
 	}

	if (m_strNgComment.GetLength() <= 0)
	{
		pstrReturn->Format(_T("%s"), m_strNgComment);
	}
 	delete[] pszBuff;

	return FALSE;
}

CString CCimNetCommApi::GetHostSendMessage()
{
	return m_strHostSendMessage;
}

CString CCimNetCommApi::GetHostRecvMessage()
{
	return m_strHostRecvMessage;
}

void CCimNetCommApi::SetMesHostInterface()
{
	
	if(m_bIsGmesLocalTestMode==TRUE)	return;

	Read_SysIniFile(_T("MES"), _T("MES_SERVICE_PORT"),			&m_strServicePort);
	Read_SysIniFile(_T("MES"), _T("MES_NETWORK"),				&m_strNetwork);
	Read_SysIniFile(_T("MES"), _T("MES_DAEMON_PORT"),			&m_strDaemon);
	Read_SysIniFile(_T("MES"), _T("MES_LOCAL_SUBJECT"),			&m_strLocalSubject);
	Read_SysIniFile(_T("MES"), _T("MES_REMOTE_SUBJECT"),		&m_strRemoteSubject);
	Read_SysIniFile(_T("MES"), _T("MES_LOCAL_IP"),				&m_strLocalIP);
}

void CCimNetCommApi::SetEasHostInterface()
{

	if(m_bIsEasLocalTestMode==TRUE)	return;

 	Read_SysIniFile(_T("EAS"), _T("EAS_SERVICE_PORT"),			&m_strServicePortEAS);
	Read_SysIniFile(_T("EAS"), _T("EAS_NETWORK"),				&m_strNetworkEAS);
	Read_SysIniFile(_T("EAS"), _T("EAS_DAEMON_PORT"),			&m_strDaemonEAS);
	Read_SysIniFile(_T("EAS"), _T("EAS_LOCAL_SUBJECT"),			&m_strLocalSubjectEAS);
	Read_SysIniFile(_T("EAS"), _T("EAS_REMOTE_SUBJECT"),		&m_strRemoteSubjectEAS);
}

void CCimNetCommApi::SetRmsHostInterface()
{
	if (m_blsRmsLocalTestMode == TRUE) return;

	Read_SysIniFile(_T("RMS"), _T("RMS_SERVICE_PORT"), &m_strServicePortRMS);
	Read_SysIniFile(_T("RMS"), _T("RMS_NETWORK"), &m_strNetworkRMS);
	Read_SysIniFile(_T("RMS"), _T("RMS_DAEMON_PORT"), &m_strDaemonRMS);
	Read_SysIniFile(_T("RMS"), _T("RMS_LOCAL_SUBJECT"), &m_strLocalSubjectRMS);
	Read_SysIniFile(_T("RMS"), _T("RMS_REMOTE_SUBJECT"), &m_strRemoteSubjectRMS);
}

void CCimNetCommApi::SetLocalTest(int nServerType)
{

	if(nServerType==SERVER_MES)
	{
		m_bIsGmesLocalTestMode	= TRUE;

		m_strLocalSubject	= (_T("EQP.TEST"));
		m_strNetwork		= (_T(""));
		m_strRemoteSubject	= (_T("MES.TEST"));
		m_strServicePort	= (_T("7600"));
		m_strDaemon			= (_T("tcp::7600"));
	}
	else if(nServerType==SERVER_EAS)
	{
		m_bIsEasLocalTestMode	= TRUE;

		m_strLocalSubjectEAS	= (_T("EQP.TEST"));
		m_strNetworkEAS			= (_T(""));
		m_strRemoteSubjectEAS	= (_T("MES.TEST"));
		m_strServicePortEAS		= (_T("7800"));
		m_strDaemonEAS			= (_T("tcp::7800"));
	}
	else if (nServerType == SERVER_RMS)
	{
		m_blsRmsLocalTestMode = TRUE;

		m_strLocalSubjectRMS	= (_T("EQP.TEST"));
		m_strNetworkRMS			= (_T(""));
		m_strRemoteSubjectRMS	= (_T("RMS.TEST"));
		m_strServicePortRMS		= (_T("7900"));
		m_strDaemonRMS			= (_T("tcp::7900"));
	}
}

void CCimNetCommApi::SetLocalTimeZone(int timeZone)
{
	CString exeFile;
	CString param;
	CString filename;

	exeFile.Format(_T("tzutil.exe"));

	// parameter set
	if (timeZone == UTC_ZONE_VIETNAM)			param.Format(_T("/s \"SE Asia Standard Time\""));	// 베트남 UTC+07:00
	else if (timeZone == UTC_ZONE_CHINA)		param.Format(_T("/s \"China Standard Time\""));		// 중국 UTC+08:00
	else if (timeZone == UTC_ZONE_KOREA)		param.Format(_T("/s \"Korea Standard Time\""));		// 서울 UTC+09:00

	SHELLEXECUTEINFO sel;
	memset(&sel, 0, sizeof(sel));
	sel.cbSize = sizeof(sel);
	sel.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
	sel.lpFile = exeFile;
	sel.lpParameters = param;
	sel.hwnd = NULL;
	sel.lpVerb = _T("open");
	sel.nShow = SW_HIDE;
	ShellExecuteEx(&sel);

	DWORD dwResult = ::WaitForSingleObject(sel.hProcess, 2000);
	while (1)
	{
		if (dwResult == WAIT_OBJECT_0)
		{
			break;
		}
		else
		{
			// 무한 루프 되는 현상 해결 //
			DWORD dwExitCode;
			if (NULL != sel.hProcess)
			{
				GetExitCodeProcess(sel.hProcess, &dwExitCode);	 // 프로세스 나가기 코드 얻어오기
				TerminateProcess(sel.hProcess, dwExitCode);		  // 프로세스 연결 끊기
				WaitForSingleObject(sel.hProcess, 1000);		 // 종료 될때까지 대기
				CloseHandle(sel.hProcess);						  // 프로세스 핸들 닫기
				return;
			}

			break;
		}
	}
	return;
}

void CCimNetCommApi::SetLocalTimeData (CString strTime)
{

	SYSTEMTIME HostTime;

	TCHAR rdata[250];

	// Host 시간과 동기화
	GetLocalTime(&HostTime);

	rdata[0]=strTime.GetAt(0);
	rdata[1]=strTime.GetAt(1);
	rdata[2]=strTime.GetAt(2);
	rdata[3]=strTime.GetAt(3);
	rdata[4]=NULL;
	HostTime.wYear = _ttoi(rdata);

	rdata[0]=strTime.GetAt(4);
	rdata[1]=strTime.GetAt(5);
	rdata[2]=NULL;
	HostTime.wMonth = _ttoi(rdata);

	rdata[0]=strTime.GetAt(6);
	rdata[1]=strTime.GetAt(7);
	rdata[2]=NULL;
	HostTime.wDay = _ttoi(rdata);

	rdata[0]=strTime.GetAt(8);
	rdata[1]=strTime.GetAt(9);
	rdata[2]=NULL;
	HostTime.wHour = _ttoi(rdata);

	rdata[0]=strTime.GetAt(10);
	rdata[1]=strTime.GetAt(11);
	rdata[2]=NULL;
	HostTime.wMinute = _ttoi(rdata);

	rdata[0]=strTime.GetAt(12);
	rdata[1]=strTime.GetAt(13);
	rdata[2]=NULL;
	HostTime.wSecond = _ttoi(rdata);

	HostTime.wMilliseconds = 0;

	SetLocalTime(&HostTime);
}

void CCimNetCommApi::SetMachineName (CString strBuff)
{
	CString sLog;

	sLog.Format(_T("<MES> Station No Set : %s"), strBuff);
	m_pApp->Gf_writeMLog(sLog);
	m_strMachineName.Format(_T("%s"), strBuff);
}

CString CCimNetCommApi::GetMachineName()
{
	return m_strMachineName;
}

void CCimNetCommApi::SetUserId (CString strBuff)
{
	m_strUserID.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetAgingChangeTime (CString strBuff)
{
	m_strAgingChangeTime.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetAptrAgingTime (CString strBuff)
{
	m_strAptrAgingTime.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetComment (CString strBuff)
{
	strBuff.Replace(_T("\r"), _T(""));
	strBuff.Replace(_T("\n"), _T(""));
	if (strBuff.GetLength() > 200)
	{
		m_strComment.Format(_T("%s"), strBuff.Left(200));
	}
	else
	{
		m_strComment.Format(_T("%s"), strBuff);
	}
}
CString CCimNetCommApi::GetComment()
{
	return m_strComment;
}
void CCimNetCommApi::SetRemark (CString strBuff)
{
	m_strRemark.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetNGPortOut (CString strBuff)
{
	m_strNGPortOut.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetRwkCode(CString strBuff)
{
	m_strRwkCode.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetSSFlag(CString strBuff)
{
	m_strSSFlag.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetFromOper(CString strBuff)
{
	m_strFrom_Oper.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetFLDRFileName(CString strBuff)
{
	m_strFLDRFileName.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetPanelID (CString strBuff)
{
	m_strPanelID.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetBLID (CString strBuff)
{
	m_strBLID.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetSerialNumber (CString strBuff)
{
	m_strSerialNumber.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetModelName (CString strBuff)
{
	m_strModelName.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetPalletID (CString strBuff)
{
	m_strPalletID.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetPF (CString strBuff)
{
	m_strPF.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetDefectPattern (CString strBuff)
{
	m_strDefectPattern.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetPvcomAdjustValue(CString strBuff)
{
	m_strPvcomAdjustValue.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetPvcomAdjustDropValue(CString strBuff)
{
	m_strPvcomAdjustDropValue.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetPatternInfo(CString strBuff)
{
	m_strPatternInfo.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetEdidStatus(CString strBuff)
{
	m_strEdidStatus.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetOverHaulFlag(CString strBuff)
{
	m_strOverHaulFlag.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetBaExiFlag(CString strBuff)
{
	m_strBaExiFlag.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetBuyerSerialNo(CString strBuff)
{
	m_strBuyerSerialNo.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetVthValue(CString strBuff)
{
	m_strVthValue.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetBDInfo(CString strBuff)
{
	m_strBDInfo.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetWDRInfo(CString strBuff)
{
	m_strWDRInfo.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetWDREnd(CString strBuff)
{
	m_strWDREnd.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetAPDInfo(CString strBuff)
{
	while (1)
	{
		if (strBuff.Right(1) == _T(","))
		{
			strBuff.Delete(strBuff.GetLength() - 1, 1);
			continue;
		}
		break;
	}

	m_strAPDInfo.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetDefectCommentCode(CString strBuff)
{
	m_strDefectComCode.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetFullYN(CString strBuff)
{
	m_strFullYN.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetToOper(CString strBuff)
{
	m_strToOper.Format(_T("%s"), strBuff.Right(4));

}

CString CCimNetCommApi::GetToOper()
{
	return m_strToOper;
}

void CCimNetCommApi::SetRepairCD(CString strBuff)
{
	m_strRepairCD.Format(_T("%s"), strBuff);
}
void CCimNetCommApi::SetGibCode(CString strBuff)
{
	m_strGibCode.Format(_T("%s"), strBuff.Left(3));
}
void CCimNetCommApi::SetRespDept(CString strBuff)
{
	m_strRespDept.Format(_T("%s"), strBuff.Left(5));
}

void CCimNetCommApi::SetMaterialInfo(CString strBuff)
{
	m_strMaterialInfo.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetTACT(CString strBuff)
{
	m_strTactTime.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetAutoRespDeptFlag(CString strBuff)
{
	m_strAutoRespDeptFlag.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetAgingLevelInfo(CString strBuff)
{
	m_strAgingLevelInfo.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetPOIRProcessCode(CString strBuff)
{
	m_strPOIRProcessCode.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetActFlag(CString strBuff)
{
	m_strActFlag.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetChannelID (CString strBuff)
{
	m_strChannelID.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetWorkOrder(CString strBuff)
{
	m_strWorkOrder.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetPFactory(CString strBuff)
{
	m_strPFactory.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetCategory(CString strBuff)
{
	m_strCategory.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetWorkerInfo(CString strBuff)
{
	m_strWorkerInfo.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetDurableID(CString strBuff)
{
	m_strDurableID.Format(_T("%s"), strBuff);
}

void CCimNetCommApi::SetSlotNo(CString strBuff)
{
	m_strSlotNo.Format(_T("%s"), strBuff);
}

//////////////////////////////////////////////////////////////////////////////
CString CCimNetCommApi::GetRwkCode()
{
	return m_strRwkCode;
}

CString CCimNetCommApi::GetPF()
{
	return m_strPF;
}

//////////////////////////////////////////////////////////////////////////////s
BOOL CCimNetCommApi::ERCP()
{
	MakeClientTimeString();

	m_strERCP.Format(_T("ERCP ADDR=W4.G1.E3.RMS,W4.G3.W4BMTL0170.UPLOADER EQP=W4TAB1010 MACHINE=W4BMTL0170 UNIT=W4BMTL018001 RCS=H MODE_CODE=N MODEL=6060L-7949B BASEMODEL=LP160WU3-SPB2-KH1 WODR= CATEGORY=EC RECIPE= RECIPEVER= OPER= NW_CD= NW_DESCRIPTION=[] CHANGE_TYPE=B VALIDATIONINFO=[20250324:PROD:3,20250324:SVC:4,20250325:SVC:10,20250326:SVC:12,20250327:SVC:6,20250328:SVC:14,20250328:EC:18,20250329:EC:111,20250330:EC:43,20250331:EC:41,20250401:EC:61,20250401:PROD:49,20250402:EC:93,20250403:EC:23,20250404:EC:26,20250405:EC:72,20250406:EC:15,20250407:EC:34,20250408:EC:11] UNIT_INFO=[W4BMTL0170:[1]:U::3:W4BMTL018001:0:[MODEL_NB#1^018001_ZR8002#808.7364^018001_ZR8004#61.7332^018001_ZR8006#500.0000^018001_ZR29002#1199.2101^018001_ZR29004#2499.6154^018001_ZR29006#432.4710^018001_ZR8020#174.0000^018001_ZR8022#177.5253^018001_ZR8024#310.0000^018001_ZR8026#-4.0269^018001_ZR8028#0^018001_ZR8030#0^018001_ZR8032#150.0000^018001_ZR8034#176.1288^018001_ZR8040#807.0000^018001_ZR8042#0^018001_ZR8044#-5.0100^018001_ZR8046#1439.0000^018001_ZR8048#165.0000^018001_ZR8050#-4.9500^018001_ZR8052#745.2232^018001_ZR8054#336.1449^018001_ZR8056#-5.0200^018001_ZR8060#755.0000^018001_ZR8062#343.3000^018001_ZR8064#-4.9000^018001_ZR8066#28.5000^018001_ZR8068#224.5000^018001_ZR8070#-4.9300^018001_ZR8072#650.0000^018001_ZR8074#224.2044^018001_ZR8076#-4.8600^018001_ZR8078#220.0000^018001_ZR8080#595.0000^018001_ZR8082#0.1000^018001_ZR8084#50.0000^018001_ZR8086#5.0000^018001_ZR8088#560.0000^018001_ZR8092#8.0000^018001_ZR8094#0^018001_ZR8096#44.1874^018001_ZR8098#44.1874^018001_ZR8100#170.0000^018001_ZR8102#-1.7554^018001_ZR8104#172.0000^018001_ZR8106#-3.2600^018001_ZR8108#0^018001_ZR8110#0^018001_ZR8112#305.2000^018001_ZR8114#176.7261^018001_ZR8120#300.0000^018001_ZR8122#1022.0000^018001_ZR8124#-25.0000^018001_ZR8140#1100.0000^018001_ZR8142#100.0000^018001_ZR8144#-5.5000^018001_ZR8146#1478.0000^018001_ZR8148#223.0000^018001_ZR8150#-6.0000^018001_ZR8152#1035.5377^018001_ZR8154#368.0912^018001_ZR8156#-5.4600^018001_ZR8160#1034.9394^018001_ZR8162#369.7500^018001_ZR8164#-5.6500^018001_ZR8166#61.9048^018001_ZR8168#263.2032^018001_ZR8170#174.5200^018001_ZR8178#100.0000^018001_ZR8180#802.0000^018001_ZR8182#-4.1000^018001_ZR8184#167.0000^018001_ZR8186#-3.5000^018001_ZR8188#770.0000^018001_ZR8196#45.9165^018001_ZR8198#45.9165^018001_ZR8300#0^018001_ZR8302#0^018001_ZR8304#0^018001_ZR8320#0^018001_ZR8322#0^018001_ZR8324#0^018001_ZR8326#0^018001_ZR8328#0^018001_ZR8330#0^018001_ZR8332#0^018001_ZR8334#0^018001_ZR8340#0^018001_ZR8342#0^018001_ZR8344#0^018001_ZR8346#0^018001_ZR8348#0^018001_ZR8350#0^018001_ZR8352#0^018001_ZR8354#0^018001_ZR8356#0^018001_ZR8360#0^018001_ZR8362#0^018001_ZR8364#0^018001_ZR8366#0^018001_ZR8368#0^018001_ZR8370#0^018001_ZR8380#0^018001_ZR8382#0^018001_ZR8384#0^018001_ZR8386#0^018001_ZR8388#0^018001_ZR8396#0^018001_ZR8398#0^018001_ZR8400#0^018001_ZR8402#0^018001_ZR8404#0^018001_ZR8406#0^018001_ZR8408#0^018001_ZR8410#0^018001_ZR8412#0^018001_ZR8414#0^018001_ZR8420#0^018001_ZR8422#0^018001_ZR8424#0^018001_ZR8440#0^018001_ZR8442#0^018001_ZR8444#0^018001_ZR8446#0^018001_ZR8448#0^018001_ZR8450#0^018001_ZR8452#0^018001_ZR8454#0^018001_ZR8456#0^018001_ZR8460#0^018001_ZR8462#0^018001_ZR8464#0^018001_ZR8466#0^018001_ZR8468#0^018001_ZR8470#0^018001_ZR8478#0^018001_ZR8480#0^018001_ZR8482#0^018001_ZR8484#0^018001_ZR8486#0^018001_ZR8488#0^018001_ZR8496#0^018001_ZR8498#0^018001_ZR8544#3^018001_ZR8552#0^018001_ZR8554#0^018001_ZR8556#0^018001_ZR8558#0^018001_ZR8566#3^018001_ZR8572#0^018001_ZR8574#0^018001_ZR8576#0^018001_ZR8578#0^018001_ZR8580#1^018001_ZR8582#1^018001_ZR8584#1^018001_ZR8586#1^018001_ZR8590#0^018001_ZR8599#0^018001_ZR8599_1#0^018001_ZR8599_2#0^018001_ZR8599_3#0^018001_ZR8599_4#1^018001_ZR8599_5#0^018001_ZR8600#16 INCH^018001_ZR8700#0.5^018001_ZR8701#0.1^018001_ZR8702#0.3^018001_ZR8703#40^018001_ZR8704#0^018001_ZR8705#0.1^018001_ZR8710#0.5^018001_ZR8711#0.2^018001_ZR8712#0.3^018001_ZR8713#45^018001_ZR8714#0^018001_ZR8715#0.3^018001_ZR8800#250.0000^018001_ZR8802#450.0000^018001_ZR8804#0^018001_ZR8806#0^018001_ZR8808#0^018001_ZR8810#0^018001_ZR8812#0^018001_ZR8814#0^018001_ZR8816#0^018001_ZR8818#0^018001_ZR8820#0^018001_ZR8822#0^018001_ZR8824#0^018001_ZR8826#0^018001_ZR8828#0^018001_ZR8830#0^018001_ZR8832#0^018001_ZR8834#0^018001_ZR8836#0^018001_ZR8838#0^018001_ZR8840#0^018001_ZR8842#0^018001_ZR8844#0^018001_ZR8846#0^018001_ZR8900#116.0000^018001_ZR8902#153.5000^018001_ZR8904#296.0000^018001_ZR8906#331.0000^018001_ZR8908#0^018001_ZR8910#0^018001_ZR8912#0^018001_ZR8914#0^018001_ZR8916#0^018001_ZR8918#0^018001_ZR8920#0^018001_ZR8922#0^018001_ZR8924#0^018001_ZR8926#0^018001_ZR8928#0^018001_ZR8930#0^018001_ZR8932#0^018001_ZR8934#0^018001_ZR8936#0^018001_ZR8938#0^018001_ZR8940#0^018001_ZR8942#0^018001_ZR8944#0^018001_ZR8946#0^018001_ZR29008#100.0000^018001_ZR29010#253.8120^018001_ZR29012#269.2657^018001_ZR29014#270.1898^018001_ZR29016#-11.6995^018001_ZR29018#13.0666^018001_ZR29020#2856.3785^018001_ZR29022#1556.0000^018001_ZR29024#154.0524^018001_ZR29026#782.2222^018001_ZR29030#269.7909^018001_ZR29032#270.5223^018001_ZR29034#58.1715^018001_ZR29036#1.8000^018001_ZR29040#-0.8727^018001_ZR29042#381.0000^018001_ZR29044#680.2026^018001_ZR29048#-2.5901^018001_ZR29050#-4.7586^018001_ZR29052#399.5985^018001_ZR29056#0^018001_ZR29058#0^018001_ZR29064#31.2226^018001_ZR29066#31.2226^018001_ZR29068#41.9265^018001_ZR29070#41.9265^018001_ZR29072#19.5029^018001_ZR29074#18.5750^018001_ZR29076#19.7327^018001_ZR29078#19.4518^018001_ZR29080#1.7277^018001_ZR29082#376.0000^018001_ZR29084#680.5404^018001_ZR29086#-0.7133^018001_ZR29090#-6.4355^018001_ZR29092#400.0000^018001_ZR29096#0^018001_ZR29098#0^018001_ZR29104#34.5866^018001_ZR29106#34^018001_ZR29596_11#0^018001_ZR29596_12#0^018001_ZR29596_13#1^018001_ZR29596_14#0^018001_ZR29120#370.0000^018001_ZR29122#320.0000^018001_ZR29124#290.0000^018001_ZR29130#370.0000^018001_ZR29132#320.0000^018001_ZR29134#290.0000]] REPLY_REQ=Y TO_EQP= MMC_TXN_ID=20250408111744"),

		//m_strEAYT.Format(_T("ERCP ADDR=W4.G1.E3.RMS,W4.G3.W4BMTL0170.UPLOADER EQP=W4TAB1010 MACHINE=W4BMTL0170 UNIT=W4BMTL018001 RCS=H MODE_CODE=N MODEL=6060L-7949B BASEMODEL=LP160WU3-SPB2-KH1 WODR= CATEGORY=EC RECIPE= RECIPEVER= OPER= NW_CD= NW_DESCRIPTION=[] CHANGE_TYPE=B VALIDATIONINFO=[20250324:PROD:3,20250324:SVC:4,20250325:SVC:10,20250326:SVC:12,20250327:SVC:6,20250328:SVC:14,20250328:EC:18,20250329:EC:111,20250330:EC:43,20250331:EC:41,20250401:EC:61,20250401:PROD:49,20250402:EC:93,20250403:EC:23,20250404:EC:26,20250405:EC:72,20250406:EC:15,20250407:EC:34,20250408:EC:11] UNIT_INFO=[W4BMTL0170:[1]:U::3:W4BMTL018001:0:[MODEL_NB#1^018001_ZR8002#808.7364^018001_ZR8004#61.7332^018001_ZR8006#500.0000^018001_ZR29002#1199.2101^018001_ZR29004#2499.6154^018001_ZR29006#432.4710^018001_ZR8020#174.0000^018001_ZR8022#177.5253^018001_ZR8024#310.0000^018001_ZR8026#-4.0269^018001_ZR8028#0^018001_ZR8030#0^018001_ZR8032#150.0000^018001_ZR8034#176.1288^018001_ZR8040#807.0000^018001_ZR8042#0^018001_ZR8044#-5.0100^018001_ZR8046#1439.0000^018001_ZR8048#165.0000^018001_ZR8050#-4.9500^018001_ZR8052#745.2232^018001_ZR8054#336.1449^018001_ZR8056#-5.0200^018001_ZR8060#755.0000^018001_ZR8062#343.3000^018001_ZR8064#-4.9000^018001_ZR8066#28.5000^018001_ZR8068#224.5000^018001_ZR8070#-4.9300^018001_ZR8072#650.0000^018001_ZR8074#224.2044^018001_ZR8076#-4.8600^018001_ZR8078#220.0000^018001_ZR8080#595.0000^018001_ZR8082#0.1000^018001_ZR8084#50.0000^018001_ZR8086#5.0000^018001_ZR8088#560.0000^018001_ZR8092#8.0000^018001_ZR8094#0^018001_ZR8096#44.1874^018001_ZR8098#44.1874^018001_ZR8100#170.0000^018001_ZR8102#-1.7554^018001_ZR8104#172.0000^018001_ZR8106#-3.2600^018001_ZR8108#0^018001_ZR8110#0^018001_ZR8112#305.2000^018001_ZR8114#176.7261^018001_ZR8120#300.0000^018001_ZR8122#1022.0000^018001_ZR8124#-25.0000^018001_ZR8140#1100.0000^018001_ZR8142#100.0000^018001_ZR8144#-5.5000^018001_ZR8146#1478.0000^018001_ZR8148#223.0000^018001_ZR8150#-6.0000^018001_ZR8152#1035.5377^018001_ZR8154#368.0912^018001_ZR8156#-5.4600^018001_ZR8160#1034.9394^018001_ZR8162#369.7500^018001_ZR8164#-5.6500^018001_ZR8166#61.9048^018001_ZR8168#263.2032^018001_ZR8170#174.5200^018001_ZR8178#100.0000^018001_ZR8180#802.0000^018001_ZR8182#-4.1000^018001_ZR8184#167.0000^018001_ZR8186#-3.5000^018001_ZR8188#770.0000^018001_ZR8196#45.9165^018001_ZR8198#45.9165^018001_ZR8300#0^018001_ZR8302#0^018001_ZR8304#0^018001_ZR8320#0^018001_ZR8322#0^018001_ZR8324#0^018001_ZR8326#0^018001_ZR8328#0^018001_ZR8330#0^018001_ZR8332#0^018001_ZR8334#0^018001_ZR8340#0^018001_ZR8342#0^018001_ZR8344#0^018001_ZR8346#0^018001_ZR8348#0^018001_ZR8350#0^018001_ZR8352#0^018001_ZR8354#0^018001_ZR8356#0^018001_ZR8360#0^018001_ZR8362#0^018001_ZR8364#0^018001_ZR8366#0^018001_ZR8368#0^018001_ZR8370#0^018001_ZR8380#0^018001_ZR8382#0^018001_ZR8384#0^018001_ZR8386#0^018001_ZR8388#0^018001_ZR8396#0^018001_ZR8398#0^018001_ZR8400#0^018001_ZR8402#0^018001_ZR8404#0^018001_ZR8406#0^018001_ZR8408#0^018001_ZR8410#0^018001_ZR8412#0^018001_ZR8414#0^018001_ZR8420#0^018001_ZR8422#0^018001_ZR8424#0^018001_ZR8440#0^018001_ZR8442#0^018001_ZR8444#0^018001_ZR8446#0^018001_ZR8448#0^018001_ZR8450#0^018001_ZR8452#0^018001_ZR8454#0^018001_ZR8456#0^018001_ZR8460#0^018001_ZR8462#0^018001_ZR8464#0^018001_ZR8466#0^018001_ZR8468#0^018001_ZR8470#0^018001_ZR8478#0^018001_ZR8480#0^018001_ZR8482#0^018001_ZR8484#0^018001_ZR8486#0^018001_ZR8488#0^018001_ZR8496#0^018001_ZR8498#0^018001_ZR8544#3^018001_ZR8552#0^018001_ZR8554#0^018001_ZR8556#0^018001_ZR8558#0^018001_ZR8566#3^018001_ZR8572#0^018001_ZR8574#0^018001_ZR8576#0^018001_ZR8578#0^018001_ZR8580#1^018001_ZR8582#1^018001_ZR8584#1^018001_ZR8586#1^018001_ZR8590#0^018001_ZR8599#0^018001_ZR8599_1#0^018001_ZR8599_2#0^018001_ZR8599_3#0^018001_ZR8599_4#1^018001_ZR8599_5#0^018001_ZR8600#16 INCH^018001_ZR8700#0.5^018001_ZR8701#0.1^018001_ZR8702#0.3^018001_ZR8703#40^018001_ZR8704#0^018001_ZR8705#0.1^018001_ZR8710#0.5^018001_ZR8711#0.2^018001_ZR8712#0.3^018001_ZR8713#45^018001_ZR8714#0^018001_ZR8715#0.3^018001_ZR8800#250.0000^018001_ZR8802#450.0000^018001_ZR8804#0^018001_ZR8806#0^018001_ZR8808#0^018001_ZR8810#0^018001_ZR8812#0^018001_ZR8814#0^018001_ZR8816#0^018001_ZR8818#0^018001_ZR8820#0^018001_ZR8822#0^018001_ZR8824#0^018001_ZR8826#0^018001_ZR8828#0^018001_ZR8830#0^018001_ZR8832#0^018001_ZR8834#0^018001_ZR8836#0^018001_ZR8838#0^018001_ZR8840#0^018001_ZR8842#0^018001_ZR8844#0^018001_ZR8846#0^018001_ZR8900#116.0000^018001_ZR8902#153.5000^018001_ZR8904#296.0000^018001_ZR8906#331.0000^018001_ZR8908#0^018001_ZR8910#0^018001_ZR8912#0^018001_ZR8914#0^018001_ZR8916#0^018001_ZR8918#0^018001_ZR8920#0^018001_ZR8922#0^018001_ZR8924#0^018001_ZR8926#0^018001_ZR8928#0^018001_ZR8930#0^018001_ZR8932#0^018001_ZR8934#0^018001_ZR8936#0^018001_ZR8938#0^018001_ZR8940#0^018001_ZR8942#0^018001_ZR8944#0^018001_ZR8946#0^018001_ZR29008#100.0000^018001_ZR29010#253.8120^018001_ZR29012#269.2657^018001_ZR29014#270.1898^018001_ZR29016#-11.6995^018001_ZR29018#13.0666^018001_ZR29020#2856.3785^018001_ZR29022#1556.0000^018001_ZR29024#154.0524^018001_ZR29026#782.2222^018001_ZR29030#269.7909^018001_ZR29032#270.5223^018001_ZR29034#58.1715^018001_ZR29036#1.8000^018001_ZR29040#-0.8727^018001_ZR29042#381.0000^018001_ZR29044#680.2026^018001_ZR29048#-2.5901^018001_ZR29050#-4.7586^018001_ZR29052#399.5985^018001_ZR29056#0^018001_ZR29058#0^018001_ZR29064#31.2226^018001_ZR29066#31.2226^018001_ZR29068#41.9265^018001_ZR29070#41.9265^018001_ZR29072#19.5029^018001_ZR29074#18.5750^018001_ZR29076#19.7327^018001_ZR29078#19.4518^018001_ZR29080#1.7277^018001_ZR29082#376.0000^018001_ZR29084#680.5404^018001_ZR29086#-0.7133^018001_ZR29090#-6.4355^018001_ZR29092#400.0000^018001_ZR29096#0^018001_ZR29098#0^018001_ZR29104#34.5866^018001_ZR29106#34^018001_ZR29596_11#0^018001_ZR29596_12#0^018001_ZR29596_13#1^018001_ZR29596_14#0^018001_ZR29120#370.0000^018001_ZR29122#320.0000^018001_ZR29124#290.0000^018001_ZR29130#370.0000^018001_ZR29132#320.0000^018001_ZR29134#290.0000]] REPLY_REQ=Y TO_EQP= MMC_TXN_ID=20250408111744"),

		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strLocalIP,
		m_strServicePort,
		m_strClientDate
	);

	int nRetCode = MessageSend(ECS_MODE_EAYT);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal


}
//////////////////////////////////////////////////////////////////////////////
BOOL CCimNetCommApi::EAYT ()
{
	MakeClientTimeString ();

	m_strEAYT.Format (_T ("EAYT ADDR=%s,%s EQP=%s NET_IP=%s NET_PORT=%s MODE=AUTO CLIENT_DATE=%s"),
	//m_strEAYT.Format(_T("EAYT ADDR=M2.G3.EQP.MOD.172_23_23_23,M2.G3.EQP.MOD.172_23_23_23 EQP=4TPOL110 NET_IP=172.19.141.123 NET_PORT=9000 MODE=AUTO CLIENT_DATE = 20070910010101 "),
	//m_strEAYT.Format(_T("EAYT ADDR=M2.G1.EIFsvr.M2T,M2.G1.EES.M2EQP0102.MES EQP=M2TCVD07 NAME=M2.G1.EES.M2EQP0102.MES REPLY_REQ=Y TO_EQP= MMC_TXN_ID=03312895#0001"),

	//m_strEAYT.Format(_T("ERCP ADDR=W4.G1.E3.RMS,W4.G3.W4BMTL0170.UPLOADER EQP=W4TAB1010 MACHINE=W4BMTL0170 UNIT=W4BMTL018001 RCS=H MODE_CODE=N MODEL=6060L-7949B BASEMODEL=LP160WU3-SPB2-KH1 WODR= CATEGORY=EC RECIPE= RECIPEVER= OPER= NW_CD= NW_DESCRIPTION=[] CHANGE_TYPE=B VALIDATIONINFO=[20250324:PROD:3,20250324:SVC:4,20250325:SVC:10,20250326:SVC:12,20250327:SVC:6,20250328:SVC:14,20250328:EC:18,20250329:EC:111,20250330:EC:43,20250331:EC:41,20250401:EC:61,20250401:PROD:49,20250402:EC:93,20250403:EC:23,20250404:EC:26,20250405:EC:72,20250406:EC:15,20250407:EC:34,20250408:EC:11] UNIT_INFO=[W4BMTL0170:[1]:U::3:W4BMTL018001:0:[MODEL_NB#1^018001_ZR8002#808.7364^018001_ZR8004#61.7332^018001_ZR8006#500.0000^018001_ZR29002#1199.2101^018001_ZR29004#2499.6154^018001_ZR29006#432.4710^018001_ZR8020#174.0000^018001_ZR8022#177.5253^018001_ZR8024#310.0000^018001_ZR8026#-4.0269^018001_ZR8028#0^018001_ZR8030#0^018001_ZR8032#150.0000^018001_ZR8034#176.1288^018001_ZR8040#807.0000^018001_ZR8042#0^018001_ZR8044#-5.0100^018001_ZR8046#1439.0000^018001_ZR8048#165.0000^018001_ZR8050#-4.9500^018001_ZR8052#745.2232^018001_ZR8054#336.1449^018001_ZR8056#-5.0200^018001_ZR8060#755.0000^018001_ZR8062#343.3000^018001_ZR8064#-4.9000^018001_ZR8066#28.5000^018001_ZR8068#224.5000^018001_ZR8070#-4.9300^018001_ZR8072#650.0000^018001_ZR8074#224.2044^018001_ZR8076#-4.8600^018001_ZR8078#220.0000^018001_ZR8080#595.0000^018001_ZR8082#0.1000^018001_ZR8084#50.0000^018001_ZR8086#5.0000^018001_ZR8088#560.0000^018001_ZR8092#8.0000^018001_ZR8094#0^018001_ZR8096#44.1874^018001_ZR8098#44.1874^018001_ZR8100#170.0000^018001_ZR8102#-1.7554^018001_ZR8104#172.0000^018001_ZR8106#-3.2600^018001_ZR8108#0^018001_ZR8110#0^018001_ZR8112#305.2000^018001_ZR8114#176.7261^018001_ZR8120#300.0000^018001_ZR8122#1022.0000^018001_ZR8124#-25.0000^018001_ZR8140#1100.0000^018001_ZR8142#100.0000^018001_ZR8144#-5.5000^018001_ZR8146#1478.0000^018001_ZR8148#223.0000^018001_ZR8150#-6.0000^018001_ZR8152#1035.5377^018001_ZR8154#368.0912^018001_ZR8156#-5.4600^018001_ZR8160#1034.9394^018001_ZR8162#369.7500^018001_ZR8164#-5.6500^018001_ZR8166#61.9048^018001_ZR8168#263.2032^018001_ZR8170#174.5200^018001_ZR8178#100.0000^018001_ZR8180#802.0000^018001_ZR8182#-4.1000^018001_ZR8184#167.0000^018001_ZR8186#-3.5000^018001_ZR8188#770.0000^018001_ZR8196#45.9165^018001_ZR8198#45.9165^018001_ZR8300#0^018001_ZR8302#0^018001_ZR8304#0^018001_ZR8320#0^018001_ZR8322#0^018001_ZR8324#0^018001_ZR8326#0^018001_ZR8328#0^018001_ZR8330#0^018001_ZR8332#0^018001_ZR8334#0^018001_ZR8340#0^018001_ZR8342#0^018001_ZR8344#0^018001_ZR8346#0^018001_ZR8348#0^018001_ZR8350#0^018001_ZR8352#0^018001_ZR8354#0^018001_ZR8356#0^018001_ZR8360#0^018001_ZR8362#0^018001_ZR8364#0^018001_ZR8366#0^018001_ZR8368#0^018001_ZR8370#0^018001_ZR8380#0^018001_ZR8382#0^018001_ZR8384#0^018001_ZR8386#0^018001_ZR8388#0^018001_ZR8396#0^018001_ZR8398#0^018001_ZR8400#0^018001_ZR8402#0^018001_ZR8404#0^018001_ZR8406#0^018001_ZR8408#0^018001_ZR8410#0^018001_ZR8412#0^018001_ZR8414#0^018001_ZR8420#0^018001_ZR8422#0^018001_ZR8424#0^018001_ZR8440#0^018001_ZR8442#0^018001_ZR8444#0^018001_ZR8446#0^018001_ZR8448#0^018001_ZR8450#0^018001_ZR8452#0^018001_ZR8454#0^018001_ZR8456#0^018001_ZR8460#0^018001_ZR8462#0^018001_ZR8464#0^018001_ZR8466#0^018001_ZR8468#0^018001_ZR8470#0^018001_ZR8478#0^018001_ZR8480#0^018001_ZR8482#0^018001_ZR8484#0^018001_ZR8486#0^018001_ZR8488#0^018001_ZR8496#0^018001_ZR8498#0^018001_ZR8544#3^018001_ZR8552#0^018001_ZR8554#0^018001_ZR8556#0^018001_ZR8558#0^018001_ZR8566#3^018001_ZR8572#0^018001_ZR8574#0^018001_ZR8576#0^018001_ZR8578#0^018001_ZR8580#1^018001_ZR8582#1^018001_ZR8584#1^018001_ZR8586#1^018001_ZR8590#0^018001_ZR8599#0^018001_ZR8599_1#0^018001_ZR8599_2#0^018001_ZR8599_3#0^018001_ZR8599_4#1^018001_ZR8599_5#0^018001_ZR8600#16 INCH^018001_ZR8700#0.5^018001_ZR8701#0.1^018001_ZR8702#0.3^018001_ZR8703#40^018001_ZR8704#0^018001_ZR8705#0.1^018001_ZR8710#0.5^018001_ZR8711#0.2^018001_ZR8712#0.3^018001_ZR8713#45^018001_ZR8714#0^018001_ZR8715#0.3^018001_ZR8800#250.0000^018001_ZR8802#450.0000^018001_ZR8804#0^018001_ZR8806#0^018001_ZR8808#0^018001_ZR8810#0^018001_ZR8812#0^018001_ZR8814#0^018001_ZR8816#0^018001_ZR8818#0^018001_ZR8820#0^018001_ZR8822#0^018001_ZR8824#0^018001_ZR8826#0^018001_ZR8828#0^018001_ZR8830#0^018001_ZR8832#0^018001_ZR8834#0^018001_ZR8836#0^018001_ZR8838#0^018001_ZR8840#0^018001_ZR8842#0^018001_ZR8844#0^018001_ZR8846#0^018001_ZR8900#116.0000^018001_ZR8902#153.5000^018001_ZR8904#296.0000^018001_ZR8906#331.0000^018001_ZR8908#0^018001_ZR8910#0^018001_ZR8912#0^018001_ZR8914#0^018001_ZR8916#0^018001_ZR8918#0^018001_ZR8920#0^018001_ZR8922#0^018001_ZR8924#0^018001_ZR8926#0^018001_ZR8928#0^018001_ZR8930#0^018001_ZR8932#0^018001_ZR8934#0^018001_ZR8936#0^018001_ZR8938#0^018001_ZR8940#0^018001_ZR8942#0^018001_ZR8944#0^018001_ZR8946#0^018001_ZR29008#100.0000^018001_ZR29010#253.8120^018001_ZR29012#269.2657^018001_ZR29014#270.1898^018001_ZR29016#-11.6995^018001_ZR29018#13.0666^018001_ZR29020#2856.3785^018001_ZR29022#1556.0000^018001_ZR29024#154.0524^018001_ZR29026#782.2222^018001_ZR29030#269.7909^018001_ZR29032#270.5223^018001_ZR29034#58.1715^018001_ZR29036#1.8000^018001_ZR29040#-0.8727^018001_ZR29042#381.0000^018001_ZR29044#680.2026^018001_ZR29048#-2.5901^018001_ZR29050#-4.7586^018001_ZR29052#399.5985^018001_ZR29056#0^018001_ZR29058#0^018001_ZR29064#31.2226^018001_ZR29066#31.2226^018001_ZR29068#41.9265^018001_ZR29070#41.9265^018001_ZR29072#19.5029^018001_ZR29074#18.5750^018001_ZR29076#19.7327^018001_ZR29078#19.4518^018001_ZR29080#1.7277^018001_ZR29082#376.0000^018001_ZR29084#680.5404^018001_ZR29086#-0.7133^018001_ZR29090#-6.4355^018001_ZR29092#400.0000^018001_ZR29096#0^018001_ZR29098#0^018001_ZR29104#34.5866^018001_ZR29106#34^018001_ZR29596_11#0^018001_ZR29596_12#0^018001_ZR29596_13#1^018001_ZR29596_14#0^018001_ZR29120#370.0000^018001_ZR29122#320.0000^018001_ZR29124#290.0000^018001_ZR29130#370.0000^018001_ZR29132#320.0000^018001_ZR29134#290.0000]] REPLY_REQ=Y TO_EQP= MMC_TXN_ID=20250408111744"),

	//m_strEAYT.Format(_T("EAYT ADDR=W4.G1.E3.RMS,W4.G3.W4BMTL0170.UPLOADER EQP=W4TAB1010 NET_IP=239.28.8.54 NET_PORT=28854 MODE=AUTO CLIENT_DATE=20250316095556"),
	//m_strEAYT.Format(_T("EAYT ADDR=W4.G1.E3.RMS,W4.G3.W4BMTL0170.UPLOADER EQP=M2TCVD07 NAME=M2.G1.EES.M2EQP0102.MES REPLY_REQ=Y TO_EQP= MMC_TXN_ID=03312895#0001"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strLocalIP,
		m_strServicePort,
		m_strClientDate
		);

	int nRetCode = MessageSend (ECS_MODE_EAYT);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal


}

BOOL CCimNetCommApi::UCHK ()
{
	// -- user id...
	MakeClientTimeString ();
	m_strClientOldDate = m_strClientDate;
	m_strUCHK.Format (_T ("UCHK ADDR=%s,%s EQP=%s USER_ID=%s MODE=AUTO CLIENT_DATE=%s"),
	//m_strUCHK.Format(_T("UCHK ADDR=W4.G3.EQP.MOD.10.122.123.36,W4.G3.EQP.MOD.10.122.123.36 EQP=W4AMAL02HV01 USER_ID=02 MODE=AUTO CLIENT_DATE=20250318155459"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strUserID,
		m_strClientDate
		);

	int nRetCode = MessageSend (ECS_MODE_UCHK);
	if (0 != nRetCode)
	{
		return nRetCode;
	}

	//-------------------------------------------
	// Receive Message 처리.
	//-------------------------------------------
	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"), 0);
	if (strMsg.Compare(_T("0")))	// 
	{	
		// if return code is not zero...
		return 3;	// return code is not zero...
	}

	// return code is zero...
	GetFieldData(&strMsg, _T("HOST_DATE"), 0);
	if (strMsg.GetLength() != 14)
	{
		return 4;
	}
	m_strClientNewDate.Format(strMsg);

	if (m_bIsGmesLocalTestMode == FALSE)
	{
		// 시스템 시간을 HOST 시간과 동기화 시킨다.
		SetLocalTimeZone(UTC_ZONE_VIETNAM);
			
		SetLocalTimeData(m_strClientNewDate);
	}

	MakeClientTimeString ();

	return 0;
}

BOOL CCimNetCommApi::EDTI ()
{
	MakeClientTimeString ();
	m_strEDTI.Format (_T ("EDTI ADDR=%s,%s EQP=%s OLD_DATE=%s NEW_DATE=%s USER_ID=%s MODE=AUTO CLIENT_DATE=%s"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strClientOldDate,
		m_strClientNewDate,
		m_strUserID,
		m_strClientDate
		);
	return MessageSend (ECS_MODE_EDTI);
}

BOOL CCimNetCommApi::FLDR ()
{
	MakeClientTimeString ();

	m_strFLDR.Format (_T ("FLDR ADDR=%s,%s EQP=%s FILE_NAME=[%s] FILE_TYPE=DEFECT USER_ID=%s MODE=AUTO DOWNLOAD_TIME=%s CLIENT_DATE=%s COMMENT=[%s]"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strFLDRFileName,
		m_strUserID,
		m_strClientOldDate,
		m_strClientDate,
		_T("")	// Comment
		);
	return MessageSend (ECS_MODE_FLDR);
}

BOOL CCimNetCommApi::PCHK (int ipa_mode, int ipa_value)
{
	MakeClientTimeString ();

	m_strPCHK.Format (_T ("PCHK ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s BLID=[%s] PPALLET=%s SKD_BOX_ID= APD_REQ=Y USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[%s]"),
	//m_strPCHK.Format(_T("PCHK ADDR=%s,%s EQP=%s PID=P87S51005912AB SERIAL_NO=%s BLID=[%s] PCBID= PPALLET=%s SKD_BOX_ID= IPA_MODE=Y USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[%s]"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,
		m_strSerialNumber,
		m_strBLID,
		m_strPalletID,
		m_strUserID,
		m_strClientDate,
		_T("")	// Comment
	);

	int nRetCode = MessageSend (ECS_MODE_PCHK);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::PCHK_B(CString PID)
{
	MakeClientTimeString();

	m_strPCHK.Format(_T("PCHK ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s BLID=[%s] PPALLET=%s SKD_BOX_ID= APD_REQ=Y USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[%s]"),
		//m_strPCHK.Format(_T("PCHK ADDR=%s,%s EQP=%s PID=P87S51005912AB SERIAL_NO=%s BLID=[%s] PCBID= PPALLET=%s SKD_BOX_ID= IPA_MODE=Y USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[%s]"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		PID,
		m_strSerialNumber,
		m_strBLID,
		m_strPalletID,
		m_strUserID,
		m_strClientDate,
		_T("")	// Comment
	);

	int nRetCode = MessageSend(ECS_MODE_PCHK);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}


BOOL CCimNetCommApi::EICR (int stationMode)
{
//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString ();

	if (m_strEdidStatus.GetLength() <= 0)
		m_strEdidStatus.Format (_T("N"));
	if (m_strPF.GetLength() <= 0)
	{
		if (m_strRwkCode.GetLength() <= 0)
			m_strPF.Format(_T("P"));	// ok
		else
			m_strPF.Format(_T("F"));	// ng
	}

	m_strEICR.Format(_T("EICR ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s BLID=[%s] PF=%s RWK_CD=%s PPALLET=%s EXPECTED_RWK=%s PATTERN_INFO=[%s] DEFECT_PATTERN=%s EDID=%s PVCOM_ADJUST_VALUE=%s PVCOM_ADJUST_DROP_VALUE=%s OVERHAUL_FLAG=%s DEFECT_COMMENT_CODE=%s MODE=AUTO CLIENT_DATE=%s USER_ID=%s COMMENT=[%s] BA_EXI_FLAG=%s MATERIAL_INFO=[] BUYER_SERIAL_NO=%s CGID= VTH_VALUE=%s BD_INFO=[%s] WDR_INFO=[%s] WDR_END=%s REMARK=[%s] NG_PORT_OUT=%s TACT=%s %s"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,
		m_strSerialNumber,
		m_strBLID,
		m_strPF,
		m_strRwkCode,
		m_strPalletID,
		m_strExpectedRwk,
		m_strPatternInfo,
		m_strDefectPattern,
		m_strEdidStatus,
		m_strPvcomAdjustValue,
		m_strPvcomAdjustDropValue,
		m_strOverHaulFlag,
		m_strDefectComCode,//DefectCommentcomde 추가 CNZ 20150624
		m_strClientDate,
		m_strUserID,
		m_strComment,//_T(""),	// Comment
		m_strBaExiFlag,
		m_strBuyerSerialNo,
		m_strVthValue,
		m_strBDInfo,
		m_strWDRInfo,
		m_strWDREnd,
		m_strRemark,//REMARK
		m_strNGPortOut,
		m_strTactTime,
		m_strAgingLevelInfo		// 2022-01-06 PDH. ★중요★ m_strAgingLevelInfo 변수는 Field 이름도 같이 포함되어 있는 변수이다.
	);

	int nRetCode = MessageSend (ECS_MODE_EICR);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::RPLT(int Station)
{
	CString m_strRepair_Type_CD;

	//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString();
	m_strRPLT.Format(_T ("RPLT ADDR=%s,%s EQP=%s PID=%s SS_FLAG=%s MATERIAL_INFO=[:%s:%s::::%s:] REPAIR_CD=[%s] GIB_CD=%s RWK_TIMEKEY=%s RESP_DEPT=%s USER_ID=%s RECYCLE_INFO=[%s:%s] SERIAL_NO=%s MODE=%s CLIENT_DATE=%s COMMENT=[%s] REPAIR_TYPE_CD=%s TO_OPER=%s REPAIR_INSP_FLAG=%s TACT=%s"),
		m_strLocalSubjectMesF,		// ADDR
		m_strLocalSubjectMesF,		// ADDR
		m_strMachineName,			// EQP
		m_strPanelID,				// PID
		_T("N"),					// SS_FLAG
		_T("4BLTZ"),				// MATERIAL_TYPE : MATERIAL_INFO
		_T("1"),					// MATERIAL_QTY : MATERIAL_INFO
		_T("N"),					// CANGE_FLAG : MATERIAL_INFO
		m_strRepairCD,				// REPAIR_CD
		m_strGibCode,				// GIB_CD
		m_strClientDate,			// RWK_TIMEKEY
		m_strRespDept,				// RESP_DEPT (귀책부서)
		m_strUserID,				// USER_ID
		_T("BLU"),					// MAT_POSITION_NAME : RECYCLE INFO
		_T("N"),					// RECYCLE_FLAG : RECYCLE INFO
		m_strSerialNumber,			// SERIAL_NO
		_T("AUTO"),					// MODE
		m_strClientDate,			// CLIENT_DATE
		(m_strRemark),				// Comment
		m_strRepair_Type_CD,		// REPAIR_TYPE_CD
		m_strToOper,				// TO_OPER
		_T("N"),					// REPAIR_INSP_FLAG
		m_strTactTime
		);


	int nRetCode = MessageSend (ECS_MODE_RPLT);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::RPRQ()
{
	if (m_strSSFlag == _T("Y"))
	{
		m_strRwkCode = _T("A0G-B01-----G5N---------------------------");
		m_strSSFlag.Format(_T("Y SS_LOC=%s"), m_strFrom_Oper);
		m_strToOper = _T("5800");
	}

	MakeClientTimeString();
	m_strRPRQ.Format(_T("RPRQ ADDR=%s,%s EQP=%s PID=%s RWK_CD=%s FROM_OPER=%s TO_OPER=%s SS_FLAG=%s RESP_DEPT=%s USER_ID=%s MODE=AUTO COMMENT=[%s] CLIENT_DATE=%s"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,		//EQP
		m_strPanelID,			//PID
		m_strRwkCode,			//RWK_CODE
		m_strFrom_Oper,			//FROM_OPER
		m_strToOper,			//TO_OPER
		m_strSSFlag,			//SS_FLAG
		m_strRespDept,			//REST_DEPT
		m_strUserID,			//USER_ID
		(m_strRemark),			//COMMENT
		m_strClientDate
	);

	int nRetCode = MessageSend(ECS_MODE_RPRQ);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::SCRA(int station)
{
	CString m_strMGIBMachineNameCopy = m_strMachineName;
	
	MakeClientTimeString();
	m_strSCRA.Format(_T("SCRA ADDR=%s,%s EQP=%s PID=%s REPAIR_CD=%s USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[%s]"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,			//PANEL_ID
		m_strRepairCD,			//REPAIR_CD
		m_strUserID,			//USER_ID
		m_strClientDate,		//CLIENT_DATE
		(m_strRemark)			//COMMENT
	);

	int nRetCode = MessageSend(ECS_MODE_SCRA);
	
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::SCRP(int station)
{
	CString m_strMGIBMachineNameCopy;

	MakeClientTimeString();
	m_strSCRP.Format(_T("SCRP ADDR=%s,%s EQP=%s PID=%s SS_FLAG=%s SS_LOC=%s MATERIAL_INFO=[%s:%s:%s:%s] SCRAP_CD=[%s] RESP_DEPT=%s USER_ID=%s MODE=%s CLIENT_DATE=%s COMMENT=[%s] RECYCLE_INFO=[%s:%s] AUTO_RESP_DEPT_FLAG=%s REPAIR_TYPE_CD=%s"),
		m_strLocalSubjectMesF,		// ADDR
		m_strLocalSubjectMesF,		// ADDR
		m_strMachineName,			// EQP
		m_strPanelID,				// PID
		_T("N"),					// SS_FLAG
		_T(""),						// SS_LOC
		_T(""),						// MATERIAL_ID : MATERIAL_INFO
		_T("4BLTZ"),				// MATERIAL_TYPE : MATERIAL_INFO
		_T("1"),					// MATERIAL_QTY : MATERIAL_INFO
		_T(""),						// MATERIAL_LOC : MATERIAL_INFO
		m_strRepairCD,				// SCRAP_CD
		m_strRespDept,				// RESP_DEPT (귀책부서)
		m_strUserID,				// USER_ID
		_T("AUTO"),					// MODE
		m_strClientDate,			// RWK_TIMEKEY
		(m_strRemark),				// Comment
		_T("4PCBC"),				// MAT_POSITION_NAME : RECYCLE INFO
		_T("Y"),					// RECYCLE_FLAG : RECYCLE INFO
		m_strAutoRespDeptFlag,		// AUTO_RESP_DEPT_FLAG
		m_strRepairTypeCD	 		// REPAIR_TYPE_CD
	);

	int nRetCode = MessageSend(ECS_MODE_SCRP);
	
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}
BOOL CCimNetCommApi::MSET ()
{
	MakeClientTimeString ();

	m_strMSET.Format (_T ("MSET ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s BLID=%s MATERIAL_INFO=[%s] USER_ID=%s MODE=AUTO CLIENT_DATE=%s"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,
		m_strSerialNumber,
		m_strBLID,
		m_strMaterialInfo,
		m_strUserID,
		m_strClientDate
		);

	return MessageSend (ECS_MODE_MSET);
}

BOOL CCimNetCommApi::AGCM ()
{
	MakeClientTimeString ();

	m_strAGCM.Format (_T ("AGCM ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s BLID=[] CHANNEL_ID=%s BOARD_ID=LH128FT2GIENGAGINGBOARDFLAT1 USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[]"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,		
		m_strPanelID,
		m_strSerialNumber,
		m_strChannelID,
		m_strUserID,
		m_strClientDate
		);

	int nRetCode = MessageSend (ECS_MODE_AGCM);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::AGN_IN ()
{
	MakeClientTimeString ();

	m_strAGN_IN.Format (_T ("AGN_IN ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s PPALLET=%s FULL_YN=Y USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[]"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,		
		m_strPanelID,
		m_strSerialNumber,
		m_strChannelID,
		m_strUserID,
		m_strClientDate
		);

	LPINSPWORKINFO lpInspWorkInfo = m_pApp->GetInspWorkInfo();
	


	int nRetCode = MessageSend (ECS_MODE_AGN_IN);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	lpInspWorkInfo->m_nAgnInStartTime[lpInspWorkInfo->m_AgnInStartRack][lpInspWorkInfo->m_AgnInStartLayer][lpInspWorkInfo->m_AgnInStartChannel] = m_strClientDate;
	lpInspWorkInfo->m_nAgnInStartPid[lpInspWorkInfo->m_AgnInStartRack][lpInspWorkInfo->m_AgnInStartLayer][lpInspWorkInfo->m_AgnInStartChannel] = m_strPanelID;

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::AGN_OUT()
{
	MakeClientTimeString ();

	/*m_strAGN_OUT.Format (_T ("AGN_OUT ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s PPALLET=%s FULL_YN=Y USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[]"),*/
	m_strAGN_OUT.Format(_T("AGN_OUT ADDR=%s,%s EQP=%s PID=%s PPALLET=%s FULL_YN=Y VTH_VALUE= AGN_OFFRS_CNT= USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[]"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,		
		m_strPanelID,
		m_strChannelID,
		m_strUserID,
		m_strClientDate
		);

	int nRetCode = MessageSend (ECS_MODE_AGN_OUT);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::AGN_INSP ()
{
	m_strAGN_INSP.Format(_T ("AGN_INSP ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s BLID=[%s] PF=%s RWK_CD=%s USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[%s] REMARK=[%s]"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,
		m_strSerialNumber,
		_T(""),				// BLID
		m_strPF,			// PF
		m_strRwkCode,		// RWK_CD ( m_strRwkCode or Reason Code )
		m_strUserID,
		m_strClientDate,
		m_strComment,
		m_strRemark
		);


	int nRetCode = MessageSend (ECS_MODE_AGN_INSP);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::WDCR ()
{
	//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString ();

	m_strWDCR.Format(_T ("WDCR ADDR=%s,%s EQP=%s MODE=AUTO PID=%s SERIAL_NO=%s BLID= WDR_INFO=[%s] USER_ID=%s CLIENT_DATE=%s"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,
		m_strSerialNumber,
		m_strWDRInfo,
		m_strUserID,
		m_strClientDate
	);


	int nRetCode = MessageSend (ECS_MODE_WDCR);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::LPIR()
{
	MakeClientTimeString();
	if(m_strRemark.GetLength() == 0)
	{
		m_strLPIR.Format(_T("LPIR ADDR=%s,%s EQP=%s PID=%s ZIG_ID=%s USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[%s]"),
			m_strLocalSubjectMesF,
			m_strLocalSubjectMesF,
			m_strMachineName,
			m_strPanelID,
			m_strPalletID,
			m_strUserID,
			m_strClientDate,
			(m_strComment)
			);
	}
	else
	{
		m_strLPIR.Format(_T("LPIR ADDR=%s,%s EQP=%s PID=%s ZIG_ID=%s USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[%s] REMARK=[%s]"),
			m_strLocalSubjectMesF,
			m_strLocalSubjectMesF,
			m_strMachineName,
			m_strPanelID,
			m_strPalletID,
			m_strUserID,
			m_strClientDate,
			(m_strComment),
			m_strRemark
			);
	}

	int nRetCode = MessageSend (ECS_MODE_LPIR);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::BDCR ()
{
	//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString ();

	m_strBDCR.Format(_T ("BDCR ADDR=%s,%s EQP=%s MODE=AUTO PID=%s SERIAL_NO= BLID= BD_INFO=[%s] USER_ID=%s CLIENT_DATE=%s"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,
		m_strBDInfo,
		m_strUserID,
		m_strClientDate
		);


	int nRetCode = MessageSend (ECS_MODE_BDCR);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::INSP_INFO ()
{
	//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString ();
	m_strINSP_INFO.Format(_T ("INSP_INFO ADDR=%s,%s EQP=%s MODE=AUTO PID=%s USER_ID=%s COMMENT=[%s] CLIENT_DATE=%s"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,
		m_strUserID,
		(m_strComment),
		m_strClientDate
		);


	int nRetCode = MessageSend (ECS_MODE_INSP_INFO);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::PINF ()
{
	//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString ();
	m_strPINF.Format(_T ("PINF ADDR=%s,%s EQP=%s PID=%s CGID= SERIAL_NO= BLID= PCBID= USER_ID=%s COMMENT=[%s] MODE=AUTO CLIENT_DATE=%s"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,
		m_strUserID,
		(m_strComment),
		m_strClientDate
		);


	int nRetCode = MessageSend (ECS_MODE_PINF);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::AGN_CTR ()
{
	//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString ();
	m_strAGN_CTR.Format(_T ("AGN_CTR ADDR=%s,%s EQP=%s PID=%s AGING_CHANGE_TIME=%s USER_ID=%s COMMENT=[%s] MODE=AUTO CLIENT_DATE=%s"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,
		m_strAgingChangeTime,
		m_strUserID,
		(m_strComment),
		m_strClientDate
		);


	int nRetCode = MessageSend (ECS_MODE_AGN_CTR);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::APTR ()
{
	//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString ();
	m_strAPTR.Format(_T ("APTR ADDR=%s,%s EQP=%s PID=%s AGING_TIME=%s USER_ID=%s COMMENT=[%s] MODE=AUTO CLIENT_DATE=%s"),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		m_strPanelID,
		m_strAptrAgingTime,
		m_strUserID,
		(m_strComment),
		m_strClientDate
		);


	int nRetCode = MessageSend (ECS_MODE_APTR);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::POIR()
{
	//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString();
	m_strPOIR.Format(_T("POIR ADDR=%s,%s PID=%s PROCESS_CODE=[%s] COMMENT=[%s] ACT_FLAG=%s REPRESENTATIVE_FACTORY_CODE=G3 MODE=AUTO USER_ID=%s CLIENT_DATE=%s")
	, m_strLocalSubjectMesF
	, m_strLocalSubjectMesF
	, m_strPanelID
	, m_strPOIRProcessCode
	, m_strComment
	, m_strActFlag
	, m_strUserID
	, m_strClientDate);


	int nRetCode = MessageSend(ECS_MODE_POIR);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::SSIR()
{
	//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString();
	m_strSSIR.Format(_T("SSIR ADDR=%s,%s PID=%s SERIAL_NO=%s RWK_CD=%s SS_LOC=ASY01 MODE=AUTO USER_ID=%s CLIENT_DATE=%s COMMENT=[%s]")
		, m_strLocalSubjectMesF
		, m_strLocalSubjectMesF
		, m_strPanelID
		, m_strSerialNumber
		, m_strRwkCode
		, m_strUserID
		, m_strClientDate
		, m_strComment);

	int nRetCode = MessageSend(ECS_MODE_SSIR);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::EWOQ()
{
	MakeClientTimeString();
	m_strEWOQ.Format(_T("EWOQ ADDR=%s,%s EQP=%s COMPLETE_YN=N USER_ID=%s MODE=AUTO CLIENT_DATE=%s")
		, m_strLocalSubjectMesF
		, m_strLocalSubjectMesF
		, m_strMachineName
		, m_strUserID
		, m_strClientDate);


	int nRetCode = MessageSend(ECS_MODE_EWOQ);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;
}

BOOL CCimNetCommApi::EWCH()
{
	MakeClientTimeString();
	m_strEWCH.Format(_T("EWCH ADDR=%s,%s EQP=%s COMPLETE_YN=Y WODR=%s MODEL=%s USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[]")
		, m_strLocalSubjectMesF
		, m_strLocalSubjectMesF
		, m_strMachineName
		, m_strWorkOrder
		, m_strModelName
		, m_strUserID
		, m_strClientDate);


	int nRetCode = MessageSend(ECS_MODE_EWCH);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;
}

BOOL CCimNetCommApi::EPIQ()
{
	MakeClientTimeString();
	m_strEPIQ.Format(_T("EPIQ ADDR=%s,%s EQP=%s USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[]")
		, m_strLocalSubjectMesF
		, m_strLocalSubjectMesF
		, m_strMachineName
		, m_strUserID
		, m_strClientDate);


	int nRetCode = MessageSend(ECS_MODE_EPIQ);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;
}

BOOL CCimNetCommApi::EPCR()
{
	MakeClientTimeString();
	m_strEPCR.Format(_T("EPCR ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s WODR=%s MODEL=%s BASIC_MODEL= PFACTORY=%s CATEGORY=%s SPCD= MATERIAL_INFO=[%s] WORKER_INFO=[%s] TACT= CUSHION_MTL_ID=[] PRT_MAKER= PRT_RESOLUTION= PRT_QTY= PRT_MARGIN_H= PRT_MARGIN_V= MODE=AUTO USER_ID=%s CLIENT_DATE=%s COMMENT=[]")
		, m_strLocalSubjectMesF
		, m_strLocalSubjectMesF
		, m_strMachineName
		, m_strPanelID
		, m_strSerialNumber
		, m_strWorkOrder
		, m_strModelName
		, m_strPFactory
		, m_strCategory
		, m_strMaterialInfo
		, m_strWorkerInfo
		, m_strUserID
		, m_strClientDate);


	int nRetCode = MessageSend(ECS_MODE_EPCR);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;
}

BOOL CCimNetCommApi::DSPM()
{
	MakeClientTimeString();
	m_strDSPM.Format(_T("DSPM ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s DURABLE_ID=%s SLOT_NO=%s ACT_FLAG=%s MODE=AUTO USER_ID=%s CLIENT_DATE=%s COMMENT=[]")
		, m_strLocalSubjectMesF
		, m_strLocalSubjectMesF
		, m_strMachineName
		, m_strPanelID
		, m_strSerialNumber
		, m_strDurableID
		, m_strSlotNo
		, m_strActFlag
		, m_strUserID
		, m_strClientDate);


	int nRetCode = MessageSend(ECS_MODE_DSPM);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;
}

BOOL CCimNetCommApi::DMIN()
{
	MakeClientTimeString();
	m_strDMIN.Format(_T("DMIN ADDR=%s,%s EQP=%s DURABLE_ID=%s MODE=AUTO USER_ID=%s CLIENT_DATE=%s COMMENT=[]")
		, m_strLocalSubjectMesF
		, m_strLocalSubjectMesF
		, m_strMachineName
		, m_strDurableID
		, m_strUserID
		, m_strClientDate);


	int nRetCode = MessageSend(ECS_MODE_DMIN);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;
}

BOOL CCimNetCommApi::DMOU()
{
	MakeClientTimeString();
	m_strDMOU.Format(_T("DMOU ADDR=%s,%s EQP=%s DURABLE_ID=%s MODE=AUTO USER_ID=%s CLIENT_DATE=%s COMMENT=[]")
		, m_strLocalSubjectMesF
		, m_strLocalSubjectMesF
		, m_strMachineName
		, m_strDurableID
		, m_strUserID
		, m_strClientDate);


	int nRetCode = MessageSend(ECS_MODE_DMOU);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;
}

BOOL CCimNetCommApi::APDR ()
{
	//	정밀검사 default: N, 정밀검사 사용시 Y
	MakeClientTimeString ();

	m_strAPDR.Format(_T ("APDR ADDR=%s,%s EQP=%s MODEL=%s PID=%s SERIAL_NO=%s APD_INFO=[%s] USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[%s]"),
		m_strLocalSubjectEasF, // ADDR
		m_strLocalSubjectEasF, // ADDR
		m_strMachineName, // EQP
		m_strModelName, // MODEL
		m_strPanelID, // PID
		m_strSerialNumber, // SERIAL_NO
		m_strAPDInfo, // APD_INFO
		m_strUserID, // USER_ID
		m_strClientDate, // CLIENT_DATE
		_T("")	// Comment
		);


	int nRetCode = MessageSend (ECS_MODE_APDR);
	if (nRetCode != RTN_OK)
	{	
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{	
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::UNDO(int rack, int layer, int ch)
{
	MakeClientTimeString();

	LPINSPWORKINFO lpInspWorkInfo = m_pApp->GetInspWorkInfo();

	m_strUNDO.Format(_T("UNDO ADDR=%s,%s EQP=%s PID=%s SERIAL_NO=%s LASTEVENT_TIMEKEY=%s USER_ID=%s MODE=AUTO COMMENT=[] CLIENT_DATE=%s "),
		m_strLocalSubjectMesF,
		m_strLocalSubjectMesF,
		m_strMachineName,
		lpInspWorkInfo->m_nAgnInStartPid[rack][layer][ch],
		m_strSerialNumber,
		lpInspWorkInfo->m_nAgnInStartTime[rack][layer][ch],
		m_strUserID,
		m_strClientDate
	);

	int nRetCode = MessageSend(ECS_MODE_AGN_IN);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}

BOOL CCimNetCommApi::RMSO()
{
	MakeClientTimeString();

	LPINSPWORKINFO lpInspWorkInfo = m_pApp->GetInspWorkInfo();

	m_strRMSO.Format(_T("RMSO ADDR=%s,%s EQP=%s DURABLE_ID=%s MODE=AUTO USER_ID=%s CLIENT_DATE=%s COMMENT=[]")
		, m_strLocalSubjectMesF
		, m_strLocalSubjectMesF
		, m_strMachineName
		, m_strDurableID
		, m_strUserID
		, m_strClientDate);

	/*m_strRMSO.Format(_T("RMSO ADDR=%s,%s EQP=%s DURABLE_ID=%f MODE=AUTO USER_ID=%f CLIENT_DATE=%f COMMENT=[]")
		, m_strLocalSubjectMesF
		, m_strLocalSubjectMesF
		, m_strMachineName
		, lpInspWorkInfo->m_fTempReadValST590_2_SET[0]
		, lpInspWorkInfo->m_fTempReadValST590_3_SET[0]
		, lpInspWorkInfo->m_fTempReadValST590_4_SET[0]);*/


	int nRetCode = MessageSend(ECS_MODE_RMSO);
	if (nRetCode != RTN_OK)
	{
		return nRetCode;
	}

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;	// return code is not zero...
	}

	return RTN_OK;	// normal
}
