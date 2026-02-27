#include "pch.h"
#include "HseAging.h"

#include "atlbase.h"
#include "atlcom.h"
#include <string>

#include "CIMNetCommApp.h"

#include <vector>
#include <algorithm>


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
// ModelList.ini에서 키(001,002...) 목록 읽기 함수3
static std::vector<CString> ReadModelListKeys(const CString& modelListIniPath)
{
	std::vector<CString> keys;

	// GetPrivateProfileSection은 "key=value\0key=value\0\0" 형태로 돌려줌
	// 넉넉히 버퍼 확보 (모델 많으면 키워야 함)
	const DWORD BUF_SIZE = 64 * 1024;
	std::vector<TCHAR> buf(BUF_SIZE, 0);

	DWORD n = ::GetPrivateProfileSection(_T("ModelList"), buf.data(), (DWORD)buf.size(), modelListIniPath);
	if (n == 0)
		return keys; // 섹션 없거나 비어있음

	// buf를 순회하며 한 줄씩 파싱
	const TCHAR* p = buf.data();
	while (*p)
	{
		CString line = p;        // "001=LP140WU3"
		int eq = line.Find(_T('='));
		if (eq > 0)
		{
			CString key = line.Left(eq);
			key.Trim();
			if (!key.IsEmpty())
				keys.push_back(key);
		}

		// 다음 문자열로 이동
		p += _tcslen(p) + 1;
	}

	// 숫자 key 기준 오름차순 정렬 (문자열 "010" "002" 이런거 안전)
	std::sort(keys.begin(), keys.end(),
		[](const CString& a, const CString& b)
		{
			int ia = _ttoi(a);
			int ib = _ttoi(b);
			return ia < ib;
		});

	// 중복 제거(혹시 같은 key가 여러번 들어가면)
	keys.erase(std::unique(keys.begin(), keys.end(),
		[](const CString& a, const CString& b)
		{
			return a.CompareNoCase(b) == 0;
		}), keys.end());

	return keys;
}

static CString BuildRecipeMsgSetFromModelList(
	const CString& machine,
	const CString& unit,
	const CString& modelListIniPath)
{
	CString recipeMsgSet;
	CString one;

	auto keys = ReadModelListKeys(modelListIniPath);
	if (keys.empty())
		return recipeMsgSet; // 비어있으면 빈 문자열 반환(원하면 기본값 처리 가능)

	for (size_t i = 0; i < keys.size(); ++i)
	{
		const CString& key = keys[i]; // "001", "002", ...

		if (i == keys.size() - 1)
			one.Format(_T("%s:%s:[%s]:3:U:"), machine, unit, key);
		else
			one.Format(_T("%s:%s:[%s]:3:U:,"), machine, unit, key);

		recipeMsgSet += one;
	}

	return recipeMsgSet;
}

