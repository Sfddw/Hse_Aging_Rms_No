#pragma once

#include <direct.h>
#include <io.h>

class CIniProfileAPI
{
public:
	CIniProfileAPI(void);
	~CIniProfileAPI(void);
};

static TCHAR	wszConv[2048]={0,};
static char		szConv[2048]={0,};
static CString	sRetString;

/////////////////////////////////////////////////////////////////////////////
static void ProcessMessage()
{
	MSG msg;
	if( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/////////////////////////////////////////////////////////////////////////////
static void delayMs(DWORD delay)
{
	DWORD stTick, edTick;

	stTick = ::GetTickCount();
	while(1)
	{
		edTick = ::GetTickCount();
		if((edTick-stTick) >= delay)
			break;

		ProcessMessage();
	}
}

/////////////////////////////////////////////////////////////////////////////
static int _str_to_dec(CString hexStr)
{
	int dwRtnData=0;
	TCHAR tmpdata=0;
	int nLength=0, index=0;

	hexStr.MakeUpper();

	nLength = hexStr.GetLength();
	while(nLength != 0)
	{
		// Hex Char에 해당하는 Decimal 값을 가져온다.
		if(hexStr.GetAt(index)=='0')			tmpdata=0;
		else if(hexStr.GetAt(index)=='1')		tmpdata=1;
		else if(hexStr.GetAt(index)=='2')		tmpdata=2;
		else if(hexStr.GetAt(index)=='3')		tmpdata=3;
		else if(hexStr.GetAt(index)=='4')		tmpdata=4;
		else if(hexStr.GetAt(index)=='5')		tmpdata=5;
		else if(hexStr.GetAt(index)=='6')		tmpdata=6;
		else if(hexStr.GetAt(index)=='7')		tmpdata=7;
		else if(hexStr.GetAt(index)=='8')		tmpdata=8;
		else if(hexStr.GetAt(index)=='9')		tmpdata=9;
		else if(hexStr.GetAt(index)=='A')		tmpdata=10;
		else if(hexStr.GetAt(index)=='B')		tmpdata=11;
		else if(hexStr.GetAt(index)=='C')		tmpdata=12;
		else if(hexStr.GetAt(index)=='D')		tmpdata=13;
		else if(hexStr.GetAt(index)=='E')		tmpdata=14;
		else if(hexStr.GetAt(index)=='F')		tmpdata=15;

		// 결과 값을 누적한다.
		dwRtnData += tmpdata;

		// Index 값을 증가 시킨다.
		index++;

		// 마지막 Data이면 while문을 빠져 나간다.
		if(nLength == index)	break;

		// 누적된 Data 값을 Shift 시킨다.
		dwRtnData <<= 4;
	}

	return dwRtnData;
}

/////////////////////////////////////////////////////////////////////////////
static void char_To_wchar(char* szOrg, wchar_t* wszConv)
{
	// char*형의 모델명을 wchar 형태의 모델명으로 변환.
	int mlen = (int)strlen(szOrg);
	int wlen = 0;
	wlen		= MultiByteToWideChar(CP_ACP,0,szOrg,mlen,NULL,0);
	memset((void*)wszConv, 0, sizeof(WCHAR)*(wlen+1));
	wlen		= MultiByteToWideChar(CP_ACP,0,szOrg,mlen,wszConv,wlen);
}

static wchar_t* char_To_wchar(char* szOrg)
{
	// char*형의 모델명을 wchar 형태의 모델명으로 변환.
	int mlen = (int)strlen(szOrg);
	int wlen = 0;
	wlen		= MultiByteToWideChar(CP_ACP,0,szOrg,mlen,NULL,0);
	memset((void*)wszConv, 0, sizeof(WCHAR)*(wlen+1));
	wlen		= MultiByteToWideChar(CP_ACP,0,szOrg,mlen,wszConv,wlen);

	return wszConv;
}

static void wchar_To_char(wchar_t* wszOrg, char* szConv)
{
	//먼저 길이를 구한다.
	int nMultiByteLen = WideCharToMultiByte(CP_ACP, 0, wszOrg, -1, NULL, 0, NULL,NULL);
	//변환한다.
	WideCharToMultiByte(CP_ACP, 0, wszOrg, -1, szConv, nMultiByteLen, NULL, NULL);
}

static char* wchar_To_char(wchar_t* wszOrg)
{
	//먼저 길이를 구한다.
	int nMultiByteLen = WideCharToMultiByte(CP_ACP, 0, wszOrg, -1, NULL, 0, NULL,NULL);
	//변환한다.
	WideCharToMultiByte(CP_ACP, 0, wszOrg, -1, szConv, nMultiByteLen, NULL, NULL);

	return szConv;
}


static int CALLBACK BrowseForFolder_CallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED)
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)lpData);
	return 0;

}

