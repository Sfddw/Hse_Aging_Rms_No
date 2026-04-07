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

static CString Make3DigitKey(int no)
{
	CString key;
	key.Format(_T("%03d"), no);
	return key;
}

static CString MakeModelListValue(const CString& fileName)
{
	// 예: "1_LP14W4AL.SPB-2.ini" -> "1_LP14W4ALSPB-2"
	CString baseName = fileName;

	int dot = baseName.ReverseFind(_T('.'));
	if (dot > 0)
		baseName = baseName.Left(dot); // 확장자 제거

	baseName.Replace(_T("."), _T("")); // 내부 점(.) 제거
	return baseName;
}

static CString NormalizeRecipeKey3Digit(const CString& recipeKey)
{
	CString text = recipeKey;
	text.Trim();

	if (text.Right(4).CompareNoCase(_T(".ini")) == 0)
		text = text.Left(text.GetLength() - 4);

	int no = _ttoi(text);
	CString key;
	key.Format(_T("%03d"), no);
	return key;
}

// 파일명에서 앞 숫자만 추출
static BOOL ExtractLeadingNumberFromFileName(const CString& fileName, int& outNo)
{
	// fileName 예: "2_model-spb2.ini", "013_LP190.ini", "04.TEST.ini"
	outNo = 0;

	CString base = fileName;
	base.Trim();

	// 확장자 제거(.ini)
	int dot = base.ReverseFind(_T('.'));
	if (dot > 0)
		base = base.Left(dot);

	// 맨 앞 연속 숫자 추출
	int i = 0;
	while (i < base.GetLength() && _istdigit(base[i])) i++;

	if (i == 0)
		return FALSE; // 앞에 숫자가 없으면 실패

	outNo = _ttoi(base.Left(i));
	return (outNo > 0);
}

/// <summary>
/// Model 폴더 숫자 <-> 원본 모델명 매핑
/// </summary>
/// <param name="modelDir"></param>
/// <param name="outMap"></param>
/// <returns></returns>
static BOOL BuildModelNumberNameMap(const CString& modelDir, CMapStringToString& outMap)
{
	outMap.RemoveAll();

	WIN32_FIND_DATA wfd = { 0 };
	HANDLE hFind = INVALID_HANDLE_VALUE;

	CString pattern;
	pattern.Format(_T("%s\\*.ini"), modelDir.GetString());

	hFind = FindFirstFile(pattern, &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
		return TRUE; // Model 폴더에 ini가 없어도 실패 아님

	do
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		CString fileName = wfd.cFileName;   // 예: "1_LP14W4AL.SPB-2.ini"
		int no = 0;
		if (!ExtractLeadingNumberFromFileName(fileName, no))
			continue;

		CString key = Make3DigitKey(no);            // "001"
		CString modelName = MakeModelListValue(fileName); // "1_LP14W4ALSPB-2"

		outMap.SetAt(key, modelName);

	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);
	return TRUE;
}

