#pragma once

#include <direct.h>
#include <io.h>

class CIniProfileAPI
{
public:
	CIniProfileAPI(void);
	~CIniProfileAPI(void);
};

/////////////////////////////////////////////////////////////////////////////
static void ProcessMessage()
{
	MSG msg;
	//if (PeekMessage( &msg, 0, 0, 500, PM_REMOVE ) )
	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static void delayMs(DWORD delay)
{
	__int64 stTick, edTick;

	stTick = ::GetTickCount64();
	while (1)
	{
		edTick = ::GetTickCount64();
		if ((edTick - stTick) >= delay)
			break;

		ProcessMessage();
	}
}

/////////////////////////////////////////////////////////////////////////////
static int CStringSplit(CString stringValue, CString strToken, CString* pDesc)
{
	CString strTmp;
	int index = 0;
	int nPos = 0;

	// string 에 token 문자가 없으면 문자열 그대로 넘긴다.
	if (stringValue.Find(strToken) == -1)
	{
		pDesc[index++] = stringValue;
		return index;
	}

	int i = 0;
	while (1)
	{
		int oldI = i;
		i = stringValue.Find(strToken, i) + 1;
		if (i == 0)
		{
			if (stringValue.GetLength() >= oldI)
			{
				strTmp = stringValue.Mid(oldI, stringValue.GetLength() - oldI);
				pDesc[index++] = strTmp;
			}
			break;
}
		strTmp = stringValue.Mid(oldI, i - oldI - 1);
		pDesc[index++] = strTmp;
	}

	return index;
}

static int CStringSplit(CString stringValue, CString strToken, CStringArray* pDescArray)
{
	CString strTmp;
	int index = 0;
	int nPos = 0;

	// string 에 token 문자가 없으면 문자열 그대로 넘긴다.
	if (stringValue.Find(strToken) == -1)
	{
		pDescArray->Add(stringValue);
		return index;
	}

	int i = 0;
	while (1)
	{
		int oldI = i;
		i = stringValue.Find(strToken, i) + 1;
		if (i == 0)
		{
			if (stringValue.GetLength() >= oldI)
			{
				strTmp = stringValue.Mid(oldI, stringValue.GetLength() - oldI);
				pDescArray->Add(strTmp);
			}
			break;
		}
		strTmp = stringValue.Mid(oldI, i - oldI - 1);
		pDescArray->Add(strTmp);
	}

	return index;
}

/////////////////////////////////////////////////////////////////////////////
static int CALLBACK BrowseForFolder_CallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED)
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)lpData);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
static void Write_ProfileString(CString strFileName, CString strSection, CString strKey, CString strValue)
{
	::WritePrivateProfileString(strSection, strKey, strValue, strFileName);
}

static void Read_ProfileString(CString strFileName, CString strSection, CString strKey, CString* retValue)
{
	WCHAR wszData[512] = { 0, };
	::GetPrivateProfileString(strSection, strKey, 0, wszData, sizeof(wszData), strFileName);

	retValue->Format(_T("%s"), wszData);
}

static void Read_ProfileString(CString strFileName, CString strSection, CString strKey, int* retValue)
{
	WCHAR wszData[512] = { 0, };
	::GetPrivateProfileString(strSection, strKey, 0, wszData, sizeof(wszData), strFileName);

	*retValue = _ttoi(wszData);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{

	::WritePrivateProfileString(lpTitle, lpKey, lpValue, _T("./Config/Operation.ini"));
}

static void Write_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, int nData)
{
	CString szData;

	szData.Format(_T("%d"), nData);
	::WritePrivateProfileString(lpTitle, lpKey, szData, _T("./Config/Operation.ini"));
}

static void Write_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, float fData)
{
	CString szData;

	szData.Format(_T("%f"), fData);
	::WritePrivateProfileString(lpTitle, lpKey, szData, _T("./Config/Operation.ini"));
}

static void Read_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, int* pRetValue)
{
	wchar_t wszData[100] = { 0, };

	*pRetValue = 0;
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T("./Config/Operation.ini"));

	*pRetValue = _ttoi(wszData);
}

static void Read_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, float* pRetValue)
{
	wchar_t wszData[100] = { 0, };

	*pRetValue = 0;
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T("./Config/Operation.ini"));

	*pRetValue = (float)_tstof(wszData);
}

static void Read_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, CString* szRetString)
{
	wchar_t wszData[100] = { 0, };

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T("./Config/Operation.ini"));

	szRetString->Format(_T("%s"), wszData);
}

static void Read_SysIniFile(LPCWSTR lpTitle, LPCWSTR lpKey, TCHAR* wszRetString)
{
	wchar_t wszData[100] = { 0, };

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T("./Config/Operation.ini"));
	wcscpy_s(wszRetString, (wcslen(wszData) + 1), wszData);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void Write_ModelFile(LPCWSTR lpModelName, LPCWSTR lpSection, LPCWSTR lpKey, LPCWSTR lpValue)
{
	CString szModelPath;

	szModelPath.Format(_T(".\\Model\\%s.ini"), lpModelName);
	::WritePrivateProfileString(lpSection, lpKey, lpValue, szModelPath);
}