static void Write_ProfileString(LPCWSTR lpFileName, LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{
	::WritePrivateProfileString(lpTitle, lpKey, lpValue, lpFileName);
}

static void Read_ProfileString(LPCWSTR lpFileName, LPCWSTR lpTitle, LPCWSTR lpKey, CString *retValue)
{
	wchar_t wszData[512] = {0,};
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), lpFileName);

	retValue->Format(_T("%s"), wszData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{

	::WritePrivateProfileString(lpTitle, lpKey, lpValue, _T(".\\INI\\Operation.ini"));
}

static void Write_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, int nData)
{
	CString szData;

	szData.Format(_T("%d"), nData);
	::WritePrivateProfileString(lpTitle, lpKey, szData, _T(".\\INI\\Operation.ini"));
}

static void Write_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, float fData)
{
	CString szData;

	szData.Format(_T("%f"), fData);
	::WritePrivateProfileString(lpTitle, lpKey, szData, _T(".\\INI\\Operation.ini"));
}

static void Read_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, char *szRetString)
{
	wchar_t wszData[100] = {0,};


	memset(szRetString,'\0',sizeof(szRetString));
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\Operation.ini"));

	wchar_To_char(wszData, szRetString);
}

static void Read_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, int *pRetValue)
{
	wchar_t wszData[100] = {0,};

	*pRetValue = 0;
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\Operation.ini"));        

	*pRetValue = _ttoi(wszData);
}

static void Read_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, float *pRetValue)
{
	wchar_t wszData[100] = {0,};

	*pRetValue = 0;
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\Operation.ini"));        

	*pRetValue = (float)_tstof(wszData);
}

static void Read_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, CString *szRetString)
{
	wchar_t wszData[100] = {0,};

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\Operation.ini"));

	szRetString->Format(_T("%s"), wszData);
}

static void Read_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, TCHAR *wszRetString)
{
	wchar_t wszData[100] = {0,};

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\Operation.ini"));
	wcscpy_s(wszRetString, (wcslen(wszData)+1), wszData);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_ModelFile(LPCWSTR groupName, LPCWSTR lpModelName, LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{
	CString szModelPath;

	szModelPath.Format(_T(".\\Model\\%s\\%s.MOD"), groupName, lpModelName);
	::WritePrivateProfileString(lpTitle, lpKey, lpValue, szModelPath);        
}

static void Write_ModelFile(LPCWSTR groupName, LPCWSTR lpModelName, LPCWSTR lpTitle, LPCWSTR lpKey, int ndata)
{
	CString szData;
	CString szModelPath;

	szData.Format(_T("%d"), ndata);
	szModelPath.Format(_T(".\\Model\\%s\\%s.MOD"), groupName, lpModelName);
	::WritePrivateProfileString(lpTitle, lpKey, szData, szModelPath);        
}

static void Write_ModelFile(LPCWSTR groupName, LPCWSTR lpModelName, LPCWSTR lpTitle, LPCWSTR lpKey, long nData)
{
	CString szData;
	CString szModelPath;

	szData.Format(_T("%d"), nData);
	szModelPath.Format(_T(".\\Model\\%s\\%s.MOD"), groupName, lpModelName);
	::WritePrivateProfileString(lpTitle, lpKey, szData, szModelPath);        
}

static void Write_ModelFile(LPCWSTR groupName, LPCWSTR lpModelName, LPCWSTR lpTitle, LPCWSTR lpKey, double fdata)
{
	CString szData;
	CString szModelPath;

	szData.Format(_T("%f"), fdata);
	szModelPath.Format(_T(".\\Model\\%s\\%s.MOD"), groupName, lpModelName);
	::WritePrivateProfileString(lpTitle, lpKey, szData, szModelPath);        
}

static void Read_ModelFile(LPCWSTR groupName, CString lpModelName, LPCWSTR lpTitle, LPCWSTR lpKey, CString *szRetString)
{
	wchar_t wszData[100] = {0,};
	CString szModelPath;

	// 모델 File 경로 설정
	szModelPath.Format(_T(".\\Model\\%s\\%s.MOD"), groupName, lpModelName);

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), szModelPath);

	szRetString->Format(_T("%s"), wszData);
}