/// <summary>
/// Recipe 폴더의 숫자 ini를 읽어서 ModelList.ini 생성
/// </summary>
/// <param name="modelDir"></param>
/// <param name="recipeDir"></param>
/// <returns></returns>
static BOOL BuildModelListIniFromRecipeAndModel(
	const CString& modelDir,
	const CString& recipeDir,
	const CString& rmsRootDir)
{
	CMapStringToString modelMap;
	if (!BuildModelNumberNameMap(modelDir, modelMap))
		return FALSE;

	CString outPath;
	outPath.Format(_T("%s\\ModelList.ini"), rmsRootDir.GetString());

	CStdioFile f;
	if (!f.Open(outPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return FALSE;

	f.WriteString(_T("[ModelList]\n"));

	WIN32_FIND_DATA wfd = { 0 };
	HANDLE hFind = INVALID_HANDLE_VALUE;

	CString pattern;
	pattern.Format(_T("%s\\*.ini"), recipeDir.GetString());

	hFind = FindFirstFile(pattern, &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		f.Close();
		return TRUE;
	}

	CArray<int, int> recipeNos;

	do
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		CString fileName = wfd.cFileName; // 001.ini, 002.ini ...

		int no = 0;
		if (!ExtractLeadingNumberFromFileName(fileName, no))
			continue;

		BOOL exists = FALSE;
		for (INT_PTR i = 0; i < recipeNos.GetCount(); i++)
		{
			if (recipeNos[i] == no)
			{
				exists = TRUE;
				break;
			}
		}
		if (!exists)
			recipeNos.Add(no);

	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);

	// 숫자 오름차순 정렬
	for (INT_PTR i = 0; i < recipeNos.GetCount(); i++)
	{
		for (INT_PTR j = i + 1; j < recipeNos.GetCount(); j++)
		{
			if (recipeNos[i] > recipeNos[j])
			{
				int tmp = recipeNos[i];
				recipeNos[i] = recipeNos[j];
				recipeNos[j] = tmp;
			}
		}
	}

	for (INT_PTR i = 0; i < recipeNos.GetCount(); i++)
	{
		CString key = Make3DigitKey(recipeNos[i]); // "001"
		CString modelName;

		if (modelMap.Lookup(key, modelName))
		{
			CString line;
			line.Format(_T("%s=%s\n"), key.GetString(), modelName.GetString());
			f.WriteString(line);
		}
		else
		{
			CString line;
			line.Format(_T("%s=\n"), key.GetString());
			f.WriteString(line);
		}
	}

	f.Close();
	return TRUE;
}

/// Recipe 폴더 생성할지 안할지 
static BOOL EnsureDirectoryExists(LPCTSTR dirPath)
{
	// 이미 있으면 OK
	if (_waccess(dirPath, 0) == 0)
		return TRUE;

	// 없으면 생성
	if (_wmkdir(dirPath) == 0)
		return TRUE;

	return (_waccess(dirPath, 0) == 0);
}

/// <summary>
///  CurModel.ini / ModelList.ini 생성할지 안할지
/// </summary>
/// <param name="filePath"></param>
/// <param name="defaultContent"></param>
/// <returns></returns>
static BOOL EnsureTextFileExists(LPCTSTR filePath, LPCTSTR defaultContent = _T(""))
{
	// 파일이 이미 있으면 OK
	if (_access(CT2A(filePath), 0) == 0)
		return TRUE;

	CStdioFile f;
	if (!f.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return FALSE;

	if (defaultContent && defaultContent[0] != 0)
		f.WriteString(defaultContent);

	f.Close();
	return TRUE;
}

/// Recipe 상위폴더 생성 후 Recipe1,2,3,4,5,6 생성 후 CurModel, ModelList 생성
static BOOL InitRecipeSubFoldersAndBaseFiles(const CString& recipeRootDir, int recipeCount = 6)
{
	// 상위 Recipe 폴더 먼저 생성
	if (!EnsureDirectoryExists(recipeRootDir))
		return FALSE;

	for (int i = 1; i <= recipeCount; i++)
	{
		CString subDir;
		subDir.Format(_T("%s\\Recipe%d"), recipeRootDir.GetString(), i);

		// .\Recipe\Recipe1 ~ .\Recipe\Recipe6 생성
		if (!EnsureDirectoryExists(subDir))
			return FALSE;

		// 각 하위 폴더 안에 CurModel.ini, ModelList.ini 생성
		CString curModelIni, modelListIni;
		curModelIni.Format(_T("%s\\CurModel.ini"), subDir.GetString());
		modelListIni.Format(_T("%s\\ModelList.ini"), subDir.GetString());

		if (!EnsureTextFileExists(curModelIni, _T("[CurModel]\n")))
			return FALSE;

		if (!EnsureTextFileExists(modelListIni, _T("[ModelList]\n")))
			return FALSE;
	}

	return TRUE;
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

// Model -> Recipe 복사할 때 "번호.ini"로 저장
static BOOL CopyModelIniToRecipe_NumberOnly(const CString& modelDir, const CString& recipeDir, BOOL bOverwrite = TRUE)
{
	WIN32_FIND_DATA wfd = { 0 };
	HANDLE hFind = INVALID_HANDLE_VALUE;

	CString pattern;
	pattern.Format(_T("%s\\*.ini"), modelDir.GetString());

	hFind = FindFirstFile(pattern, &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
		return TRUE; // ini가 없어도 성공 처리

	do
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		CString srcName = wfd.cFileName;
		int no = 0;
		if (!ExtractLeadingNumberFromFileName(srcName, no))
			continue;

		CString srcPath, dstPath;
		srcPath.Format(_T("%s\\%s"), modelDir.GetString(), srcName);
		dstPath.Format(_T("%s\\%03d.ini"), recipeDir.GetString(), no); // 001.ini

		BOOL bFailIfExists = (bOverwrite ? FALSE : TRUE);

		if (!::CopyFile(srcPath, dstPath, bFailIfExists))
		{
			if (!bOverwrite && GetLastError() == ERROR_FILE_EXISTS)
				continue;

			FindClose(hFind);
			return FALSE;
		}

	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);
	return TRUE;
}

/// <summary>
/// Recipe1~Recipe6 생성/복사 함수에서 ModelList.ini 생성 연결
/// </summary>
/// <param name="modelDir"></param>
/// <param name="recipeRootDir"></param>
/// <param name="recipeCount"></param>
/// <param name="bCopyIniOverwrite"></param>
/// <param name="bBuildModelList"></param>
/// <returns></returns>
static BOOL InitRecipeSubFoldersAndCopyModelIni(
	const CString& modelDir,
	const CString& recipeRootDir,
	int recipeCount = 6,              // 여기서는 rack 개수 개념으로 사용
	BOOL bCopyIniOverwrite = TRUE,
	BOOL bBuildModelList = TRUE)
{
	if (!EnsureDirectoryExists(recipeRootDir))
		return FALSE;

	CString recipeDir;
	recipeDir.Format(_T("%s\\Recipe"), recipeRootDir.GetString());

	if (!EnsureDirectoryExists(recipeDir))
		return FALSE;

	// Model -> RMS\Recipe 로 001.ini 형식 복사
	if (!CopyModelIniToRecipe_NumberOnly(modelDir, recipeDir, bCopyIniOverwrite))
		return FALSE;

	// Parameter.ini 생성 (내용 없음)
	CString parameterIni;
	parameterIni.Format(_T("%s\\Parameter.ini"), recipeRootDir.GetString());
	if (!EnsureTextFileExists(parameterIni, _T("")))
		return FALSE;

	// RACK1~6 CurModel 파일 생성 (내용 없음)
	for (int i = 1; i <= recipeCount; i++)
	{
		CString rackCurModelIni;
		rackCurModelIni.Format(_T("%s\\RACK%dCurModel.ini"), recipeRootDir.GetString(), i);

		if (!EnsureTextFileExists(rackCurModelIni, _T("")))
			return FALSE;
	}

	// RMS\ModelList.ini 생성
	if (bBuildModelList)
	{
		if (!BuildModelListIniFromRecipeAndModel(modelDir, recipeDir, recipeRootDir))
			return FALSE;
	}
	else
	{
		CString modelListIni;
		modelListIni.Format(_T("%s\\ModelList.ini"), recipeRootDir.GetString());

		if (!EnsureTextFileExists(modelListIni, _T("[ModelList]\n")))
			return FALSE;
	}

	return TRUE;
}



// recipe ini 정리(sync delete)
static void DeleteRecipeIniNotInModel(const CString& modelDir, const CString& recipeDir)
{
	// 1) Model 폴더에 있는 ini 이름들을 set로 수집
	CMapStringToPtr modelSet;
	WIN32_FIND_DATA wfd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	CString pattern;
	pattern.Format(_T("%s\\*.ini"), modelDir.GetString());
	hFind = FindFirstFile(pattern, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// 파일명만 (예: 001_LP110WU3.ini)
				modelSet.SetAt(wfd.cFileName, (void*)1);
			}
		} while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}

	// 2) Recipe 폴더 ini들을 순회하며, Model에 없으면 삭제
	pattern.Format(_T("%s\\*.ini"), recipeDir.GetString());
	hFind = FindFirstFile(pattern, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				void* dummy = nullptr;
				if (!modelSet.Lookup(wfd.cFileName, dummy))
				{
					CString delPath;
					delPath.Format(_T("%s\\%s"), recipeDir.GetString(), wfd.cFileName);
					DeleteFile(delPath);
				}
			}
		} while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}
}