static void Write_ModelFile(LPCWSTR lpModelName, LPCWSTR lpSection, LPCWSTR lpKey, int ndata)
{
	CString szData;
	CString szModelPath;

	szData.Format(_T("%d"), ndata);
	szModelPath.Format(_T(".\\Model\\%s.ini"), lpModelName);
	::WritePrivateProfileString(lpSection, lpKey, szData, szModelPath);
}

static void Write_ModelFile(LPCWSTR lpModelName, LPCWSTR lpSection, LPCWSTR lpKey, long nData)
{
	CString szData;
	CString szModelPath;

	szData.Format(_T("%d"), nData);
	szModelPath.Format(_T(".\\Model\\%s.ini"), lpModelName);
	::WritePrivateProfileString(lpSection, lpKey, szData, szModelPath);
}

static void Write_ModelFile(LPCWSTR lpModelName, LPCWSTR lpSection, LPCWSTR lpKey, double fdata)
{
	CString szData;
	CString szModelPath;

	szData.Format(_T("%f"), fdata);
	szModelPath.Format(_T(".\\Model\\%s.ini"), lpModelName);
	::WritePrivateProfileString(lpSection, lpKey, szData, szModelPath);
}

static void Read_ModelFile(CString sModelName, LPCWSTR lpSection, LPCWSTR lpKey, CString* szRetString)
{
	TCHAR wszData[400] = { 0, };
	CString sModelPath;

	// 모델 File 경로 설정
	sModelPath.Format(_T(".\\Model\\%s.ini"), sModelName.GetString());

	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, sModelPath);

	szRetString->Format(_T("%s"), wszData);
}

static void Read_ModelFile(CString sModelName, LPCWSTR lpSection, LPCWSTR lpKey, int* pRetValue)
{
	CString sModelPath, sdata;
	TCHAR wszData[400] = { 0, };
	char szData[200] = { 0, };

	// 모델 File 경로 설정
	*pRetValue = 0;
	sModelPath.Format(_T(".\\Model\\%s.ini"), sModelName.GetString());

	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, sModelPath);

	*pRetValue = _ttoi(wszData);
}

static void Read_ModelFile(CString sModelName, LPCWSTR lpSection, LPCWSTR lpKey, LONG* pRetValue)
{
	CString sModelPath, sdata;
	TCHAR wszData[400] = { 0, };

	// 모델 File 경로 설정
	sModelPath.Format(_T(".\\Model\\%s.ini"), sModelName.GetString());

	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, sModelPath);

	*pRetValue = _tcstol(wszData, NULL, 10);
}

static void Read_ModelFile(CString sModelName, LPCWSTR lpSection, LPCWSTR lpKey, float* pRetValue)
{
	CString sModelPath, sdata;
	TCHAR wszData[400] = { 0, };
	char szData[200] = { 0, };

	// 모델 File 경로 설정
	sModelPath.Format(_T(".\\Model\\%s.ini"), sModelName.GetString());

	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, sModelPath);

	*pRetValue = (float)_tstof(wszData);
}

static void Read_ModelFile(CString sModelName, LPCWSTR lpSection, LPCWSTR lpKey, double* pRetValue)
{
	CString sModelPath, sdata;
	TCHAR wszData[400] = { 0, };

	// 모델 File 경로 설정
	sModelPath.Format(_T(".\\Model\\%s.ini"), sModelName.GetString());

	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, sModelPath);

	*pRetValue = (float)_tstof(wszData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_SummaryInfo(LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{

	::WritePrivateProfileString(lpTitle, lpKey, lpValue, _T("./Config/Summary.ini"));
}

static void Read_SummaryInfo(LPCWSTR lpTitle, LPCWSTR lpKey, CString* szRetString)
{
	wchar_t wszData[100] = { 0, };

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T("./Config/Summary.ini"));

	szRetString->Format(_T("%s"), wszData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_MesPIDInfo(LPCWSTR lpTitle, LPCWSTR lpKey, LPCWSTR lpValue)
{
	::WritePrivateProfileString(lpTitle, lpKey, lpValue, _T("./Config/MesPID_Info.ini"));
}

static void Read_MesPIDInfo(LPCWSTR lpTitle, LPCWSTR lpKey, CString* szRetString)
{
	wchar_t wszData[1000] = { 0, };

	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T("./Config/MesPID_Info.ini"));

	szRetString->Format(_T("%s"), wszData);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Write_MesStatusInfo(LPCWSTR lpTitle, LPCWSTR lpKey, int nValue)
{
	CString sdata;

	sdata.Format(_T("%d"), nValue);
	::WritePrivateProfileString(lpTitle, lpKey, sdata, _T("./Config/MesStatus.ini"));
}

static void Read_MesStatusInfo(LPCWSTR lpTitle, LPCWSTR lpKey, int* pRetValue)
{
	wchar_t wszData[100] = { 0, };

	*pRetValue = 0;
	::GetPrivateProfileString(lpTitle, lpKey, 0, wszData, sizeof(wszData), _T("./Config/MesStatus.ini"));

	*pRetValue = (char)_ttoi(wszData);
}