static void Read_ModelFile(LPCWSTR groupName, CString lpModelName, LPCWSTR lpTitle, LPCWSTR lpKey, char *szRetString)
{
	wchar_t wszData[100] = {0,};
	CString szModelPath;

	// Return Memory Initialize
	memset(szRetString,'\0',sizeof(szRetString));

	// 모델 File 경로 설정
	szModelPath.Format(_T(".\\Model\\%s\\%s.MOD"), groupName, lpModelName);

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), szModelPath);

	wchar_To_char(wszData, szRetString);
}

static void Read_ModelFile(LPCWSTR groupName, CString lpModelName, LPCWSTR lpTitle, LPCWSTR lpKey, int *pRetValue)
{
	CString szModelPath;
	wchar_t wszData[100] = {0,};
	char szData[50] = {0,};

	// 모델 File 경로 설정
	*pRetValue = 0;
	szModelPath.Format(_T(".\\Model\\%s\\%s.MOD"), groupName, lpModelName);

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), szModelPath);        

	wchar_To_char(wszData, szData);

	*pRetValue = atoi(szData);
}

static void Read_ModelFile(LPCWSTR groupName, CString lpModelName, LPCWSTR lpTitle, LPCWSTR lpKey, LONG *pRetValue)
{
	CString szModelPath;
	wchar_t wszData[100] = {0,};

	// 모델 File 경로 설정
	*pRetValue = 0;
	szModelPath.Format(_T(".\\Model\\%s\\%s.MOD"), groupName, lpModelName);

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), szModelPath);        

	*pRetValue = wcstol(wszData, NULL, 10);
}

static void Read_ModelFile(LPCWSTR groupName, CString lpModelName, LPCWSTR lpTitle, LPCWSTR lpKey, float *pRetValue)
{
	CString szModelPath;
	wchar_t wszData[100] = {0,};
	char szData[50] = {0,};

	// 모델 File 경로 설정
	*pRetValue = 0;
	szModelPath.Format(_T(".\\Model\\%s\\%s.MOD"), groupName, lpModelName);

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), szModelPath);        

	wchar_To_char(wszData, szData);

	*pRetValue = (float)atof(szData);
}

static void Read_ModelFile(LPCWSTR groupName, CString lpModelName, LPCWSTR lpTitle, LPCWSTR lpKey, double *pRetValue)
{
	CString szModelPath;
	wchar_t wszData[100] = {0,};
	char szData[50] = {0,};

	// 모델 File 경로 설정
	*pRetValue = 0;
	szModelPath.Format(_T(".\\Model\\%s\\%s.MOD"), groupName, lpModelName);

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), szModelPath);        

	wchar_To_char(wszData, szData);

	*pRetValue = atof(szData);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_IDMatchingFile(LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{
	::WritePrivateProfileString(lpTitle, lpKey, lpValue, _T(".\\Ini\\IDMatching.ini"));        
}

static void Read_IDMatchingFile(LPCWSTR lpTitle, LPCWSTR lpKey, CString *szRetString)
{
	wchar_t wszData[100] = {0,};

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\Ini\\IDMatching.ini"));

	szRetString->Format(_T("%s"), wszData);
}

static void Read_IDMatchingFile(LPCWSTR lpTitle, LPCWSTR lpKey, int *pRetValue)
{
	wchar_t wszData[100] = {0,};

	*pRetValue = 0;
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\Ini\\IDMatching.ini"));

	*pRetValue = _ttoi(wszData);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void Write_WorkingRatioFile(CString filePath, LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{
	::WritePrivateProfileString(lpTitle, lpKey, lpValue, filePath);        
}

static void Read_WorkingRatioFile(CString filePath, LPCWSTR lpTitle, LPCWSTR lpKey, CString *szRetString)
{
	wchar_t wszData[100] = {0,};

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), filePath);

	szRetString->Format(_T("%s"), wszData);
}

static void Read_WorkingRatioFile(CString filePath, LPCWSTR lpTitle, LPCWSTR lpKey, int *pRetValue)
{
	wchar_t wszData[100] = {0,};

	*pRetValue = 0;

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), filePath);

	*pRetValue = _ttoi(wszData);
}


static void Write_GmesFile(LPCWSTR lpTitle, LPCWSTR lpKey, CString strValue)
{
	::WritePrivateProfileString(lpTitle, lpKey, strValue, _T(".\\INI\\GMES.ini"));
}

static void Read_GmesFile(LPCWSTR lpTitle, LPCWSTR lpKey, CString *szRetString)
{
	wchar_t wszData[100] = {0,};

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\GMES.ini"));

	szRetString->Format(_T("%s"), wszData);
}