//////////////////
// 레시피 번호와 모델번호 매칭 시킨 후 찾기
static BOOL FindModelNameByRecipeNo(const CString& modelDir, int recipeNo, CString& outModelName)
{
	outModelName.Empty();

	// 예: ".\\Model\\1_*.ini"
	CString pattern;
	pattern.Format(_T("%s\\%d_*.ini"), modelDir.GetString(), recipeNo);

	WIN32_FIND_DATA fd = { 0 };
	HANDLE h = FindFirstFile(pattern, &fd);
	if (h == INVALID_HANDLE_VALUE)
		return FALSE;

	// 첫 번째 매칭 파일명 사용
	CString file = fd.cFileName; // 예: "1_LP140WU3.ini"
	FindClose(h);

	// 확장자 제거
	int dot = file.ReverseFind(_T('.'));
	if (dot > 0)
		file = file.Left(dot);   // 예: "1_LP140WU3"

	outModelName = file;
	return TRUE;
}
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

			StartRmsRecvThread();

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

	lpInspWorkInfo = m_pApp->GetInspWorkInfo();

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
			//m_strLocalSubjectRmsF.Format(_T("%s,%s"), m_strLocalSubjectRMS, m_strLocalSubjectIP);
			m_strLocalSubjectRmsF.Format(_T("%s"), m_strLocalSubjectRMS);
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
	case ECS_MODE_ERCP:
		m_strHostSendMessage = m_strERCP;
		break;
	default:
		return RTN_MSG_NOT_SEND;	// 통신 NG
	}

	sLog.Format(_T("<HOST_R> %s"), m_strHostSendMessage);
	m_pApp->Gf_writeMLog(sLog);

	m_strHostRecvMessage.Format(_T("EMPTY"));

	Sleep (10);

	if(nMode != ECS_MODE_APDR && nMode != ECS_MODE_RMSO && nMode != ECS_MODE_ERCP)
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
	else if (nMode == ECS_MODE_RMSO || nMode == ECS_MODE_ERCP) // RMS 온도값 메시지
	{
		if (m_pApp->m_blsRmsConnect == FALSE)
			return RTN_MSG_NOT_SEND;

		StopRmsRecvThread();

		VARIANT_BOOL bRetCode = rms->SendTibMessage((_bstr_t)m_strHostSendMessage);


		do {
			if (rms->GetreceivedDataFlag() == VARIANT_TRUE) {
				m_sReceiveMessage = (LPCTSTR)rms->GetReceiveData();
				StartRmsRecvThread();
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
	Read_SysIniFile(_T("RMS"), _T("RMS_EQP"), &m_strEqpRMS);
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

void CCimNetCommApi::SetERCPInfo(CString strBuff)
{
	m_strERCPInfo.Format(_T("%s"), strBuff);
	TRACE(_T("[SetERCPInfo] this=%p, value=%s\n"), this, m_strERCPInfo);
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
	LPINSPWORKINFO lpInspWorkInfo = m_pApp->GetInspWorkInfo();
	TRACE(_T("[SetERCPInfo] this=%p, value=%s\n"), this, m_strERCPInfo);

	m_strERCP.Format(_T("ERCP ADDR=%s,%s EQP=%s MACHINE=%s UNIT=%s RCS=H MODE_CODE=N MODEL=%s BASEMODEL= CATEGORY= RECIPE= RECIPEVER= OPER= NW_CD= NW_DESCRIPTION=[] CHANGE_TYPE=B VALIDATIONINFO=[] UNIT_INFO=[%s:[1]:U:1:3:%s:0:[%s]] REPLY_REQ=Y TO_EQP= MMC_TXN_ID="),

	/*m_strLocalSubjectMesF,*/
	m_strRemoteSubjectRMS, // REMOTE SUBJECT ADDR=%s(1)
	m_strLocalSubjectRMS, // LOCAL SUBJECT ADDR=%s(2)
	m_strEqpRMS, // RMS EQP EQP=%s
	m_strMachineName.Left(m_strMachineName.GetLength() - 2), // MACHINE=%s
	m_strMachineName, // UNIT=%s
	r_strmodelName, // MODEL=%s


	m_strMachineName.Left(m_strMachineName.GetLength() - 2),
	m_strMachineName,
	m_strERCPInfo

	//m_strMachineName.Left(m_strMachineName.GetLength() - 2), // MACHINE
	//m_strMachineName, // UNIT
	//m_strPanelID,

	//m_strMachineName.Right(6),
	//lpInspWorkInfo->m_fTempReadValST590_2[0],
	//lpInspWorkInfo->m_fTempReadValST590_3[0],
	//lpInspWorkInfo->m_fTempReadValST590_4[0]

    

	/*m_strLocalIP,
	m_strServicePort,
	m_strClientDate*/
	);

	int nRetCode = MessageSend(ECS_MODE_ERCP);
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
	GetFieldData(&strMsg, _T("ERR_CD"));
	if (strMsg.Compare(_T("0")))
	{
		return 3;
	}

	return RTN_OK;	// normal


}
//////////////////////////////////////////////////////////////////////////////
BOOL CCimNetCommApi::EAYT ()
{
	MakeClientTimeString ();

	m_strEAYT.Format (_T ("EAYT ADDR=%s,%s EQP=%s NET_IP=%s NET_PORT=%s MODE=AUTO CLIENT_DATE=%s"),


	//m_strEAYT.Format(_T("ERCP ADDR=W4.G1.E3.RMS,W4.G3.W4AMAL01MH.UPLOADER EQP=W4ASY1010 MACHINE=W4AMAL01MH UNIT=W4AMAL01MH01 RCS=H MODE_CODE=N MODEL=LP160WU3-SPB2-KH1-B BASEMODEL= WODR=25BLM001N-FA07 CATEGORY=PROD RECIPE= RECIPEVER= OPER= NW_CD= NW_DESCRIPTION=[] CHANGE_TYPE=B VALIDATIONINFO=[]"),
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
	
	CString sLog;

	int nRetCode = MessageSend (ECS_MODE_AGN_IN);
	sLog.Format(_T("Send %s"), m_strAGN_IN);
	m_pApp->Gf_writeMLog_Rack(sLog, (lpInspWorkInfo->m_AgnInStartRack) + 1);
	sLog.Format(_T("Receive %s"), m_strHostRecvMessage);
	m_pApp->Gf_writeMLog_Rack(sLog, (lpInspWorkInfo->m_AgnInStartRack) + 1);

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
	CString sLog;
	sLog.Format(_T("Send %s"), m_strAGN_OUT);
	m_pApp->Gf_writeMLog_Rack(sLog, (lpInspWorkInfo->m_AgnInStartRack) + 1);
	sLog.Format(_T("Receive %s"), m_strHostRecvMessage);
	m_pApp->Gf_writeMLog_Rack(sLog, (lpInspWorkInfo->m_AgnInStartRack) + 1);
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

	m_strRMSO.Format(_T("%s")
		, lpInspWorkInfo->Ercp_Test_Message);

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

struct RMS_RECV_THREAD_PARAM
{
	CCimNetCommApi* pThis;
	IStream* pStream;   // COM marshaled interface stream
};

BOOL CCimNetCommApi::StartRmsRecvThread()
{
	// 이미 돌고 있으면 OK
	if (m_pRmsRecvThread != nullptr)
		return TRUE;

	// Stop Event 생성
	if (m_hRmsStopEvent == nullptr)
	{
		m_hRmsStopEvent = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
		if (m_hRmsStopEvent == nullptr)
			return FALSE;
	}
	::ResetEvent(m_hRmsStopEvent);

	// gmes 유효성 체크
	if (rms == nullptr)
		return FALSE;

	// ✅ 스레드에 COM 인터페이스를 안전하게 넘기기 위해 Marshal
	IStream* pStream = nullptr;
	HRESULT hr = CoMarshalInterThreadInterfaceInStream(IID_ICallRMSClass, rms, &pStream);
	if (FAILED(hr) || pStream == nullptr)
		return FALSE;

	auto* pParam = new RMS_RECV_THREAD_PARAM;
	pParam->pThis = this;
	pParam->pStream = pStream;

	m_pRmsRecvThread = AfxBeginThread(RmsRecvThreadProc, pParam);
	if (!m_pRmsRecvThread)
	{
		pStream->Release();
		delete pParam;
		return FALSE;
	}

	m_pRmsRecvThread->m_bAutoDelete = TRUE;
	return TRUE;
}

void CCimNetCommApi::StopRmsRecvThread()
{
	if (m_hRmsStopEvent)
		::SetEvent(m_hRmsStopEvent);

	// 스레드가 CWinThread라 Join 개념이 약해서, 약간 대기만
	if (m_pRmsRecvThread)
	{
		// 최대 1초 정도 기다렸다가 정리
		for (int i = 0; i < 100; i++)
		{
			::Sleep(10);
			// 강제 종료는 비추. stop event로 정상 종료 유도
		}
		m_pRmsRecvThread = nullptr;
	}

	if (m_hRmsStopEvent)
	{
		::CloseHandle(m_hRmsStopEvent);
		m_hRmsStopEvent = nullptr;
	}

	// 큐/마지막 메시지 정리(선택)
	{
		CSingleLock lock(&m_csRmsRecv, TRUE);
		m_lastRmsRecv.Empty();
		m_rmsRecvQueue.clear();
	}
}

UINT __cdecl CCimNetCommApi::RmsRecvThreadProc(LPVOID pParam)
{
	auto* param = reinterpret_cast<RMS_RECV_THREAD_PARAM*>(pParam);
	CCimNetCommApi* pThis = param->pThis;
	IStream* pStream = param->pStream;
	delete param; // ✅ param 해제 (pStream은 아래에서 ReleaseStream으로 처리)

	// ✅ 스레드에서 COM 초기화
	HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	// Marshal 해제하면서 인터페이스 복구
	ICallRMSClass* pRmsThread = nullptr;
	HRESULT hr = CoGetInterfaceAndReleaseStream(pStream, IID_ICallRMSClass, (void**)&pRmsThread);
	if (FAILED(hr) || pRmsThread == nullptr)
	{
		if (SUCCEEDED(hrCo)) CoUninitialize();
		return 0;
	}

	while (true)
	{
		// Stop 체크
		if (pThis->m_hRmsStopEvent &&
			::WaitForSingleObject(pThis->m_hRmsStopEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}

		// ✅ “항상 수신 대기”는 여기서 폴링/플래그 방식으로 구현
		// DLL 내부에서 DispatchThread가 _queue.Dispatch()로 이벤트를 올려서
		// _receivedFlag 세팅한다고 했으니, 여기서는 Flag만 감시하면 됨.
		if (pRmsThread->GetreceivedDataFlag() == VARIANT_TRUE)
		{
			_bstr_t b = pRmsThread->GetReceiveData(); // 이 호출이 flag를 false로 만든다는 구현이었음
			CString msg = (LPCTSTR)b;
			m_pApp->Gf_writeRMSLog(msg);

			CString cmd = msg.Left(4);
			if (cmd == _T("EPLR")) // EPLR 메시지
			{

				pThis->HandleRmsMsg_EPLR(msg, pRmsThread);
				//CString machine, unit, Eplr_Recipe_Info, Eplr_Recipe_MsgSet;
				//int posMachine = msg.Find(_T("MACHINE="));
				//if (posMachine >= 0)
				//{
				//	int start = posMachine + (int)_tcslen(_T("MACHINE="));
				//	machine = msg.Mid(start, 10);   // MACHINE 값 10글자
				//}

				//int posUnit = msg.Find(_T("UNIT="));
				//if (posUnit >= 0)
				//{
				//	int start = posUnit + (int)_tcslen(_T("UNIT="));
				//	unit = msg.Mid(start, 12);      // UNIT 값 12글자
				//}

				//auto ExtractTokenValue = [&](LPCTSTR key) -> CString
				//	{
				//		CString k(key);
				//		int pos = msg.Find(k);
				//		if (pos < 0) return _T("");

				//		int start = pos + k.GetLength();
				//		int end = msg.Find(_T(" "), start);
				//		if (end < 0) end = msg.GetLength();

				//		return msg.Mid(start, end - start);
				//	};
				//CString SEQ_No = ExtractTokenValue(_T("SEQ_NO="));

				//Eplr_Recipe_Info.Format(_T("%s:%s:[000]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[001]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[002]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[003]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[004]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[005]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[006]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[007]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[008]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[009]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[010]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[011]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[012]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[013]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[014]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[015]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[016]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[017]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[018]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[019]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[020]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[021]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[022]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[023]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[024]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[025]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[026]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[027]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[028]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[029]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[030]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[031]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[032]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[033]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[034]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[034]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[035]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[036]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[037]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[038]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[039]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[040]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[041]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[042]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[043]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[044]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[045]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[046]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[047]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[048]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[049]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[050]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[051]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[052]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[053]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[054]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[055]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[056]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[057]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[058]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[059]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[060]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[061]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[062]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[063]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[064]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[065]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[066]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[067]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[068]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[069]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[070]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[071]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[072]:3:U:,"), machine, unit);					Eplr_Recipe_MsgSet += Eplr_Recipe_Info;
				//Eplr_Recipe_Info.Format(_T("%s:%s:[073]:3:U:"), machine, unit);						Eplr_Recipe_MsgSet += Eplr_Recipe_Info;



				//// 1) EPLR_R 메시지 구성 (예시는 형식만)
				//CString reply;
				//reply.Format(_T("EPLR_R ADDR=%s,%s EQP=%s RECIPEINFO=[%s] ESD= ESDINFO=[] SEQ_NO=%s MMC_TXN_ID="),
				//	pThis->m_strRemoteSubjectRMS,
				//	pThis->m_strLocalSubjectRMS,
				//	pThis->m_strEqpRMS,
				//	Eplr_Recipe_MsgSet,
				//	SEQ_No 
				//);
				//m_pApp->Gf_writeRMSLog(reply);
				//// TODO: 실제 RMS 프로토콜에 맞게 필드 채우기

				//// 2) 딱 1번 전송 (대기 없음)
				////if (m_pApp->m_blsRmsConnect == FALSE)
				////	return; // 또는 처리

				//VARIANT_BOOL ok = pRmsThread->SendTibMessage((_bstr_t)reply);
				//
				//if (ok == VARIANT_FALSE)
				//	m_pApp->Gf_writeRMSLog(_T("[RMS] EPLR_R send failed"));
				//else
				//	m_pApp->Gf_writeRMSLog(_T("[RMS] EPLR_R send ok"));

				//// ✅ 여기서 끝. 수신대기 루프 절대 없음.
			}
			else if (cmd == _T("EPPR"))
			{
				pThis->HandleRmsMsg_EPPR(msg, pRmsThread);
				/*CString reply;
				reply.Format(_T("EPPR_R ADDR= EQP= RECIPEINFO=[::[]] ESD= SEQ_NO= MMC_TXN_ID"));

				m_pApp->Gf_writeRMSLog(reply);

				VARIANT_BOOL ok = pRmsThread->SendTibMessage((_bstr_t)reply);
				if (ok == VARIANT_FALSE)
					m_pApp->Gf_writeRMSLog(_T("[RMS] EPPR_R send failed"));
				else
					m_pApp->Gf_writeRMSLog(_T("[RMS] EPPR_R send ok"));*/
			}
			/*else if (cmd == _T("ERCP"))
			{
				CString reply;
				reply.Format(_T("ERCP_R ADDR= RTN_CD=0"));

				m_pApp->Gf_writeRMSLog(reply);

				VARIANT_BOOL ok = pRmsThread->SendTibMessage((_bstr_t)reply);
				if (ok == VARIANT_FALSE)
					m_pApp->Gf_writeRMSLog(_T("[RMS] ERCP_R send failed"));
				else
					m_pApp->Gf_writeRMSLog(_T("[RMS] ERCP_R send ok"));
			}*/
			else if (cmd == _T("EPSC"))
			{
				CString reply;
				reply.Format(_T("EPSC_R EQP= MACHINE= UNIT= SYSTEM= UNIT_TYPE= OPERATION_TYPE= COMMAND_CODE= PARACOUNT= SETTING_INFO= ACK= ERR_MSG_ENG= ERR_MSG_LOC= SEQ_NO= MMC_TXN_ID= USER_ID="));

				m_pApp->Gf_writeRMSLog(reply);

				VARIANT_BOOL ok = pRmsThread->SendTibMessage((_bstr_t)reply);
				if (ok == VARIANT_FALSE)
					m_pApp->Gf_writeRMSLog(_T("[RMS] EPSC_R send failed"));
				else
					m_pApp->Gf_writeRMSLog(_T("[RMS] EPSC_R send ok"));
			}
			else if (cmd == _T("EPDC"))
			{

			}

			// 저장 (최신 + 큐)
			{
				CSingleLock lock(&pThis->m_csRmsRecv, TRUE);
				pThis->m_lastRmsRecv = msg;
				pThis->m_rmsRecvQueue.push_back(msg);

				// 큐 메모리 제한 (원하는 만큼 조절)
				if (pThis->m_rmsRecvQueue.size() > 500)
					pThis->m_rmsRecvQueue.pop_front();
			}

			// 필요하면 여기서 UI 통지(PostMessage) 같은 걸 할 수도 있음.
		}

		::Sleep(5); // CPU 점유 줄이기
	}

	pRmsThread->Release();
	if (SUCCEEDED(hrCo)) CoUninitialize();
	return 0;
}

// ============================================================================
// Accessors
// ============================================================================

BOOL CCimNetCommApi::TryPopRmsMessage(CString& outMsg)
{
	CSingleLock lock(&m_csRmsRecv, TRUE);
	if (m_rmsRecvQueue.empty())
		return FALSE;

	outMsg = m_rmsRecvQueue.front();
	m_rmsRecvQueue.pop_front();
	return TRUE;
}

CString CCimNetCommApi::GetLastRmsMessage()
{
	CSingleLock lock(&m_csRmsRecv, TRUE);
	return m_lastRmsRecv;
}

void CCimNetCommApi::HandleRmsMsg_EPLR(const CString& msg, ICallRMSClass* pRmsThread)
{
	// 0) 안전 체크
	if (pRmsThread == nullptr)
		return;

	// 1) MACHINE / UNIT 추출
	CString machine, unit;

	int posMachine = msg.Find(_T("MACHINE="));
	if (posMachine >= 0)
	{
		int start = posMachine + (int)_tcslen(_T("MACHINE="));
		machine = msg.Mid(start, 10); // MACHINE 10글자 고정
	}

	int posUnit = msg.Find(_T("UNIT="));
	if (posUnit >= 0)
	{
		int start = posUnit + (int)_tcslen(_T("UNIT="));
		unit = msg.Mid(start, 12); // UNIT 12글자 고정
	}

	// 값이 없으면 그냥 종료(로그는 선택)
	if (machine.GetLength() != 10 || unit.GetLength() != 12)
	{
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPLR parse fail (MACHINE/UNIT)"));
		return;
	}

	// 2) SEQ_NO 추출
	auto ExtractTokenValue = [&](LPCTSTR key) -> CString
		{
			CString k(key);
			int pos = msg.Find(k);
			if (pos < 0) return _T("");

			int start = pos + k.GetLength();
			int end = msg.Find(_T(" "), start);
			if (end < 0) end = msg.GetLength();

			return msg.Mid(start, end - start);
		};

	CString seqNo = ExtractTokenValue(_T("SEQ_NO="));

	// 3) RECIPEINFO 문자열 생성 (루프로 짧게!)
	//CString recipeMsgSet, one;
	//for (int i = 0; i <= 73; ++i)
	//{
	//	// 마지막 항목만 ',' 없이 끝내려면
	//	if (i == 73)
	//		one.Format(_T("%s:%s:[%03d]:3:U:"), machine, unit, i);
	//	else
	//		one.Format(_T("%s:%s:[%03d]:3:U:,"), machine, unit, i);

	//	recipeMsgSet += one;
	//}
	CString modelListPath = _T(".\\Recipe\\ModelList.ini");
	CString recipeMsgSet = BuildRecipeMsgSetFromModelList(machine, unit, modelListPath);

	// 4) EPLR_R 구성
	CString reply;
	reply.Format(
		_T("EPLR_R ADDR=%s,%s EQP=%s RECIPEINFO=[%s] ESD= ESDINFO=[] SEQ_NO=%s MMC_TXN_ID="),
		m_strRemoteSubjectRMS,
		m_strLocalSubjectRMS,
		m_strEqpRMS,
		recipeMsgSet,
		seqNo
	);

	m_pApp->Gf_writeRMSLog(reply);

	// 5) 딱 1번 전송(대기 없음)
	VARIANT_BOOL ok = pRmsThread->SendTibMessage((_bstr_t)reply);

	if (ok == VARIANT_FALSE)
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPLR_R send failed"));
	else
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPLR_R send ok"));
}

void CCimNetCommApi::HandleRmsMsg_EPPR(const CString& msg, ICallRMSClass* pRmsThread)
{
	// 0) 안전 체크
	if (pRmsThread == nullptr)
		return;

	// 1) MACHINE / UNIT 추출
	CString machine, unit, recipe_no, recipe_type, recipe_yn, model_name, eqp_name;
	int cur_yn;

	int posMachine = msg.Find(_T("MACHINE="));
	if (posMachine >= 0)
	{
		int start = posMachine + (int)_tcslen(_T("MACHINE="));
		machine = msg.Mid(start, 10); // MACHINE 10글자 고정
	}

	int posUnit = msg.Find(_T("UNIT="));
	if (posUnit >= 0)
	{
		int start = posUnit + (int)_tcslen(_T("UNIT="));
		unit = msg.Mid(start, 12); // UNIT 12글자 고정
	}

	//int posNo = msg.Find(_T("RECIPE="));
	//if (posNo >= 0)
	//{
	//	int start = posNo + (int)_tcslen(_T("RECIPE="));
	//	recipe_no = msg.Mid(start, 3); //  RECIPE 3글자 고정
	//}
	int pos = msg.Find(_T("RECIPE="));
	if (pos >= 0)
	{
		int start = pos + (int)_tcslen(_T("RECIPE="));

		// 1) 공백 전까지 자르기
		int end = msg.Find(_T(' '), start);
		if (end < 0) end = msg.GetLength();   // 혹시 공백이 없으면 끝까지

		CString token = msg.Mid(start, end - start);
		token.Trim(); // 혹시 모를 공백 제거

		// 2) "001" -> 1 로 변환 후 다시 문자열로
		int n = _ttoi(token);                 // 앞의 0 자동 제거됨
		recipe_no.Format(_T("%d"), n);        // "1", "16", "33"
	}

	int posType = msg.Find(_T("RECIPETYPE="));
	if (posType >= 0)
	{
		int start = posType + (int)_tcslen(_T("RECIPETYPE="));
		recipe_type = msg.Mid(start, 1); // UNIT 1글자 고정
	}

	int posYn = msg.Find(_T("CURRENT_RECIPE_YN="));
	if (posYn >= 0)
	{
		int start = posYn + (int)_tcslen(_T("CURRENT_RECIPE_YN="));
		recipe_yn = msg.Mid(start, 1); // UNIT 1글자 고정
	}

	if (recipe_yn == "Y")
	{
		cur_yn = 6;
	}
	if (recipe_yn == "N")
	{
		cur_yn = 4;
	}
	

	int recipeNoInt = _ttoi(recipe_no);

	CString modelDir = _T(".\\Model");
	if (!FindModelNameByRecipeNo(modelDir, recipeNoInt, model_name))
	{
		CString msgErr;
		msgErr.Format(_T("Model file not found for RECIPE=%d\r\nSearch: %s\\%d_*.ini"),
			recipeNoInt, modelDir.GetString(), recipeNoInt);
		m_pApp->Gf_writeRMSLog(msgErr);
		return; // 또는 기본 모델 처리
	}

	// 값이 없으면 그냥 종료(로그는 선택)
	if (machine.GetLength() != 10 || unit.GetLength() != 12)
	{
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPLR parse fail (MACHINE/UNIT)"));
		return;
	}

	// 2) SEQ_NO 추출
	auto ExtractTokenValue = [&](LPCTSTR key) -> CString
		{
			CString k(key);
			int pos = msg.Find(k);
			if (pos < 0) return _T("");

			int start = pos + k.GetLength();
			int end = msg.Find(_T(" "), start);
			if (end < 0) end = msg.GetLength();

			return msg.Mid(start, end - start);
		};

	CString seqNo = ExtractTokenValue(_T("SEQ_NO="));

	
	lpModelInfo = m_pApp->GetModelInfo();
	lpSystemInfo = m_pApp->GetSystemInfo();

	// ✅ 찾은 모델 로딩
	m_pApp->Gf_loadModelData(model_name);

	CString recipeMsgSet;

	eqp_name = lpSystemInfo->m_sEqpName;

	model_name.Format(_T("MODEL_NB$%d;"), cur_yn);														recipeMsgSet += model_name; // MODEL_NUMBER
	model_name.Format(_T("DIMMING_SEL_MODEL_INFO$%d;"), lpModelInfo->m_nDimmingSel);					recipeMsgSet += model_name; // DIMMING SEL
	model_name.Format(_T("PWM_FREQ_MODEL_INFO$%d;"),  lpModelInfo->m_nPwmFreq);							recipeMsgSet += model_name; // PWM_FREQ
	model_name.Format(_T("PWM_DUTY_MODEL_INFO$%d;"), lpModelInfo->m_nPwmDuty);							recipeMsgSet += model_name; // PWM_DUTY
	model_name.Format(_T("VBR_VOLT_MODEL_INFO$%f;"), lpModelInfo->m_fVbrVolt);							recipeMsgSet += model_name; // VBR_VOLT
	model_name.Format(_T("CABLE_OPEN_MODEL_INFO$%d;"), lpModelInfo->m_nFuncCableOpen);					recipeMsgSet += model_name; // CABLE_OPEN
	model_name.Format(_T("POWER_ON_SEQ1_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnSeq1);					recipeMsgSet += model_name; // POWER_ON_SEQ1
	model_name.Format(_T("POWER_ON_SEQ2_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnSeq2);					recipeMsgSet += model_name; // POWER_ON_SEQ2
	model_name.Format(_T("POWER_ON_SEQ3_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnSeq3);					recipeMsgSet += model_name; // POWER_ON_SEQ3
	model_name.Format(_T("POWER_ON_SEQ4_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnSeq4);					recipeMsgSet += model_name; // POWER_ON_SEQ4
	model_name.Format(_T("POWER_ON_SEQ5_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnSeq5);					recipeMsgSet += model_name; // POWER_ON_SEQ5
	model_name.Format(_T("POWER_ON_SEQ6_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnSeq6);					recipeMsgSet += model_name; // POWER_ON_SEQ6
	model_name.Format(_T("POWER_ON_SEQ7_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnSeq7);					recipeMsgSet += model_name; // POWER_ON_SEQ7
	model_name.Format(_T("POWER_ON_SEQ8_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnSeq8);					recipeMsgSet += model_name; // POWER_ON_SEQ8
	model_name.Format(_T("POWER_ON_SEQ9_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnSeq9);					recipeMsgSet += model_name; // POWER_ON_SEQ9
	model_name.Format(_T("POWER_ON_SEQ10_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnSeq10);				recipeMsgSet += model_name; // POWER_ON_SEQ10
	model_name.Format(_T("POWER_ON_DELAY1_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnDelay1);				recipeMsgSet += model_name; // POWER_ON_DELAY1
	model_name.Format(_T("POWER_ON_DELAY2_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnDelay2);				recipeMsgSet += model_name; // POWER_ON_DELAY2
	model_name.Format(_T("POWER_ON_DELAY3_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnDelay3);				recipeMsgSet += model_name; // POWER_ON_DELAY3
	model_name.Format(_T("POWER_ON_DELAY4_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnDelay4);				recipeMsgSet += model_name; // POWER_ON_DELAY4
	model_name.Format(_T("POWER_ON_DELAY5_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnDelay5);				recipeMsgSet += model_name; // POWER_ON_DELAY5
	model_name.Format(_T("POWER_ON_DELAY6_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnDelay6);				recipeMsgSet += model_name; // POWER_ON_DELAY6
	model_name.Format(_T("POWER_ON_DELAY7_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnDelay7);				recipeMsgSet += model_name; // POWER_ON_DELAY7
	model_name.Format(_T("POWER_ON_DELAY8_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnDelay8);				recipeMsgSet += model_name; // POWER_ON_DELAY8
	model_name.Format(_T("POWER_ON_DELAY9_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOnDelay9);				recipeMsgSet += model_name; // POWER_ON_DELAY9


	model_name.Format(_T("POWER_OFF_SEQ1_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffSeq1);				recipeMsgSet += model_name; // POWER_OFF_SEQ1
	model_name.Format(_T("POWER_OFF_SEQ2_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffSeq2);				recipeMsgSet += model_name; // POWER_OFF_SEQ2
	model_name.Format(_T("POWER_OFF_SEQ3_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffSeq3);				recipeMsgSet += model_name; // POWER_OFF_SEQ3
	model_name.Format(_T("POWER_OFF_SEQ4_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffSeq4);				recipeMsgSet += model_name; // POWER_OFF_SEQ4
	model_name.Format(_T("POWER_OFF_SEQ5_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffSeq5);				recipeMsgSet += model_name; // POWER_OFF_SEQ5
	model_name.Format(_T("POWER_OFF_SEQ6_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffSeq6);				recipeMsgSet += model_name; // POWER_OFF_SEQ6
	model_name.Format(_T("POWER_OFF_SEQ7_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffSeq7);				recipeMsgSet += model_name; // POWER_OFF_SEQ7
	model_name.Format(_T("POWER_OFF_SEQ8_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffSeq8);				recipeMsgSet += model_name; // POWER_OFF_SEQ8
	model_name.Format(_T("POWER_OFF_SEQ9_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffSeq9);				recipeMsgSet += model_name; // POWER_OFF_SEQ9
	model_name.Format(_T("POWER_OFF_SEQ10_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffSeq10);				recipeMsgSet += model_name; // POWER_OFF_SEQ10
	model_name.Format(_T("POWER_OFF_DELAY1_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffDelay1);			recipeMsgSet += model_name; // POWER_OFF_DELAY1
	model_name.Format(_T("POWER_OFF_DELAY2_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffDelay2);			recipeMsgSet += model_name; // POWER_OFF_DELAY2
	model_name.Format(_T("POWER_OFF_DELAY3_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffDelay3);			recipeMsgSet += model_name; // POWER_OFF_DELAY3
	model_name.Format(_T("POWER_OFF_DELAY4_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffDelay4);			recipeMsgSet += model_name; // POWER_OFF_DELAY4
	model_name.Format(_T("POWER_OFF_DELAY5_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffDelay5);			recipeMsgSet += model_name; // POWER_OFF_DELAY5
	model_name.Format(_T("POWER_OFF_DELAY6_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffDelay6);			recipeMsgSet += model_name; // POWER_OFF_DELAY6
	model_name.Format(_T("POWER_OFF_DELAY7_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffDelay7);			recipeMsgSet += model_name; // POWER_OFF_DELAY7
	model_name.Format(_T("POWER_OFF_DELAY8_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffDelay8);			recipeMsgSet += model_name; // POWER_OFF_DELAY8
	model_name.Format(_T("POWER_OFF_DELAY9_MODEL_INFO$%d;"), lpModelInfo->m_nPowerOffDelay9);			recipeMsgSet += model_name; // POWER_OFF_DELAY9
	model_name.Format(_T("VCC_VOLT_MODEL_INFO$%f;"), lpModelInfo->m_fVccVolt);							recipeMsgSet += model_name; // VCC_VOLT
	model_name.Format(_T("VCC_VOLT_OFFSET_MODEL_INFO$%f;"), lpModelInfo->m_fVccVoltOffset);				recipeMsgSet += model_name; // VCC_VOLT_OFFSET
	model_name.Format(_T("VCC_LIMIT_VOLT_LOW_MODEL_INFO$%f;"), lpModelInfo->m_fVccLimitVoltLow);		recipeMsgSet += model_name; // VCC_LIMIT_VOLT_LOW
	model_name.Format(_T("VCC_LIMIT_VOLT_HIGH_MODEL_INFO$%f;"), lpModelInfo->m_fVccLimitVoltHigh);		recipeMsgSet += model_name; // VCC_LIMIT_VOLT_HIGH
	model_name.Format(_T("VCC_LIMIT_CURR_LOW_MODEL_INFO$%f;"), lpModelInfo->m_fVccLimitCurrLow);		recipeMsgSet += model_name; // VCC_LIMIT_CURR_LOW
	model_name.Format(_T("VCC_LIMIT_CURR_HIGH_MODEL_INFO$%f;"), lpModelInfo->m_fVccLimitCurrHigh);		recipeMsgSet += model_name; // VCC_LIMIT_CURR_HIGH
	model_name.Format(_T("VBL_VOLT_MODEL_INFO$%f;"), lpModelInfo->m_fVblVolt);							recipeMsgSet += model_name; // VBL_VOLT
	model_name.Format(_T("VBL_OFFSET_MODEL_INFO$%f;"), lpModelInfo->m_fVblVoltOffset);					recipeMsgSet += model_name; // VBL_VOLT_OFFSET
	model_name.Format(_T("VBL_LIMIT_VOLT_LOW_MODEL_INFO$%f;"), lpModelInfo->m_fVblLimitVoltLow);		recipeMsgSet += model_name; // VBL_LIMIT_VOLT_LOW
	model_name.Format(_T("VBL_LIMIT_VOLT_HIGH_MODEL_INFO$%f;"), lpModelInfo->m_fVblLimitVoltHigh);		recipeMsgSet += model_name; // VBL_LIMIT_VOLT_HIGH
	model_name.Format(_T("VBL_LIMIT_CURR_LOW_MODEL_INFO$%f;"), lpModelInfo->m_fVblLimitCurrLow);		recipeMsgSet += model_name; // VBL_LIMIT_CURR_LOW
	model_name.Format(_T("VBL_LIMIT_CURR_HIGH_MODEL_INFO$%f;"), lpModelInfo->m_fVblLimitCurrHigh);		recipeMsgSet += model_name; // VBL_LIMIT_CURR_HIGH
	model_name.Format(_T("AGING_TIME_HH_MODEL_INFO$%d;"), lpModelInfo->m_nAgingTimeHH);					recipeMsgSet += model_name; // AGING_TIME_HH
	model_name.Format(_T("AGING_TIME_MM_MODEL_INFO$%d;"), lpModelInfo->m_nAgingTimeMM);					recipeMsgSet += model_name; // AGING_TIME_MM
	model_name.Format(_T("AGING_TIME_MINUTE_MODEL_INFO$%d;"), lpModelInfo->m_nAgingTimeMinute);			recipeMsgSet += model_name; // AGING_TIME_MINUTE
	model_name.Format(_T("AGING_END_WAIT_TIME_MODEL_INFO$%d;"), lpModelInfo->m_nAgingEndWaitTime);		recipeMsgSet += model_name; // AGING_END_WAIT_TIM
	model_name.Format(_T("TEMPERATURE_USE_MODEL_INFO$%d;"), lpModelInfo->m_nOpeTemperatureUse);			recipeMsgSet += model_name; // TEMPERATURE_USE
	model_name.Format(_T("TEMPERATURE_MIN_MODEL_INFO$%d;"), lpModelInfo->m_nOpeTemperatureMin);			recipeMsgSet += model_name; // TEMPERATURE_MIN
	model_name.Format(_T("TEMPERATURE_MAX_MODEL_INFO$%d;"), lpModelInfo->m_nOpeTemperatureMax);			recipeMsgSet += model_name; // TEMPERATURE_MAX
	model_name.Format(_T("DOOR_USE_MODEL_INFO$%d"), lpModelInfo->m_nOpeDoorUse);						recipeMsgSet += model_name; // DOOR_USE
	//model_name.Format(_T("TEMP_ZONE_S1$%f;"), lpInspWorkInfo->m_fTempReadVal[0]);						recipeMsgSet += model_name; // 1ZONE 온도 (S1)
	//model_name.Format(_T("TEMP_ZONE_S2$%f;"), lpInspWorkInfo->m_fTempReadVal[1]);						recipeMsgSet += model_name; // 1ZONE 온도 (S2)
	//model_name.Format(_T("TEMP_ZONE_S3$%f;"), lpInspWorkInfo->m_fTempReadVal[2]);						recipeMsgSet += model_name; // 2ZONE 온도 (S3)
	//model_name.Format(_T("TEMP_ZONE_S4$%f;"), lpInspWorkInfo->m_fTempReadVal[3]);						recipeMsgSet += model_name; // 2ZONE 온도 (S4)
	//model_name.Format(_T("TEMP_ZONE_S5$%f;"), lpInspWorkInfo->m_fTempReadVal[4]);						recipeMsgSet += model_name; // 3ZONE 온도 (S5)
	//model_name.Format(_T("TEMP_ZONE_S6$%f;"), lpInspWorkInfo->m_fTempReadVal[5]);						recipeMsgSet += model_name; // 3ZONE 온도 (S6)
	//model_name.Format(_T("TEMP_ZONE_SET1$%f;"), lpInspWorkInfo->m_fTempReadValST590_2[0]);				recipeMsgSet += model_name; // 1ZONE 메인 컨트롤러 세팅값
	//model_name.Format(_T("TEMP_ZONE_SET2$%f;"), lpInspWorkInfo->m_fTempReadValST590_2[1]);				recipeMsgSet += model_name; // 2ZONE 메인 컨트롤러 세팅값
	//model_name.Format(_T("TEMP_ZONE_SET3$%f;"), lpInspWorkInfo->m_fTempReadValST590_2[2]);				recipeMsgSet += model_name; // 3ZONE 메인 컨트롤러 세팅값

	CString reply;
	reply.Format(
		_T("EPPR_R ADDR=%s,%s EQP=%s RECIPEINFO=[%s:%s:[%d]:3:U:%s::[0#[%s]]] ESD= ESDINFO=[] SEQ_NO=%s MMC_TXN_ID="),
		m_strRemoteSubjectRMS,
		m_strLocalSubjectRMS,
		m_strEqpRMS,
		eqp_name.Left(eqp_name.GetLength() - 2),
		m_strEqpRMS,
		cur_yn,
		recipe_yn,
		recipeMsgSet,
		seqNo
	);

	m_pApp->Gf_writeRMSLog(reply);

	VARIANT_BOOL ok = pRmsThread->SendTibMessage((_bstr_t)reply);
	if (ok == VARIANT_FALSE)
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPPR_R send failed"));
	else
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPPR_R send ok"));
}