// Model 폴더의 ini 목록을 Recipe\ModelList.txt에 기록 (원하면 사용)
static BOOL BuildModelListIni(LPCTSTR modelDir, LPCTSTR outTxtPath)
{
	CString pattern;
	pattern.Format(_T("%s\\*.ini"), modelDir);

	WIN32_FIND_DATA fd = { 0 };
	HANDLE h = FindFirstFile(pattern, &fd);

	// 파일 열기 (ini 스타일로 새로 생성)
	CStdioFile f;
	if (!f.Open(outTxtPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
	{
		if (h != INVALID_HANDLE_VALUE) FindClose(h);
		return FALSE;
	}

	f.WriteString(_T("[ModelList]\n"));

	if (h == INVALID_HANDLE_VALUE)
	{
		f.Close();
		return TRUE;
	}

	FindClose(h);
	f.Close();
	return TRUE;
}

// Model 폴더의 *.ini 파일을 Recipe 폴더로 복사
static BOOL CopyModelIniToRecipe(LPCTSTR modelDir, LPCTSTR recipeDir, BOOL bOverwrite = TRUE)
{
	CString srcPattern;
	srcPattern.Format(_T("%s\\*.ini"), modelDir);

	WIN32_FIND_DATA fd = { 0 };
	HANDLE h = FindFirstFile(srcPattern, &fd);
	if (h == INVALID_HANDLE_VALUE)
	{
		// 복사할 ini가 없으면 TRUE로 처리(원하면 FALSE로 바꿔도 됨)
		return TRUE;
	}

	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		CString src, dst;
		src.Format(_T("%s\\%s"), modelDir, fd.cFileName);
		dst.Format(_T("%s\\%s"), recipeDir, fd.cFileName);

		// CopyFile의 3번째 인자: bFailIfExists
		// 덮어쓰기(bOverwrite=TRUE) => bFailIfExists=FALSE
		BOOL ok = CopyFile(src, dst, bOverwrite ? FALSE : TRUE);
		// 실패해도 다음 파일 계속(원하면 여기서 return FALSE 처리)
		(void)ok;

	} while (FindNextFile(h, &fd));

	FindClose(h);
	return TRUE;
}

// "Recipe 폴더 생성 + ini 복사 + txt 생성” 원샷 초기화
static BOOL InitRecipeFolderAndFiles(BOOL bCopyIniOverwrite = TRUE, BOOL bBuildModelList = TRUE)
{
	CString modelDir = _T(".\\Model");
	CString recipeRootDir = _T(".\\RMS");

	// .\Recipe\Recipe1 ~ .\Recipe\Recipe6 생성
	// 각 폴더에 Model ini들을 번호.ini 형식으로 복사
	// 그리고 CurModel.ini / ModelList.ini 생성
	if (!InitRecipeSubFoldersAndCopyModelIni(
		modelDir,
		recipeRootDir,
		6,
		bCopyIniOverwrite,
		bBuildModelList))
	{
		return FALSE;
	}

	return TRUE;
}

// CurModel.ini에 AgingStart 모델 추가
static void SaveCurModelIni_FromModelText(const CString& selectedText)
{
	// selectedText 예:
	//  - "02_LP110WU3"  / "02_LP110WU3.ini"
	//  - "04.TEST"      / "04.TEST.ini"
	//  - "LP110WU3" (번호 없는 케이스도 대비: 이 경우는 그냥 저장 스킵하거나, ModelList.ini에서 역검색 필요)

	CString text = selectedText;
	text.Trim();

	// 확장자 제거(.ini 등)
	int dotExt = text.ReverseFind(_T('.'));
	if (dotExt > 0)
	{
		CString ext = text.Mid(dotExt);
		ext.MakeLower();
		if (ext == _T(".ini"))
			text = text.Left(dotExt);
	}

	// 앞 2자리 key
	CString key;
	if (text.GetLength() >= 2)
		key = text.Left(2);

	// key가 숫자인지 간단 체크
	if (key.GetLength() != 2 || !_istdigit(key[0]) || !_istdigit(key[1]))
		return; // 번호 없으면 여기서 스킵(원하면 ModelList.ini 역검색 로직 추가 가능)

	// 구분자 '_' 또는 '.'
	int sep = text.Find(_T('_'));
	if (sep < 0) sep = text.Find(_T('.'));

	CString model;
	if (sep >= 0 && sep + 1 < text.GetLength())
		model = text.Mid(sep + 1);
	else
		model = (text.GetLength() > 2) ? text.Mid(2) : _T("");

	model.Trim();
	if (model.IsEmpty())
		return;

	// Recipe 폴더/파일 보장 (없으면 생성)
	// (이 부분은 너가 이미 만들어둔 EnsureDirectoryExists/EnsureTextFileExists 사용 가능)
	// 간단히 WinAPI로 처리:
	CreateDirectory(_T(".\\Recipe"), nullptr);

	// CurModel.ini에 저장
	::WritePrivateProfileString(_T("CurModel"), key, model, _T(".\\Recipe\\CurModel.ini"));
}

// CurModel.ini에 aging 종료 모델 삭제
static void RemoveCurModelIni_ByModelText(const CString& sModelName)
{
	CString text = sModelName;
	text.Trim();

	// 확장자 제거(.ini)
	if (text.Right(4).CompareNoCase(_T(".ini")) == 0)
		text = text.Left(text.GetLength() - 4);

	// key 추출: 앞 2자리
	CString key;
	if (text.GetLength() >= 2 && _istdigit(text[0]) && _istdigit(text[1]))
		key = text.Left(2);

	key.Trim();
	if (key.IsEmpty())
		return;

	// [CurModel]에서 key 삭제
	::WritePrivateProfileString(_T("CurModel"), key, nullptr, _T(".\\Recipe\\CurModel.ini"));
}

static void Read_RecipeFile(const CString& recipeKey, LPCWSTR lpSection, LPCWSTR lpKey, CString* szRetString)
{
	TCHAR wszData[400] = { 0, };
	CString path;
	CString key = NormalizeRecipeKey3Digit(recipeKey);

	path.Format(_T(".\\RMS\\Recipe\\%s.ini"), key.GetString());

	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, path);
	szRetString->Format(_T("%s"), wszData);
}