static void Read_ErrorCode(LPCWSTR lpTitle, LPCWSTR lpKey, CString *szRetString)
{
	wchar_t wszData[100] = {0,};

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\Error_Code.ini"));

	szRetString->Format(_T("%s"), wszData);

	// 	int pos=0;
	// 	pos=szRetString->Find(_T("="),0);
	// 	if (pos!=-1)
	// 	{
	// 		szRetString->Delete(0,pos+1);
	// 		pos=szRetString->Find(_T("="),0);
	// 		if (pos!=-1)
	// 		{
	// 			szRetString->Delete(pos,szRetString->GetLength()-pos);
	// 		}
	// 	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_MesStatusInfo(LPCWSTR lpTitle, LPCWSTR lpKey, int nValue)
{
	CString sdata;

	sdata.Format(_T("%d"), nValue);
	::WritePrivateProfileString(lpTitle, lpKey, sdata, _T(".\\Ini\\MesStatus.ini"));        
}

static void Read_MesStatusInfo(LPCWSTR lpTitle, LPCWSTR lpKey, int *pRetValue)
{
	wchar_t wszData[100] = {0,};

	*pRetValue = 0;
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\Ini\\MesStatus.ini"));

	*pRetValue = (char)_ttoi(wszData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static BOOL Write_agingStartEndTime(LPCWSTR lpTitle, LPCWSTR lpKey, CString sValue)
{
	return ::WritePrivateProfileString(lpTitle, lpKey, sValue, _T(".\\Ini\\StartEndTime.ini"));        
}

static void Read_agingStartEndTime(LPCWSTR lpTitle, LPCWSTR lpKey, __int64 *pRetValue)
{
	wchar_t wszData[100] = {0,};

	*pRetValue = 0;
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\Ini\\StartEndTime.ini"));

	*pRetValue = _ttoi64(wszData);
}

static void Read_agingStartEndTime(LPCWSTR lpTitle, LPCWSTR lpKey, int *pRetValue)
{
	wchar_t wszData[100] = {0,};

	*pRetValue = 0;
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\Ini\\StartEndTime.ini"));

	*pRetValue = _ttoi(wszData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_SummaryInfo(LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{

	::WritePrivateProfileString(lpTitle, lpKey, lpValue, _T(".\\INI\\Summary.ini"));
}

static void Read_SummaryInfo(LPCWSTR lpTitle, LPCWSTR lpKey, CString *szRetString)
{
	wchar_t wszData[100] = {0,};

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\Summary.ini"));

	szRetString->Format(_T("%s"), wszData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_HingeGroupInfo(LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{

	::WritePrivateProfileString(lpTitle, lpKey, lpValue, _T(".\\INI\\HingeGroup.ini"));
}

static void Read_HingeGroupInfo(LPCWSTR lpTitle, LPCWSTR lpKey, CString *szRetString)
{
	wchar_t wszData[1024] = {0,};

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\HingeGroup.ini"));

	szRetString->Format(_T("%s"), wszData);
}

static void Read_HingeGroupInfo(LPCWSTR lpTitle, LPCWSTR lpKey, int *pRetValue)
{
	CString szModelPath;
	wchar_t wszData[100] = {0,};
	char szData[50] = {0,};

	// 모델 File 경로 설정
	*pRetValue = 0;

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\HingeGroup.ini"));  

	wchar_To_char(wszData, szData);

	*pRetValue = atoi(szData);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_HingeDB(LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{
	::WritePrivateProfileString(lpTitle, lpKey, lpValue, _T(".\\INI\\HingeDB.ini"));
}

static void Write_HingeDB(LPCWSTR lpTitle, LPCWSTR lpKey, int nValue)
{
	CString sdata=_T("");

	sdata.Format(_T("%d"), nValue);

	::WritePrivateProfileString(lpTitle, lpKey, sdata, _T(".\\INI\\HingeDB.ini"));
}

static void Read_HingeDB(LPCWSTR lpTitle, LPCWSTR lpKey, CString *szRetString)
{
	wchar_t wszData[1024] = {0,};

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\HingeDB.ini"));

	szRetString->Format(_T("%s"), wszData);
}

static void Read_HingeDB(LPCWSTR lpTitle, LPCWSTR lpKey, int *pRetValue)
{
	CString szModelPath;
	wchar_t wszData[100] = {0,};
	char szData[50] = {0,};

	// 모델 File 경로 설정
	*pRetValue = 0;

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T(".\\INI\\HingeDB.ini"));  

	wchar_To_char(wszData, szData);

	*pRetValue = atoi(szData);
}

