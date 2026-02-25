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
		// ini가 없어도 섹션만 만들어두고 종료
		f.Close();
		return TRUE;
	}

	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		CString file = fd.cFileName;          // 예: "01_LP110WU3.ini" or "04.TEST.ini"
		CString base = file;

		// 확장자 .ini 제거
		int dotExt = base.ReverseFind(_T('.'));
		if (dotExt > 0)
			base = base.Left(dotExt);         // 예: "01_LP110WU3" or "04.TEST"

		// 앞 번호(2자리) 추출
		CString key;
		if (base.GetLength() >= 2)
			key = base.Left(2);               // "01", "04"...

		// 구분자 찾기: '_' 우선, 없으면 '.'
		int sep = base.Find(_T('_'));
		if (sep < 0)
			sep = base.Find(_T('.'));

		// 값(모델명) 추출
		CString value;
		if (sep >= 0 && sep + 1 < base.GetLength())
		{
			value = base.Mid(sep + 1);        // "LP110WU3", "TEST", "LP140_TEST"...
		}
		else
		{
			// 구분자가 없으면, 앞 2자리 이후 전체를 값으로(예: "01LP110WU3" 같은 케이스 대비)
			value = (base.GetLength() > 2) ? base.Mid(2) : _T("");
		}

		key.Trim(); value.Trim();

		// key가 숫자가 아닐 수도 있으니 간단 검증(원하면 더 강하게)
		// key가 비면 skip
		if (key.IsEmpty() || value.IsEmpty())
			continue;

		CString line;
		line.Format(_T("%s=%s\n"), key, value);
		f.WriteString(line);

	} while (FindNextFile(h, &fd));

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
	// 폴더들
	CString modelDir = _T(".\\Model");
	CString recipeDir = _T(".\\Recipe");

	if (!EnsureDirectoryExists(recipeDir))
		return FALSE;

	// ini 복사
	if (!CopyModelIniToRecipe(modelDir, recipeDir, bCopyIniOverwrite))
		return FALSE;

	// txt 생성
	CString curModelIni = _T(".\\Recipe\\CurModel.ini");
	CString modelListIni = _T(".\\Recipe\\ModelList.ini");

	// CurModel은 기본값 빈 파일(원하면 "01_LP110WU3" 같은 기본값 넣어도 됨)
	if (!EnsureTextFileExists(curModelIni, _T("[CurModel]")))
		return FALSE;

	if (bBuildModelList)
	{
		if (!BuildModelListIni(modelDir, modelListIni))
			return FALSE;
	}
	else
	{
		if (!EnsureTextFileExists(modelListIni, _T("[ModelList]\n")))
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