static void Read_RecipeFile(const CString& recipeKey, LPCWSTR lpSection, LPCWSTR lpKey, int* pRetValue)
{
	TCHAR wszData[400] = { 0, };
	CString path;
	CString key = NormalizeRecipeKey3Digit(recipeKey);

	path.Format(_T(".\\RMS\\Recipe\\%s.ini"), key.GetString());

	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, path);
	*pRetValue = _ttoi(wszData);
}

static void Read_RecipeFile(const CString& recipeKey, LPCWSTR lpSection, LPCWSTR lpKey, float* pRetValue)
{
	TCHAR wszData[400] = { 0, };
	CString path;
	CString key = NormalizeRecipeKey3Digit(recipeKey);

	path.Format(_T(".\\RMS\\Recipe\\%s.ini"), key.GetString());

	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, path);
	*pRetValue = (float)_tstof(wszData);
}

static void Write_RecipeFile(LPCWSTR lpRecipeFileName, LPCWSTR lpSection, LPCWSTR lpKey, LPCWSTR lpValue)
{
	CString szRecipePath;
	CString key = NormalizeRecipeKey3Digit(lpRecipeFileName);

	szRecipePath.Format(_T(".\\RMS\\Recipe\\%s.ini"), key.GetString());
	::WritePrivateProfileString(lpSection, lpKey, lpValue, szRecipePath);
}

static void Write_RecipeFile(LPCWSTR lpRecipeFileName, LPCWSTR lpSection, LPCWSTR lpKey, int nData)
{
	CString szData;
	CString szRecipePath;
	CString key = NormalizeRecipeKey3Digit(lpRecipeFileName);

	szData.Format(_T("%d"), nData);
	szRecipePath.Format(_T(".\\RMS\\Recipe\\%s.ini"), key.GetString());
	::WritePrivateProfileString(lpSection, lpKey, szData, szRecipePath);
}

// Recipy_YN이 Y일 때 CurModel값 가져오는 함수들
static void Write_RecipeFile(LPCWSTR lpRecipeFileName, LPCWSTR lpSection, LPCWSTR lpKey, float fData)
{
	CString szData;
	CString szRecipePath;
	CString key = NormalizeRecipeKey3Digit(lpRecipeFileName);

	szData.Format(_T("%.3f"), fData);
	szRecipePath.Format(_T(".\\RMS\\Recipe\\%s.ini"), key.GetString());
	::WritePrivateProfileString(lpSection, lpKey, szData, szRecipePath);
}

static void Read_IniFileByPath(const CString& filePath, LPCWSTR lpSection, LPCWSTR lpKey, int* pRetValue)
{
	TCHAR wszData[400] = { 0, };
	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, filePath);
	*pRetValue = _ttoi(wszData);
}

static void Read_IniFileByPath(const CString& filePath, LPCWSTR lpSection, LPCWSTR lpKey, float* pRetValue)
{
	TCHAR wszData[400] = { 0, };
	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, filePath);
	*pRetValue = (float)_tstof(wszData);
}

static void Read_IniFileByPath(const CString& filePath, LPCWSTR lpSection, LPCWSTR lpKey, CString* pRetValue)
{
	TCHAR wszData[400] = { 0, };
	::GetPrivateProfileString(lpSection, lpKey, 0, wszData, sizeof(wszData) / 2, filePath);
	*pRetValue = wszData;
}