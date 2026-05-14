#include "pch.h"
#include "HseAging.h"

#include "atlbase.h"
#include "atlcom.h"
#include <string>

#include "CIMNetCommApp.h"

#include <vector>
#include <algorithm>
#include <map>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Message Receive Class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
const UINT SINK_ID = 1;

CString m_sReceiveMessage;
BOOL	m_nMessageReceiveFlag;


static _ATL_FUNC_INFO HandleTibRvMsgEvent = { CC_STDCALL, VT_EMPTY, 1, { VT_BSTR } };
static _ATL_FUNC_INFO HandleTibRvStateEvent = { CC_STDCALL, VT_EMPTY, 1, { VT_BSTR} };

// ===================== [추가 시작] EPDC DELETE_INFO -> Parameter.ini clear helper =====================

static BOOL IsWhiteSpaceChar_EPDC(TCHAR ch)
{
	return (ch == _T(' ') ||
		ch == _T('\t') ||
		ch == _T('\r') ||
		ch == _T('\n'));
}

static CString TrimCopy_EPDC(const CString& s)
{
	CString t = s;
	t.Trim();
	return t;
}

// DELETE_INFO 같이 중첩 [] 구조를 통째로 가져오기 위한 전용 함수
static CString ExtractFieldValueFullBracket_EPDC(const CString& msg, LPCTSTR keyWithEq)
{
	CString key(keyWithEq);

	int pos = msg.Find(key);
	if (pos < 0)
		return _T("");

	int start = pos + key.GetLength();
	if (start >= msg.GetLength())
		return _T("");

	while (start < msg.GetLength() && IsWhiteSpaceChar_EPDC(msg[start]))
		start++;

	if (start >= msg.GetLength())
		return _T("");

	if (msg[start] != _T('['))
	{
		int end = start;
		while (end < msg.GetLength() && !IsWhiteSpaceChar_EPDC(msg[end]))
			end++;

		return msg.Mid(start, end - start);
	}

	int depth = 0;
	for (int i = start; i < msg.GetLength(); ++i)
	{
		if (msg[i] == _T('['))
			++depth;
		else if (msg[i] == _T(']'))
		{
			--depth;
			if (depth == 0)
				return msg.Mid(start, i - start + 1);
		}
	}

	return msg.Mid(start);
}

static CString MapEpdcDeleteNameToIniSection(const CString& paramName) // EPDC
{
	// RMS의 MODEL_NB는 Parameter.ini의 MODEL_NUMBER 섹션과 동일하게 처리
	/*if (paramName.CompareNoCase(_T("MODEL_NB")) == 0)
		return _T("MODEL_NUMBER");*/

	// 나머지는 현재 장비 기준으로 섹션명과 동일하다고 가정
	return paramName;
}

// EPPR
static BOOL ReadModelInfoKeysFromRackRecipeIni(
	const CString& rackRecipeIniPath,
	std::vector<CString>& outKeys,
	CString& outErrMsg)
{
	outKeys.clear();
	outErrMsg.Empty();

	if (!FileExistsSimple(rackRecipeIniPath))
	{
		outErrMsg.Format(_T("Rack recipe ini not found. file=%s"), rackRecipeIniPath.GetString());
		return FALSE;
	}

	const DWORD BUF_SIZE = 64 * 1024;
	std::vector<TCHAR> buf(BUF_SIZE, 0);

	DWORD read = ::GetPrivateProfileSection(
		_T("MODEL_INFO"),
		buf.data(),
		(DWORD)buf.size(),
		rackRecipeIniPath
	);

	if (read == 0)
	{
		outErrMsg.Format(_T("MODEL_INFO section not found or empty. file=%s"), rackRecipeIniPath.GetString());
		return FALSE;
	}

	const TCHAR* p = buf.data();

	while (*p)
	{
		CString line = p;

		int eqPos = line.Find(_T('='));
		if (eqPos > 0)
		{
			CString key = line.Left(eqPos);
			key.Trim();

			if (!key.IsEmpty())
				outKeys.push_back(key);
		}

		p += _tcslen(p) + 1;
	}

	return TRUE;
}

/// <summary>
/// EPPR ini 값 읽는 함수
/// </summary>
/// <param name="iniPath"></param>
/// <param name="key"></param>
/// <returns></returns>
static CString ReadModelInfoValue(
	const CString& iniPath,
	const CString& key)
{
	TCHAR value[512] = { 0 };

	::GetPrivateProfileString(
		_T("MODEL_INFO"),
		key,
		_T(""),
		value,
		_countof(value),
		iniPath
	);

	CString ret = value;
	ret.Trim();

	return ret;
}

// [EPSC] COMMAND_CODE = I 조회 함수
static BOOL ReadEpscParamNamesFromRackRecipeIni(
	const CString& rackRecipeIniPath,
	std::vector<CString>& outParamNames,
	CString& outErrMsg)
{
	outParamNames.clear();
	outErrMsg.Empty();

	if (!FileExistsSimple(rackRecipeIniPath))
	{
		outErrMsg.Format(_T("Rack recipe file not found. file=%s"), rackRecipeIniPath.GetString());
		return FALSE;
	}

	const DWORD BUF_SIZE = 64 * 1024;
	std::vector<TCHAR> buf(BUF_SIZE, 0);

	DWORD read = ::GetPrivateProfileSection(
		_T("MODEL_INFO"),
		buf.data(),
		(DWORD)buf.size(),
		rackRecipeIniPath
	);

	if (read == 0)
	{
		outErrMsg.Format(_T("MODEL_INFO section is empty. file=%s"), rackRecipeIniPath.GetString());
		return FALSE;
	}

	const TCHAR* p = buf.data();

	while (*p)
	{
		CString line = p;

		int eqPos = line.Find(_T('='));
		if (eqPos > 0)
		{
			CString key = line.Left(eqPos);
			key.Trim();

			if (!key.IsEmpty())
				outParamNames.push_back(key);
		}

		p += _tcslen(p) + 1;
	}

	return TRUE;
}

// EPPR recipeMsgSet 생성 함수추가
static BOOL BuildEpprRecipeMsgSetFromIni(
	const CString& rackRecipeIniPath,
	const CString& valueSourceIniPath,
	CString& outRecipeMsgSet,
	int& outParamCount,
	CString& outErrMsg)
{
	outRecipeMsgSet.Empty();
	outParamCount = 0;
	outErrMsg.Empty();

	if (!FileExistsSimple(valueSourceIniPath))
	{
		outErrMsg.Format(_T("Value source ini not found. file=%s"), valueSourceIniPath.GetString());
		return FALSE;
	}

	std::vector<CString> keys;

	if (!ReadModelInfoKeysFromRackRecipeIni(rackRecipeIniPath, keys, outErrMsg))
		return FALSE;

	for (size_t i = 0; i < keys.size(); ++i)
	{
		CString key = keys[i];
		key.Trim();

		if (key.IsEmpty())
			continue;

		CString value = ReadModelInfoValue(valueSourceIniPath, key);

		CString one;

		if (key.CompareNoCase(_T("MODEL_NB")) == 0)
		{
			one.Format(_T("%s$%s;"),
				key.GetString(),
				value.GetString());
		}
		else
		{
			one.Format(_T("%s_MODEL_INFO$%s;"),
				key.GetString(),
				value.GetString());
		}

		outRecipeMsgSet += one;
		outParamCount++;
	}

	return TRUE;
}

// [EPPR] 현재 Recipe 번호 읽는 함수 추가
static int ReadRecipeNoFromModelInfoIni(const CString& iniPath)
{
	CString value;

	value = ReadModelInfoValue(iniPath, _T("MODEL_NB"));
	int recipeNo = _ttoi(value);

	if (recipeNo > 0)
		return recipeNo;

	value = ReadModelInfoValue(iniPath, _T("MODEL_NUMBER"));
	recipeNo = _ttoi(value);

	return recipeNo;
}

static BOOL IniSectionExistsSimple(const CString& iniPath, const CString& section)
{
	TCHAR buf[4] = { 0 };

	DWORD read = ::GetPrivateProfileSection(
		section,
		buf,
		_countof(buf),
		iniPath
	);

	return read > 0;
}

static CString ReadIniValueSimple(const CString& iniPath, const CString& section, const CString& key)
{
	TCHAR buf[512] = { 0 };
	::GetPrivateProfileString(section, key, _T(""), buf, _countof(buf), iniPath);
	return CString(buf);
}

static void AppendEpscSettingItemFromIni(
	CString& outMsg,
	const CString& parameterIniPath,
	const CString& displayName,
	const CString& iniSection)
{
	static const LPCTSTR kRmsFields[] =
	{
		_T("PINDEX"),
		_T("SPECIAL_ITEM"),
		_T("ADDRESS"),
		_T("PARA_OFFSET"),
		_T("CURRENT_ADDRESS"),
		_T("UNIT_TYPE"),
		_T("WORD_SIZE"),
		_T("DECIMAL_PLACE"),
		_T("SIGN_YN"),
		_T("SNDPOS"),
		_T("FILE_PATH"),
		_T("FILE_NAME"),
		_T("INTERNAL_PARA_NAME"),
		_T("DELIMITER"),
		_T("ROW_NUM"),
		_T("COL_NUM"),
		_T("BIT_LENGTH")
	};

	if (!outMsg.IsEmpty())
		outMsg += _T(",");

	outMsg.AppendFormat(_T(":%s:["), displayName.GetString());

	for (int i = 0; i < _countof(kRmsFields); ++i)
	{
		CString val = ReadIniValueSimple(parameterIniPath, iniSection, kRmsFields[i]);

		outMsg.AppendFormat(_T("%s#%s^"), kRmsFields[i], val.GetString());
	}

	outMsg += _T("]");
}

// RACK_Recipe.ini 기준으로 EPSC SETTING_INFO 만드는 함수
static BOOL BuildEpscSettingInfoFromRackFiles(
	const CString& rackParameterIniPath,
	const CString& rackRecipeIniPath,
	CString& outSettingInfo,
	int& outParaCount,
	CString& outErrMsg)
{
	outSettingInfo.Empty();
	outParaCount = 0;
	outErrMsg.Empty();

	if (!FileExistsSimple(rackParameterIniPath))
	{
		outErrMsg.Format(_T("Rack parameter file not found. file=%s"), rackParameterIniPath.GetString());
		return FALSE;
	}

	if (!FileExistsSimple(rackRecipeIniPath))
	{
		outErrMsg.Format(_T("Rack recipe file not found. file=%s"), rackRecipeIniPath.GetString());
		return FALSE;
	}

	std::vector<CString> paramNames;

	if (!ReadEpscParamNamesFromRackRecipeIni(rackRecipeIniPath, paramNames, outErrMsg))
		return FALSE;

	for (size_t i = 0; i < paramNames.size(); ++i)
	{
		CString paramName = paramNames[i];
		paramName.Trim();

		if (paramName.IsEmpty())
			continue;

		// RACKn_Recipe.ini에는 있는데 RACKn_Parameter.ini에 섹션이 없으면 응답에서 제외
		// S 메시지로 정상 추가된 항목이면 두 파일에 모두 존재해야 함
		if (!IniSectionExistsSimple(rackParameterIniPath, paramName))
			continue;

		AppendEpscSettingItemFromIni(
			outSettingInfo,
			rackParameterIniPath,
			paramName,  // 표시 이름
			paramName   // ini section 이름
		);

		outParaCount++;
	}

	return TRUE;
}

static void SplitDeleteNamesByCaret(const CString& src, std::vector<CString>& outNames)
{
	CString cur;

	for (int i = 0; i < src.GetLength(); ++i)
	{
		TCHAR ch = src[i];

		if (ch == _T('^'))
		{
			cur.Trim();
			if (!cur.IsEmpty())
				outNames.push_back(cur);
			cur.Empty();
		}
		else
		{
			cur.AppendChar(ch);
		}
	}

	cur.Trim();
	if (!cur.IsEmpty())
		outNames.push_back(cur);
}

static BOOL ParseEpdcDeleteInfo(const CString& deleteInfoRaw, std::vector<CString>& outNames)
{
	outNames.clear();

	CString text = TrimCopy_EPDC(deleteInfoRaw);
	if (text.IsEmpty())
		return TRUE;

	// 바깥 [] 제거: [:[MODEL_NB^DIMMING_SEL]] -> :[MODEL_NB^DIMMING_SEL]
	if (text.GetLength() >= 2 && text[0] == _T('[') && text[text.GetLength() - 1] == _T(']'))
		text = text.Mid(1, text.GetLength() - 2);

	text.Trim();
	if (text.IsEmpty())
		return TRUE;

	int searchPos = 0;
	bool foundBlock = false;

	while (true)
	{
		int start = text.Find(_T(":["), searchPos);
		if (start < 0)
			break;

		int end = text.Find(_T(']'), start + 2);
		if (end < 0)
			break;

		CString block = text.Mid(start + 2, end - (start + 2)); // MODEL_NB^DIMMING_SEL
		SplitDeleteNamesByCaret(block, outNames);

		foundBlock = true;
		searchPos = end + 1;
	}

	// 혹시 :[ ] 형식 없이 그냥 MODEL_NB^DIMMING_SEL 형태로 들어오면 fallback
	if (!foundBlock)
	{
		SplitDeleteNamesByCaret(text, outNames);
	}

	// trim + 중복 제거
	std::vector<CString> cleaned;
	for (size_t i = 0; i < outNames.size(); ++i)
	{
		CString one = TrimCopy_EPDC(outNames[i]);
		if (one.IsEmpty())
			continue;

		bool exists = false;
		for (size_t j = 0; j < cleaned.size(); ++j)
		{
			if (cleaned[j].CompareNoCase(one) == 0)
			{
				exists = true;
				break;
			}
		}

		if (!exists)
			cleaned.push_back(one);
	}

	outNames.swap(cleaned);
	return TRUE;
}

static BOOL ClearParaListValuesInSection(const CString& parameterIniPath, const CString& section, CString& outErrMsg)
{
	static const LPCTSTR kParaKeys[] =
	{
		_T("PINDEX"),
		_T("SPECIAL_ITEM"),
		_T("ADDRESS"),
		_T("PARA_OFFSET"),
		_T("CURRENT_ADDRESS"),
		_T("UNIT_TYPE"),
		_T("WORD_SIZE"),
		_T("DECIMAL_PLACE"),
		_T("SIGN_YN"),
		_T("SNDPOS"),
		_T("FILE_PATH"),
		_T("FILE_NAME"),
		_T("INTERNAL_PARA_NAME"),
		_T("DELIMITER"),
		_T("ROW_NUM"),
		_T("COL_NUM"),
		_T("BIT_LENGTH")
	};

	for (int i = 0; i < _countof(kParaKeys); ++i)
	{
		if (!::WritePrivateProfileString(section, kParaKeys[i], _T(""), parameterIniPath))
		{
			outErrMsg.Format(_T("Clear failed. section=%s key=%s"),
				section.GetString(),
				kParaKeys[i]);
			return FALSE;
		}
	}

	return TRUE;
}

/// <summary>
/// EPDC의 섹션 및 키 삭제
/// </summary>
/// <param name="iniPath"></param>
/// <param name="keyName"></param>
/// <param name="outDeletedKeyCount"></param>
/// <param name="outErrMsg"></param>
/// <returns></returns>
static BOOL DeleteKeyFromAllSections(
	const CString& iniPath,
	const CString& keyName,
	int& outDeletedKeyCount,
	CString& outErrMsg)
{
	outDeletedKeyCount = 0;

	CString key = keyName;
	key.Trim();

	if (key.IsEmpty())
		return TRUE;

	const DWORD BUF_SIZE = 64 * 1024;
	std::vector<TCHAR> sectionBuf(BUF_SIZE, 0);

	DWORD n = ::GetPrivateProfileSectionNames(
		sectionBuf.data(),
		(DWORD)sectionBuf.size(),
		iniPath
	);

	if (n == 0)
		return TRUE;

	const TCHAR* p = sectionBuf.data();

	while (*p)
	{
		CString section = p;

		TCHAR valueBuf[1024] = { 0 };
		const TCHAR* marker = _T("__EPDC_KEY_NOT_FOUND__");

		::GetPrivateProfileString(
			section,
			key,
			marker,
			valueBuf,
			_countof(valueBuf),
			iniPath
		);

		if (_tcscmp(valueBuf, marker) != 0)
		{
			if (!::WritePrivateProfileString(section, key, NULL, iniPath))
			{
				outErrMsg.Format(
					_T("Delete key failed. file=%s section=%s key=%s"),
					iniPath.GetString(),
					section.GetString(),
					key.GetString()
				);
				return FALSE;
			}

			outDeletedKeyCount++;
		}

		p += _tcslen(p) + 1;
	}

	return TRUE;
}

static BOOL DeleteIniSectionWhole(
	const CString& iniPath,
	const CString& section,
	CString& outErrMsg)
{
	CString sec = section;
	sec.Trim();

	if (sec.IsEmpty())
		return TRUE;

	if (!::WritePrivateProfileString(sec, NULL, NULL, iniPath))
	{
		outErrMsg.Format(
			_T("Delete section failed. file=%s section=%s"),
			iniPath.GetString(),
			sec.GetString()
		);
		return FALSE;
	}

	return TRUE;
}

static BOOL DeleteParameterFromRecipeIni(
	const CString& recipeIniPath,
	const CString& paramName,
	int& outDeletedCount,
	CString& outErrMsg)
{
	CString name = paramName;
	name.Trim();

	if (name.IsEmpty())
		return TRUE;

	// 1) [PARAM_NAME] 섹션 전체 삭제
	if (!DeleteIniSectionWhole(recipeIniPath, name, outErrMsg))
		return FALSE;

	outDeletedCount++;

	// 2) 모든 섹션 안의 PARAM_NAME= key 삭제
	int deletedKeyCount = 0;
	if (!DeleteKeyFromAllSections(recipeIniPath, name, deletedKeyCount, outErrMsg))
		return FALSE;

	outDeletedCount += deletedKeyCount;

	return TRUE;
}

static BOOL ApplyEpdcDeleteInfoToParameterAndRecipeIni(
	const CString& parameterIniPath,
	const CString& recipeIniPath,
	const CString& deleteInfoRaw,
	int& outDeletedCount,
	CString& outErrMsg)
{
	outDeletedCount = 0;
	outErrMsg.Empty();

	std::vector<CString> names;
	if (!ParseEpdcDeleteInfo(deleteInfoRaw, names))
	{
		outErrMsg = _T("ParseEpdcDeleteInfo failed");
		return FALSE;
	}

	for (size_t i = 0; i < names.size(); ++i)
	{
		CString paramName = MapEpdcDeleteNameToIniSection(names[i]);
		paramName.Trim();

		if (paramName.IsEmpty())
			continue;

		// 1) RACKnParameter.ini 에서 [PARAM_NAME] 섹션 전체 삭제
		if (!DeleteIniSectionWhole(parameterIniPath, paramName, outErrMsg))
			return FALSE;

		outDeletedCount++;

		// 2) 현재 rack에 매칭된 Recipe\xxx.ini 에서 같은 파라미터 삭제
		int recipeDeletedCount = 0;
		if (!DeleteParameterFromRecipeIni(recipeIniPath, paramName, recipeDeletedCount, outErrMsg))
			return FALSE;

		outDeletedCount += recipeDeletedCount;
	}

	return TRUE;
}

static BOOL DeleteParameterFromRackRecipeIni(
	const CString& rackRecipeIniPath,
	const CString& paramName,
	int& outDeletedCount,
	CString& outErrMsg)
{
	CString name = paramName;
	name.Trim();

	if (name.IsEmpty())
		return TRUE;

	// 1) RACKn_Recipe.ini의 [MODEL_INFO] 안에서 key 삭제
	if (!::WritePrivateProfileString(
		_T("MODEL_INFO"),
		name,
		NULL,
		rackRecipeIniPath))
	{
		outErrMsg.Format(
			_T("Delete recipe key failed. file=%s section=MODEL_INFO key=%s"),
			rackRecipeIniPath.GetString(),
			name.GetString()
		);
		return FALSE;
	}

	outDeletedCount++;

	// 2) 혹시 [PARAM_NAME] 섹션 형태로 들어간 경우도 대비해서 섹션 삭제
	::WritePrivateProfileString(
		name,
		NULL,
		NULL,
		rackRecipeIniPath
	);

	return TRUE;
}

static BOOL ApplyEpdcDeleteInfoToRackFiles(
	const CString& rackParameterIniPath,
	const CString& rackRecipeIniPath,
	const CString& deleteInfoRaw,
	int& outDeletedCount,
	CString& outErrMsg)
{
	outDeletedCount = 0;
	outErrMsg.Empty();

	std::vector<CString> names;

	if (!ParseEpdcDeleteInfo(deleteInfoRaw, names))
	{
		outErrMsg = _T("ParseEpdcDeleteInfo failed");
		return FALSE;
	}

	for (size_t i = 0; i < names.size(); ++i)
	{
		CString paramName = MapEpdcDeleteNameToIniSection(names[i]);
		paramName.Trim();

		if (paramName.IsEmpty())
			continue;

		// 1) RACKn_Parameter.ini에서 [PARAM_NAME] 섹션 전체 삭제
		if (!::WritePrivateProfileString(
			paramName,
			NULL,
			NULL,
			rackParameterIniPath))
		{
			outErrMsg.Format(
				_T("Delete parameter section failed. file=%s section=%s"),
				rackParameterIniPath.GetString(),
				paramName.GetString()
			);
			return FALSE;
		}

		outDeletedCount++;

		// 2) RACKn_Recipe.ini에서 [MODEL_INFO] 안의 PARAM_NAME key 삭제
		int recipeDeletedCount = 0;
		if (!DeleteParameterFromRackRecipeIni(
			rackRecipeIniPath,
			paramName,
			recipeDeletedCount,
			outErrMsg))
		{
			return FALSE;
		}

		outDeletedCount += recipeDeletedCount;
	}

	return TRUE;
}

// EPSC 파싱용 구조체 추가
struct RMS_EPSC_SUBITEM
{
	CString key;
	CString value;
};

struct RMS_EPSC_PARAM
{
	CString paramName;
	std::vector<RMS_EPSC_SUBITEM> subItems;
};

static CString TrimCopy_RmsEpsc(const CString& src)
{
	CString s = src;
	s.Trim();
	return s;
}

static void ParseEpscSubItems(
	const CString& block,
	std::vector<RMS_EPSC_SUBITEM>& outSubItems)
{
	outSubItems.clear();

	CString text = block;
	text.Trim();

	int start = 0;

	while (start <= text.GetLength())
	{
		int caret = text.Find(_T('^'), start);

		CString token;
		if (caret >= 0)
		{
			token = text.Mid(start, caret - start);
			start = caret + 1;
		}
		else
		{
			token = text.Mid(start);
			start = text.GetLength() + 1;
		}

		token.Trim();
		if (token.IsEmpty())
			continue;

		int sharp = token.Find(_T('#'));
		if (sharp < 0)
			continue;

		RMS_EPSC_SUBITEM item;
		item.key = token.Left(sharp);
		item.value = token.Mid(sharp + 1);

		item.key.Trim();
		item.value.Trim();

		if (!item.key.IsEmpty())
			outSubItems.push_back(item);
	}
}

static BOOL ParseEpscSettingInfoForUpsert(
	const CString& settingInfoRaw,
	std::vector<RMS_EPSC_PARAM>& outParams,
	CString& outErrMsg)
{
	outParams.clear();
	outErrMsg.Empty();

	CString text = settingInfoRaw;
	text.Trim();

	if (text.IsEmpty())
		return TRUE;

	// 바깥 [] 제거
	if (text.GetLength() >= 2 &&
		text[0] == _T('[') &&
		text[text.GetLength() - 1] == _T(']'))
	{
		text = text.Mid(1, text.GetLength() - 2);
	}

	text.Trim();

	int pos = 0;

	while (pos < text.GetLength())
	{
		// 다음 :PARAM:[ 찾기
		int colon = text.Find(_T(':'), pos);
		if (colon < 0)
			break;

		int mark = text.Find(_T(":["), colon + 1);
		if (mark < 0)
			break;

		CString paramName = text.Mid(colon + 1, mark - (colon + 1));
		paramName.Trim();

		int blockStart = mark + 2;
		int blockEnd = text.Find(_T(']'), blockStart);
		if (blockEnd < 0)
		{
			outErrMsg.Format(_T("SETTING_INFO parse failed. param=%s"), paramName.GetString());
			return FALSE;
		}

		CString block = text.Mid(blockStart, blockEnd - blockStart);

		RMS_EPSC_PARAM param;
		param.paramName = paramName;
		param.paramName.Trim();

		ParseEpscSubItems(block, param.subItems);

		if (!param.paramName.IsEmpty())
			outParams.push_back(param);

		pos = blockEnd + 1;
	}

	return TRUE;
}

static BOOL IniKeyExistsSimple(
	const CString& iniPath,
	const CString& section,
	const CString& key)
{
	TCHAR value[1024] = { 0 };
	const TCHAR* marker = _T("__RMS_KEY_NOT_FOUND__");

	::GetPrivateProfileString(
		section,
		key,
		marker,
		value,
		_countof(value),
		iniPath
	);

	return (_tcscmp(value, marker) != 0);
}

static BOOL EnsureRackRecipeParameterKey(
	const CString& rackRecipeIniPath,
	const CString& paramName,
	CString& outErrMsg)
{
	CString name = paramName;
	name.Trim();

	if (name.IsEmpty())
		return TRUE;

	// 파일이 없으면 [MODEL_INFO]만 가진 파일로 생성
	if (!FileExistsSimple(rackRecipeIniPath))
	{
		if (!EnsureTextFileExists(rackRecipeIniPath, _T("[MODEL_INFO]\n")))
		{
			outErrMsg.Format(_T("Create rack recipe file failed. file=%s"),
				rackRecipeIniPath.GetString());
			return FALSE;
		}
	}

	// 이미 있으면 그대로 둠
	if (IniKeyExistsSimple(rackRecipeIniPath, _T("MODEL_INFO"), name))
		return TRUE;

	// 없으면 PARAM_NAME= 형태로 추가
	if (!::WritePrivateProfileString(
		_T("MODEL_INFO"),
		name,
		_T(""),
		rackRecipeIniPath))
	{
		outErrMsg.Format(
			_T("Add recipe parameter failed. file=%s key=%s"),
			rackRecipeIniPath.GetString(),
			name.GetString()
		);
		return FALSE;
	}

	return TRUE;
}

static BOOL ApplyEpscSettingInfoToRackFiles(
	const CString& rackParameterIniPath,
	const CString& rackRecipeIniPath,
	const CString& settingInfoRaw,
	int& outAppliedCount,
	CString& outErrMsg)
{
	outAppliedCount = 0;
	outErrMsg.Empty();

	std::vector<RMS_EPSC_PARAM> params;

	if (!ParseEpscSettingInfoForUpsert(settingInfoRaw, params, outErrMsg))
		return FALSE;

	// 파일이 없으면 최소 파일 생성
	if (!FileExistsSimple(rackParameterIniPath))
	{
		if (!EnsureTextFileExists(rackParameterIniPath, _T("")))
		{
			outErrMsg.Format(_T("Create rack parameter file failed. file=%s"),
				rackParameterIniPath.GetString());
			return FALSE;
		}
	}

	if (!FileExistsSimple(rackRecipeIniPath))
	{
		if (!EnsureTextFileExists(rackRecipeIniPath, _T("[MODEL_INFO]\n")))
		{
			outErrMsg.Format(_T("Create rack recipe file failed. file=%s"),
				rackRecipeIniPath.GetString());
			return FALSE;
		}
	}

	for (size_t i = 0; i < params.size(); ++i)
	{
		CString paramName = params[i].paramName;
		paramName.Trim();

		if (paramName.IsEmpty())
			continue;

		// 1) RACKn_Recipe.ini에 PARAM_NAME= 없으면 추가
		if (!EnsureRackRecipeParameterKey(rackRecipeIniPath, paramName, outErrMsg))
			return FALSE;

		// 2) RACKn_Parameter.ini의 [PARAM_NAME] 아래 소항목 추가/수정
		for (size_t j = 0; j < params[i].subItems.size(); ++j)
		{
			CString key = params[i].subItems[j].key;
			CString value = params[i].subItems[j].value;

			key.Trim();
			value.Trim();

			if (key.IsEmpty())
				continue;

			if (!::WritePrivateProfileString(
				paramName,
				key,
				value,
				rackParameterIniPath))
			{
				outErrMsg.Format(
					_T("Write parameter failed. file=%s section=%s key=%s value=%s"),
					rackParameterIniPath.GetString(),
					paramName.GetString(),
					key.GetString(),
					value.GetString()
				);
				return FALSE;
			}
		}

		outAppliedCount++;
	}

	return TRUE;
}

// ===================== [추가 끝] EPDC DELETE_INFO -> Parameter.ini clear helper =====================

/// <summary>
/// CurModel.ini에서 현재 Recipe 번호를 읽어와서 recipe.ini 경로 만듬
/// </summary>
/// <param name="text"></param>
/// <param name="outRecipeNo"></param>
/// <returns></returns>
static BOOL ParseLeadingRecipeNo(const CString& text, int& outRecipeNo)
{
	outRecipeNo = 0;

	CString temp = text;
	temp.Trim();

	if (temp.IsEmpty())
		return FALSE;

	int dot = temp.ReverseFind(_T('.'));
	if (dot > 0)
		temp = temp.Left(dot);

	int i = 0;
	while (i < temp.GetLength() && _istdigit(temp[i]))
		i++;

	if (i <= 0)
		return FALSE;

	outRecipeNo = _ttoi(temp.Left(i));
	return outRecipeNo > 0;
}

static BOOL TryReadRecipeNoFromSection(
	const CString& iniPath,
	const CString& section,
	int& outRecipeNo)
{
	static const LPCTSTR keys[] =
	{
		_T("MODEL_NB"),
		_T("MODEL_NUMBER"),
		_T("RECIPE_NO"),
		_T("MODEL_NO"),
		_T("CURRENT_RECIPE"),
		_T("RECIPE"),
		_T("MODEL"),
		_T("CUR_MODEL")
	};

	for (int i = 0; i < _countof(keys); ++i)
	{
		TCHAR value[512] = { 0 };

		::GetPrivateProfileString(
			section,
			keys[i],
			_T(""),
			value,
			_countof(value),
			iniPath
		);

		CString text = value;
		text.Trim();

		if (text.IsEmpty())
			continue;

		if (ParseLeadingRecipeNo(text, outRecipeNo))
			return TRUE;
	}

	return FALSE;
}

static BOOL GetCurrentRecipeNoByRack(
	int rackNo,
	int& outRecipeNo,
	CString& outErrMsg)
{
	outRecipeNo = 0;
	outErrMsg.Empty();

	if (rackNo < 1 || rackNo > 6)
	{
		outErrMsg.Format(_T("Invalid rackNo. rackNo=%d"), rackNo);
		return FALSE;
	}

	CString curModelIni;
	curModelIni.Format(_T(".\\RMS\\RACK%dCurModel.ini"), rackNo);

	if (!FileExistsSimple(curModelIni))
	{
		outErrMsg.Format(_T("CurModel file not found. file=%s"), curModelIni.GetString());
		return FALSE;
	}

	// 1순위: [MODEL_INFO]에서 MODEL_NB / MODEL_NUMBER 읽기
	if (TryReadRecipeNoFromSection(curModelIni, _T("MODEL_INFO"), outRecipeNo))
		return TRUE;

	// 2순위: [CurModel]에서 읽기
	if (TryReadRecipeNoFromSection(curModelIni, _T("CurModel"), outRecipeNo))
		return TRUE;

	// 3순위: 혹시 섹션명이 다를 경우 전체 섹션을 순회하며 fallback
	const DWORD SECTION_BUF_SIZE = 64 * 1024;
	std::vector<TCHAR> sectionBuf(SECTION_BUF_SIZE, 0);

	DWORD sectionLen = ::GetPrivateProfileSectionNames(
		sectionBuf.data(),
		(DWORD)sectionBuf.size(),
		curModelIni
	);

	if (sectionLen > 0)
	{
		const TCHAR* pSection = sectionBuf.data();

		while (*pSection)
		{
			CString section = pSection;

			if (TryReadRecipeNoFromSection(curModelIni, section, outRecipeNo))
				return TRUE;

			pSection += _tcslen(pSection) + 1;
		}
	}

	outErrMsg.Format(
		_T("Current recipe no not found. file=%s, MODEL_NB/MODEL_NUMBER is empty"),
		curModelIni.GetString()
	);

	return FALSE;
}

static BOOL GetCurrentRecipeIniPathByRack(
	int rackNo,
	CString& outRecipeIniPath,
	CString& outErrMsg)
{
	outRecipeIniPath.Empty();
	outErrMsg.Empty();

	int recipeNo = 0;

	if (!GetCurrentRecipeNoByRack(rackNo, recipeNo, outErrMsg))
		return FALSE;

	outRecipeIniPath.Format(_T(".\\RMS\\Recipe\\%03d.ini"), recipeNo);

	if (!FileExistsSimple(outRecipeIniPath))
	{
		outErrMsg.Format(
			_T("Recipe ini not found. rackNo=%d recipeNo=%03d file=%s"),
			rackNo,
			recipeNo,
			outRecipeIniPath.GetString()
		);
		return FALSE;
	}

	return TRUE;
}

/// <summary>
/// EPSC PARAMETER.INI을 RACK별로 분리하는 함수
/// </summary>
/// <param name="unit"></param>
/// <param name="outRackNo"></param>
/// <returns></returns>
static BOOL GetRackNoFromUnit(const CString& unit, int& outRackNo)
{
	outRackNo = 0;

	if (unit.IsEmpty())
		return FALSE;

	TCHAR ch = unit[unit.GetLength() - 1];

	if (ch < _T('0') || ch > _T('9'))
		return FALSE;

	outRackNo = ch - _T('0');

	// 현재 장비는 Rack 1~6만 사용
	if (outRackNo < 1 || outRackNo > 6)
		return FALSE;

	return TRUE;
}


// ===================== [추가 시작] EPSC 파싱/타입 helper =====================

struct CStringNoCaseLess
{
	bool operator()(const CString& a, const CString& b) const
	{
		return a.CompareNoCase(b) < 0;
	}
};

// KEY#VALUE 1개
struct EpscSettingField
{
	CString key;
	CString rawValue;
	CString decodedValue;
};

// 파라미터 1개
class EpscSettingItem
{
public:
	CString paramName;
	CString subUnit;
	CString rawBlock;

	std::vector<EpscSettingField> fields;

	void Clear()
	{
		paramName.Empty();
		subUnit.Empty();
		rawBlock.Empty();
		fields.clear();
		m_fieldIndex.clear();
	}

	void SetField(const CString& key, const CString& rawValue, const CString& decodedValue = _T(""))
	{
		auto it = m_fieldIndex.find(key);
		CString finalDecoded = decodedValue.IsEmpty() ? rawValue : decodedValue;

		if (it == m_fieldIndex.end())
		{
			EpscSettingField f;
			f.key = key;
			f.rawValue = rawValue;
			f.decodedValue = finalDecoded;

			m_fieldIndex[key] = fields.size();
			fields.push_back(f);
		}
		else
		{
			EpscSettingField& f = fields[it->second];
			f.rawValue = rawValue;
			f.decodedValue = finalDecoded;
		}
	}

	CString GetRaw(const CString& key) const
	{
		auto it = m_fieldIndex.find(key);
		if (it == m_fieldIndex.end())
			return _T("");
		return fields[it->second].rawValue;
	}

	CString GetDecoded(const CString& key) const
	{
		auto it = m_fieldIndex.find(key);
		if (it == m_fieldIndex.end())
			return _T("");
		return fields[it->second].decodedValue;
	}

	bool HasField(const CString& key) const
	{
		return m_fieldIndex.find(key) != m_fieldIndex.end();
	}

private:
	std::map<CString, size_t, CStringNoCaseLess> m_fieldIndex;
};

// SETTING_INFO 전체
class EpscSettingInfo
{
public:
	CString rawText;
	std::vector<EpscSettingItem> items;

	void Clear()
	{
		rawText.Empty();
		items.clear();
		m_itemIndex.clear();
	}

	EpscSettingItem* Find(const CString& paramName)
	{
		auto it = m_itemIndex.find(paramName);
		if (it == m_itemIndex.end())
			return nullptr;
		return &items[it->second];
	}

	const EpscSettingItem* Find(const CString& paramName) const
	{
		auto it = m_itemIndex.find(paramName);
		if (it == m_itemIndex.end())
			return nullptr;
		return &items[it->second];
	}

	EpscSettingItem& AddOrGet(const CString& paramName)
	{
		auto it = m_itemIndex.find(paramName);
		if (it != m_itemIndex.end())
			return items[it->second];

		EpscSettingItem item;
		item.paramName = paramName;

		size_t newIndex = items.size();
		items.push_back(item);
		m_itemIndex[paramName] = newIndex;

		return items[newIndex];
	}

private:
	std::map<CString, size_t, CStringNoCaseLess> m_itemIndex;
};

static BOOL IsWhiteSpaceChar(TCHAR ch)
{
	return (ch == _T(' ') ||
		ch == _T('\t') ||
		ch == _T('\r') ||
		ch == _T('\n'));
}

// 중첩 [] 전체를 안전하게 추출
static CString ExtractFieldValueFullBracket(const CString& msg, LPCTSTR keyWithEq)
{
	CString key(keyWithEq);

	int pos = msg.Find(key);
	if (pos < 0)
		return _T("");

	int start = pos + key.GetLength();
	if (start >= msg.GetLength())
		return _T("");

	while (start < msg.GetLength() && IsWhiteSpaceChar(msg[start]))
		start++;

	if (start >= msg.GetLength())
		return _T("");

	// [로 시작 안 하면 일반 토큰처럼 처리
	if (msg[start] != _T('['))
	{
		int end = start;
		while (end < msg.GetLength() && !IsWhiteSpaceChar(msg[end]))
			end++;

		return msg.Mid(start, end - start);
	}

	int depth = 0;
	for (int i = start; i < msg.GetLength(); ++i)
	{
		if (msg[i] == _T('['))
			++depth;
		else if (msg[i] == _T(']'))
		{
			--depth;
			if (depth == 0)
				return msg.Mid(start, i - start + 1);
		}
	}

	return msg.Mid(start);
}

static int HexCharToInt(TCHAR ch)
{
	if (ch >= _T('0') && ch <= _T('9'))
		return ch - _T('0');
	if (ch >= _T('A') && ch <= _T('F'))
		return 10 + (ch - _T('A'));
	if (ch >= _T('a') && ch <= _T('f'))
		return 10 + (ch - _T('a'));
	return -1;
}

static CString TrimCopy(const CString& s)
{
	CString t = s;
	t.Trim();
	return t;
}

static CString DecodeEpscToken(const CString& src)
{
	CString out;
	const int len = src.GetLength();

	for (int i = 0; i < len; ++i)
	{
		TCHAR ch = src[i];

		if (ch == _T('?') && (i + 2) < len)
		{
			int hi = HexCharToInt(src[i + 1]);
			int lo = HexCharToInt(src[i + 2]);

			if (hi >= 0 && lo >= 0)
			{
				TCHAR decoded = (TCHAR)((hi << 4) | lo);
				out.AppendChar(decoded);
				i += 2;
				continue;
			}
		}

		out.AppendChar(ch);
	}

	return out;
}

static CString RemoveOuterSquareBracketsIfWhole(const CString& s)
{
	CString t = TrimCopy(s);
	if (t.GetLength() >= 2 && t[0] == _T('[') && t[t.GetLength() - 1] == _T(']'))
	{
		int depth = 0;
		bool wrapsWhole = false;

		for (int i = 0; i < t.GetLength(); ++i)
		{
			if (t[i] == _T('['))
				++depth;
			else if (t[i] == _T(']'))
			{
				--depth;
				if (depth == 0)
				{
					wrapsWhole = (i == t.GetLength() - 1);
					break;
				}
			}
		}

		if (wrapsWhole)
			return t.Mid(1, t.GetLength() - 2);
	}

	return t;
}

static std::vector<CString> SplitTopLevelByComma(const CString& s)
{
	std::vector<CString> parts;
	CString cur;
	int depth = 0;

	for (int i = 0; i < s.GetLength(); ++i)
	{
		TCHAR ch = s[i];

		if (ch == _T('['))
		{
			++depth;
			cur.AppendChar(ch);
		}
		else if (ch == _T(']'))
		{
			--depth;
			cur.AppendChar(ch);
		}
		else if (ch == _T(',') && depth == 0)
		{
			CString part = TrimCopy(cur);
			if (!part.IsEmpty())
				parts.push_back(part);
			cur.Empty();
		}
		else
		{
			cur.AppendChar(ch);
		}
	}

	cur.Trim();
	if (!cur.IsEmpty())
		parts.push_back(cur);

	return parts;
}

static std::vector<CString> SplitTopLevelByCaret(const CString& s)
{
	std::vector<CString> parts;
	CString cur;
	int depth = 0;

	for (int i = 0; i < s.GetLength(); ++i)
	{
		TCHAR ch = s[i];

		if (ch == _T('['))
		{
			++depth;
			cur.AppendChar(ch);
		}
		else if (ch == _T(']'))
		{
			--depth;
			cur.AppendChar(ch);
		}
		else if (ch == _T('^') && depth == 0)
		{
			CString part = TrimCopy(cur);
			if (!part.IsEmpty())
				parts.push_back(part);
			cur.Empty();
		}
		else
		{
			cur.AppendChar(ch);
		}
	}

	cur.Trim();
	if (!cur.IsEmpty())
		parts.push_back(cur);

	return parts;
}

static BOOL ParseOneSettingEntry(const CString& entryRaw, EpscSettingItem& outItem)
{
	outItem.Clear();

	CString entry = TrimCopy(entryRaw);
	if (entry.IsEmpty())
		return FALSE;

	// 앞의 ':' 제거
	if (entry[0] == _T(':'))
		entry = entry.Mid(1);

	int posOpen = entry.Find(_T(":["));
	if (posOpen < 0)
		return FALSE;

	CString paramName = entry.Left(posOpen);
	paramName.Trim();
	if (paramName.IsEmpty())
		return FALSE;

	CString rest = entry.Mid(posOpen + 2);
	rest.Trim();

	if (!rest.IsEmpty() && rest[rest.GetLength() - 1] == _T(']'))
		rest = rest.Left(rest.GetLength() - 1);

	outItem.paramName = paramName;
	outItem.rawBlock = entryRaw;

	std::vector<CString> fieldTokens = SplitTopLevelByCaret(rest);

	for (size_t i = 0; i < fieldTokens.size(); ++i)
	{
		CString token = TrimCopy(fieldTokens[i]);
		if (token.IsEmpty())
			continue;

		int sharp = token.Find(_T('#'));

		CString key;
		CString rawValue;

		if (sharp >= 0)
		{
			key = token.Left(sharp);
			rawValue = token.Mid(sharp + 1);
		}
		else
		{
			key = token;
			rawValue = _T("");
		}

		key.Trim();
		rawValue.Trim();

		CString decodedValue = DecodeEpscToken(rawValue);
		outItem.SetField(key, rawValue, decodedValue);

		if (key.CompareNoCase(_T("SUBUNIT")) == 0)
			outItem.subUnit = decodedValue;
	}

	return TRUE;
}

static BOOL ParseSettingInfo(const CString& settingInfoRaw, EpscSettingInfo& outInfo)
{
	outInfo.Clear();
	outInfo.rawText = settingInfoRaw;

	CString text = TrimCopy(settingInfoRaw);
	if (text.IsEmpty())
		return TRUE;

	text = RemoveOuterSquareBracketsIfWhole(text);
	text.Trim();

	if (text.IsEmpty())
		return TRUE;

	std::vector<CString> entries = SplitTopLevelByComma(text);

	for (size_t i = 0; i < entries.size(); ++i)
	{
		EpscSettingItem item;
		if (ParseOneSettingEntry(entries[i], item))
		{
			EpscSettingItem& dst = outInfo.AddOrGet(item.paramName);
			dst = item;
		}
	}

	return TRUE;
}

// ===================== [추가 끝] EPSC 파싱/타입 helper =====================

// ===================== [추가 시작] EPSC_R I용 Parameter.ini -> SETTING_INFO builder =====================



struct EpscParaMap
{
	LPCTSTR displayName;   // EPSC_R에 넣을 파라미터 이름
	LPCTSTR iniSection;    // Parameter.ini 섹션명
};

static CString BuildEpscSettingInfoFromParameterIni(const CString& parameterIniPath, int& outParaCount)
{
	static const EpscParaMap kParams[] =
	{
		// MODEL_NB는 MODEL_NUMBER 섹션 값을 사용
		{ _T("MODEL_NB"),        _T("MODEL_NUMBER") },

		// RECIPE 파라미터들
		{ _T("DIMMING_SEL"),     _T("DIMMING_SEL") },
		{ _T("PWM_FREQ"),        _T("PWM_FREQ") },
		{ _T("PWM_DUTY"),        _T("PWM_DUTY") },
		{ _T("VBR_VOLT"),        _T("VBR_VOLT") },
		{ _T("CABLE_OPEN"),      _T("CABLE_OPEN") },

		{ _T("POWER_ON_SEQ1"),   _T("POWER_ON_SEQ1") },
		{ _T("POWER_ON_SEQ2"),   _T("POWER_ON_SEQ2") },
		{ _T("POWER_ON_SEQ3"),   _T("POWER_ON_SEQ3") },
		{ _T("POWER_ON_SEQ4"),   _T("POWER_ON_SEQ4") },
		{ _T("POWER_ON_SEQ5"),   _T("POWER_ON_SEQ5") },
		{ _T("POWER_ON_SEQ6"),   _T("POWER_ON_SEQ6") },
		{ _T("POWER_ON_SEQ7"),   _T("POWER_ON_SEQ7") },
		{ _T("POWER_ON_SEQ8"),   _T("POWER_ON_SEQ8") },
		{ _T("POWER_ON_SEQ9"),   _T("POWER_ON_SEQ9") },
		{ _T("POWER_ON_SEQ10"),  _T("POWER_ON_SEQ10") },

		{ _T("POWER_ON_DELAY1"), _T("POWER_ON_DELAY1") },
		{ _T("POWER_ON_DELAY2"), _T("POWER_ON_DELAY2") },
		{ _T("POWER_ON_DELAY3"), _T("POWER_ON_DELAY3") },
		{ _T("POWER_ON_DELAY4"), _T("POWER_ON_DELAY4") },
		{ _T("POWER_ON_DELAY5"), _T("POWER_ON_DELAY5") },
		{ _T("POWER_ON_DELAY6"), _T("POWER_ON_DELAY6") },
		{ _T("POWER_ON_DELAY7"), _T("POWER_ON_DELAY7") },
		{ _T("POWER_ON_DELAY8"), _T("POWER_ON_DELAY8") },
		{ _T("POWER_ON_DELAY9"), _T("POWER_ON_DELAY9") },

		{ _T("POWER_OFF_SEQ1"),  _T("POWER_OFF_SEQ1") },
		{ _T("POWER_OFF_SEQ2"),  _T("POWER_OFF_SEQ2") },
		{ _T("POWER_OFF_SEQ3"),  _T("POWER_OFF_SEQ3") },
		{ _T("POWER_OFF_SEQ4"),  _T("POWER_OFF_SEQ4") },
		{ _T("POWER_OFF_SEQ5"),  _T("POWER_OFF_SEQ5") },
		{ _T("POWER_OFF_SEQ6"),  _T("POWER_OFF_SEQ6") },
		{ _T("POWER_OFF_SEQ7"),  _T("POWER_OFF_SEQ7") },
		{ _T("POWER_OFF_SEQ8"),  _T("POWER_OFF_SEQ8") },
		{ _T("POWER_OFF_SEQ9"),  _T("POWER_OFF_SEQ9") },
		{ _T("POWER_OFF_SEQ10"), _T("POWER_OFF_SEQ10") },

		{ _T("POWER_OFF_DELAY1"), _T("POWER_OFF_DELAY1") },
		{ _T("POWER_OFF_DELAY2"), _T("POWER_OFF_DELAY2") },
		{ _T("POWER_OFF_DELAY3"), _T("POWER_OFF_DELAY3") },
		{ _T("POWER_OFF_DELAY4"), _T("POWER_OFF_DELAY4") },
		{ _T("POWER_OFF_DELAY5"), _T("POWER_OFF_DELAY5") },
		{ _T("POWER_OFF_DELAY6"), _T("POWER_OFF_DELAY6") },
		{ _T("POWER_OFF_DELAY7"), _T("POWER_OFF_DELAY7") },
		{ _T("POWER_OFF_DELAY8"), _T("POWER_OFF_DELAY8") },
		{ _T("POWER_OFF_DELAY9"), _T("POWER_OFF_DELAY9") },

		{ _T("VCC_VOLT"),            _T("VCC_VOLT") },
		{ _T("VCC_VOLT_OFFSET"),     _T("VCC_VOLT_OFFSET") },
		{ _T("VCC_LIMIT_VOLT_LOW"),  _T("VCC_LIMIT_VOLT_LOW") },
		{ _T("VCC_LIMIT_VOLT_HIGH"), _T("VCC_LIMIT_VOLT_HIGH") },
		{ _T("VCC_LIMIT_CURR_LOW"),  _T("VCC_LIMIT_CURR_LOW") },
		{ _T("VCC_LIMIT_CURR_HIGH"), _T("VCC_LIMIT_CURR_HIGH") },

		{ _T("VBL_VOLT"),            _T("VBL_VOLT") },
		{ _T("VBL_VOLT_OFFSET"),     _T("VBL_VOLT_OFFSET") },
		{ _T("VBL_LIMIT_VOLT_LOW"),  _T("VBL_LIMIT_VOLT_LOW") },
		{ _T("VBL_LIMIT_VOLT_HIGH"), _T("VBL_LIMIT_VOLT_HIGH") },
		{ _T("VBL_LIMIT_CURR_LOW"),  _T("VBL_LIMIT_CURR_LOW") },
		{ _T("VBL_LIMIT_CURR_HIGH"), _T("VBL_LIMIT_CURR_HIGH") },

		{ _T("AGING_TIME_HH"),        _T("AGING_TIME_HH") },
		{ _T("AGING_TIME_MM"),        _T("AGING_TIME_MM") },
		{ _T("AGING_TIME_MINUTE"),    _T("AGING_TIME_MINUTE") },
		{ _T("AGING_END_WAIT_TIME"),  _T("AGING_END_WAIT_TIME") },

		{ _T("TEMPERATURE_USE"), _T("TEMPERATURE_USE") },
		{ _T("TEMPERATURE_MIN"), _T("TEMPERATURE_MIN") },
		{ _T("TEMPERATURE_MAX"), _T("TEMPERATURE_MAX") },
		{ _T("DOOR_USE"),        _T("DOOR_USE") }
	};

	CString msg;
	outParaCount = 0;

	for (int i = 0; i < _countof(kParams); ++i)
	{
		AppendEpscSettingItemFromIni(
			msg,
			parameterIniPath,
			kParams[i].displayName,
			kParams[i].iniSection
		);
		outParaCount++;
	}

	return msg;
}

// ===================== [추가 끝] EPSC_R I용 Parameter.ini -> SETTING_INFO builder =====================

// ===================== [추가 시작] EPSC S -> Parameter.ini apply helper =====================

static CString MapEpscParamNameToIniSection(const CString& paramName)
{
	// RMS의 MODEL_NB는 Parameter.ini의 MODEL_NUMBER 섹션으로 매핑
	/*if (paramName.CompareNoCase(_T("MODEL_NB")) == 0)
		return _T("MODEL_NUMBER");*/

	return paramName;
}

static BOOL WriteIniValueSimple(const CString& iniPath, const CString& section, const CString& key, const CString& value)
{
	// value가 빈 문자열이어도 key= 형태로 기록됨
	return ::WritePrivateProfileString(section, key, value, iniPath);
}

static BOOL ApplyOneEpscItemToParameterIni(
	const CString& parameterIniPath,
	const EpscSettingItem& item,
	CString& outErrMsg)
{
	CString iniSection = MapEpscParamNameToIniSection(item.paramName);

	if (iniSection.IsEmpty())
	{
		outErrMsg = _T("Invalid section name");
		return FALSE;
	}

	for (size_t i = 0; i < item.fields.size(); ++i)
	{
		const EpscSettingField& field = item.fields[i];

		CString key = field.key;
		CString value = field.decodedValue;   // ?2E -> . , ?5F -> _ 반영된 값 사용

		key.Trim();
		if (key.IsEmpty())
			continue;

		if (!WriteIniValueSimple(parameterIniPath, iniSection, key, value))
		{
			outErrMsg.Format(_T("WritePrivateProfileString failed. section=%s key=%s value=%s"),
				iniSection.GetString(),
				key.GetString(),
				value.GetString());
			return FALSE;
		}
	}

	return TRUE;
}

static BOOL ApplyEpscSettingInfoToParameterIni(
	const CString& parameterIniPath,
	const EpscSettingInfo& info,
	int& outAppliedCount,
	CString& outErrMsg)
{
	outAppliedCount = 0;
	outErrMsg.Empty();

	for (size_t i = 0; i < info.items.size(); ++i)
	{
		const EpscSettingItem& item = info.items[i];

		if (!ApplyOneEpscItemToParameterIni(parameterIniPath, item, outErrMsg))
			return FALSE;

		outAppliedCount++;
	}

	return TRUE;
}

// ===================== [추가 끝] EPSC S -> Parameter.ini apply helper =====================

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
	//pattern.Format(_T("%d.ini"), recipeNo);

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
// Recipe 폴더에서 번호 ini가 존재하는지 찾는 함수
static BOOL FindRecipeIniByNo(const CString& recipeDir, CString& recipeNo, CString& outKey)
{
	outKey.Empty();

	// 예: ".\\Recipe\\1.ini"
	CString fullPath;
	fullPath.Format(_T("%s\\%s.ini"), recipeDir.GetString(), recipeNo);

	WIN32_FIND_DATA fd = { 0 };
	HANDLE h = FindFirstFile(fullPath, &fd);
	if (h == INVALID_HANDLE_VALUE)
		return FALSE;
	FindClose(h);

	// Gf_loadRecipeData() 에 넘길 key는 "1" 같은 형태로
	outKey.Format(_T("%d"), recipeNo);
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
		if (rms == nullptr)
		{
			m_blsRmsConnect = FALSE;
			return FALSE;
		}

		VARIANT_BOOL resultConnect = rms->Connect();

		if (resultConnect == VARIANT_TRUE)
		{
			m_blsRmsConnect = TRUE;

			CString sLog;
			sLog.Format(_T("<RMS> RMS Server Connection Succeeded / %s"),
				m_strLocalSubjectRMS.GetString());
			m_pApp->Gf_writeMLog(sLog);

			StartRmsRecvThread();
			return TRUE;
		}
		else
		{
			m_blsRmsConnect = FALSE;

			CString sLog;
			sLog.Format(_T("<RMS> RMS Server Connection Fail / %s"),
				m_strLocalSubjectRMS.GetString());
			m_pApp->Gf_writeMLog(sLog);

			return FALSE;
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
		StopRmsRecvThread();

		if (rms == nullptr)
			return TRUE;

		VARIANT_BOOL resultDisConnect = rms->Terminate();
		rms->Release();
		rms = nullptr;
		m_blsRmsConnect = FALSE;

		return (resultDisConnect == VARIANT_TRUE);
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

		if (rms != nullptr)
		{
			rms->Terminate();
			rms->Release();
			rms = nullptr;
		}

		HRESULT rmsHr = CoCreateInstance(
			CLSID_DllRms,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ICallRMSClass,
			reinterpret_cast<void**>(&rms)
		);

		if (FAILED(rmsHr) || rms == nullptr)
		{
			m_pApp->Gf_writeMLog(_T("<RMS> RMS CoCreateInstance Fail"));
			return FALSE;
		}

		rms->SetTimeOut(5);

		VARIANT_BOOL resultIni = rms->Init(
			(_bstr_t)m_strServicePortRMS,
			(_bstr_t)m_strNetworkRMS,
			(_bstr_t)m_strDaemonRMS,
			(_bstr_t)m_strRemoteSubjectRMS,
			(_bstr_t)m_strLocalSubjectRMS
		);

		if (resultIni != VARIANT_TRUE)
		{
			CString sLog;
			sLog.Format(_T("<RMS> RMS TIB INIT Fail / LocalSubject=%s"),
				m_strLocalSubjectRMS.GetString());
			m_pApp->Gf_writeMLog(sLog);
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

BOOL CCimNetCommApi::MessageSend (int nMode)	// Event
{
	return MessageSend(nMode, 1);
}

BOOL CCimNetCommApi::MessageSend(int nMode, int rackNo)
{
	_bstr_t bstBuff = (LPSTR)(LPCTSTR)m_strRemoteSubject;
	BSTR m_remoteTemp = bstBuff.copy();
	CString sLog = _T("");
	//	_bstr_t m_remoteTemp = bstBuff.copy ();

	CString strBuff(_T(""));

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

	Sleep(10);

	if (nMode != ECS_MODE_APDR && nMode != ECS_MODE_RMSO && nMode != ECS_MODE_ERCP)
	{
		if (m_pApp->m_bIsGmesConnect == FALSE)
			return RTN_MSG_NOT_SEND;

		VARIANT_BOOL bRetCode = gmes->SendTibMessage((_bstr_t)m_strHostSendMessage);

		do {
			if (gmes->GetreceivedDataFlag() == VARIANT_TRUE) {
				m_sReceiveMessage = (LPCTSTR)gmes->GetReceiveData();
				break;
			}
			if (bRetCode == VARIANT_FALSE) {

				m_pApp->Gf_writeMLog(_T("<HOST_S> Did not send a MES Message. Retry !!!"));
				break;
			}

		} while (1);


		if (bRetCode == VARIANT_FALSE) {

			bRetCode = gmes->SendTibMessage((_bstr_t)m_strHostSendMessage);

			sLog.Format(_T("<HOST_S> %s"), m_strHostSendMessage);
			m_pApp->Gf_writeMLog(sLog);

			do {
				if (gmes->GetreceivedDataFlag() == VARIANT_TRUE) {
					m_sReceiveMessage = (LPCTSTR)gmes->GetReceiveData();
					break;
				}
				if (bRetCode == VARIANT_FALSE) {

					AfxMessageBox(_T("Did not send a message !!!"));
					return RTN_MSG_NOT_SEND;	// 통신 NG
				}

			} while (1);
		}


	}
	else if (nMode == ECS_MODE_RMSO || nMode == ECS_MODE_ERCP)
	{
		if (m_blsRmsConnect == FALSE)
			return RTN_MSG_NOT_SEND;

		VARIANT_BOOL bRet = rms->SendTibMessage((_bstr_t)m_strHostSendMessage);

		if (bRet == VARIANT_FALSE)
		{
			m_pApp->Gf_writeMLog(_T("<HOST_S> Did not send a RMS Message. Retry !!!"));
			bRet = rms->SendTibMessage((_bstr_t)m_strHostSendMessage);

			if (bRet == VARIANT_FALSE)
			{
				AfxMessageBox(_T("Did not send a message !!! (RMS)"));
				return RTN_MSG_NOT_SEND;
			}
		}

		do
		{
			if (rms->GetreceivedDataFlag() == VARIANT_TRUE)
			{
				m_sReceiveMessage = (LPCTSTR)rms->GetReceiveData();
				break;
			}
		} while (1);
	}
	else
	{
		if (m_pApp->m_bIsEasConnect == FALSE)
			return RTN_MSG_NOT_SEND;

		VARIANT_BOOL bRetCode = eas->SendTibMessage((_bstr_t)m_strHostSendMessage);

		do {
			if (eas->GetreceivedDataFlag() == VARIANT_TRUE)
			{
				m_sReceiveMessage = (LPCTSTR)eas->GetReceiveData();
				break;
			}
			if (bRetCode == VARIANT_FALSE)
			{
				m_pApp->Gf_writeMLog(_T("<HOST_S> Did not send a EAS Message. Retry !!!"));
				break;
			}

		} while (1);

		if (bRetCode == VARIANT_FALSE) {

			bRetCode = eas->SendTibMessage((_bstr_t)m_strHostSendMessage);

			sLog.Format(_T("<HOST_S> %s"), m_strHostSendMessage);
			m_pApp->Gf_writeMLog(sLog);

			do {
				if (eas->GetreceivedDataFlag() == VARIANT_TRUE) {
					m_sReceiveMessage = (LPCTSTR)eas->GetReceiveData();
					break;
				}
				if (bRetCode == VARIANT_FALSE) {

					AfxMessageBox(_T("Did not send a message !!!"));
					return RTN_MSG_NOT_SEND;	// 통신 NG
				}

			} while (1);
		}
	}

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
	CString localSubject = m_strLocalSubjectRMS;
	CString unitName;
	unitName.Format(_T("%s01"), m_strMachineName.GetString());

	TRACE(_T("[SetERCPInfo] this=%p, value=%s\n"), this, m_strERCPInfo);

	m_strERCP.Format(
		_T("ERCP ADDR=%s,%s EQP=%s MACHINE=%s UNIT=%s RCS=H MODE_CODE=N MODEL=%s BASEMODEL= CATEGORY= RECIPE=%d RECIPEVER= OPER= NW_CD= NW_DESCRIPTION=[] CHANGE_TYPE=B VALIDATIONINFO=[] UNIT_INFO=[%s:[1]:U:1:3:%s:0:[%s]] REPLY_REQ=Y TO_EQP= MMC_TXN_ID="),
		m_strRemoteSubjectRMS,
		localSubject,
		m_strEqpRMS,
		//m_strMachineName.Left(m_strMachineName.GetLength() - 2),
		m_strMachineName,
		m_strMachineName,
		/*m_strMachineName,
		unitName,*/
		lpInspWorkInfo->Ercp_Model_Name,
		lpInspWorkInfo->Ercp_Recipe,
		m_strMachineName.Left(m_strMachineName.GetLength() - 2),
		m_strMachineName,
		m_strERCPInfo
	);

	CString modelname = lpInspWorkInfo->Ercp_Model_Name;
	int recipe = lpInspWorkInfo->Ercp_Recipe;

	BOOL nRetCode = MessageSend(ECS_MODE_ERCP);
	if (nRetCode != RTN_OK)
		return nRetCode;

	CString strMsg;
	GetFieldData(&strMsg, _T("RTN_CD"));
	if (strMsg.Compare(_T("0")) != 0)
		return 3;

	GetFieldData(&strMsg, _T("ERR_CD"));
	if (strMsg.Compare(_T("0")) != 0)
		return 3;

	return RTN_OK;
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
	m_strEWOQ.Format(_T("EWOQ ADDR=%s,%s EQP=%s COMPLETE_YN=Y USER_ID=%s MODE=AUTO CLIENT_DATE=%s")
		//, m_strRemoteSubjectRMS
		, m_strLocalSubjectMesF
		//, m_strLocalSubjectRMS
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
	m_strEPIQ.Format(_T("EPIQ ADDR=%s,%s EQP=%s COMPLETE_YN=Y USER_ID=%s MODE=AUTO CLIENT_DATE=%s COMMENT=[]")
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


	if (lpInspWorkInfo->Msg_Test == 1) // RMS
	{
		int nRetCode = MessageSend(ECS_MODE_RMSO);
		if (nRetCode != RTN_OK)
		{
			return nRetCode;
		}
	}
	else
	{
		int nRetCode = MessageSend(ECS_MODE_EAYT);
		if (nRetCode != RTN_OK)
		{
			return nRetCode;
		}
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
	IStream* pStream;   // marshaled ICallRMSClass
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
	delete param;

	HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	ICallRMSClass* pRmsThread = nullptr;
	HRESULT hr = CoGetInterfaceAndReleaseStream(pStream, IID_ICallRMSClass, (void**)&pRmsThread);
	if (FAILED(hr) || pRmsThread == nullptr)
	{
		if (SUCCEEDED(hrCo)) CoUninitialize();
		return 0;
	}

	while (true)
	{
		if (pThis->m_hRmsStopEvent &&
			::WaitForSingleObject(pThis->m_hRmsStopEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}

		if (pRmsThread->GetreceivedDataFlag() == VARIANT_TRUE)
		{
			_bstr_t b = pRmsThread->GetReceiveData();
			CString msg = (LPCTSTR)b;

			CString log;
			log.Format(_T("[RMS] %s"), msg.GetString());
			m_pApp->Gf_writeRMSLog(log);

			CString cmd = msg.Left(4);

			if (cmd == _T("EPLR"))
			{
				pThis->HandleRmsMsg_EPLR(msg, pRmsThread);
			}
			else if (cmd == _T("EPPR"))
			{
				// rackNo 인자 제거 버전으로 바꾸는게 가장 깔끔
				pThis->HandleRmsMsg_EPPR(msg, pRmsThread, 1);
			}
			else if (cmd == _T("ERCP"))
			{
				pThis->HandleRmsMsg_ERCP(msg, pRmsThread);
			}
			else if (cmd == _T("EPSC"))
			{
				pThis->HandleRmsMsg_EPSC(msg, pRmsThread);
			}
			else if (cmd == _T("EPDC"))
			{
				pThis->HandleRmsMsg_EPDC(msg, pRmsThread);
			}
		}

		::Sleep(5);
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

	CString modelListPath = _T(".\\RMS\\ModelList.ini");
	CString recipeMsgSet = BuildRecipeMsgSetFromModelList(machine, unit, modelListPath);

	CString oneItem, recipeMsgSet_Test;
	for (int i = 1; i <= 100; i++)
	{
		/*oneItem.Format(_T("W4AMAL04HV:W4AMAL04HV01:[%03d]:3:U:"), i);*/
		oneItem.Format(_T("W4AMAL04HV:W4AMAL04HV01:[%d]:3:U:"), i);

		// 첫 항목이 아니면 앞에 콤마 추가
		if (!recipeMsgSet_Test.IsEmpty())
			recipeMsgSet_Test += _T(",");

		recipeMsgSet_Test += oneItem;
	}


	// 4) EPLR_R 구성
	CString reply;
	reply.Format(
		_T("EPLR_R ADDR=%s,%s EQP=%s RECIPEINFO=[%s] ESD= ESDINFO=[] SEQ_NO=%s MMC_TXN_ID="),
		m_strRemoteSubjectRMS,
		m_strLocalSubjectRMS,
		m_strEqpRMS,
		//recipeMsgSet,
		recipeMsgSet_Test,
		seqNo
	);

	m_pApp->Gf_writeRMSLog(reply);

	// 5) 딱 1번 전송(대기 없음)
	VARIANT_BOOL ok = pRmsThread->SendTibMessageNoWait((_bstr_t)reply);
	//VARIANT_BOOL ok = pRmsThread->SendTibMessage((_bstr_t)reply);

	if (ok == VARIANT_FALSE)
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPLR_R send failed"));
	else
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPLR_R send ok"));
}

void CCimNetCommApi::HandleRmsMsg_EPPR(const CString& msg, ICallRMSClass* pRmsThread, int RackNo)
{
	if (pRmsThread == nullptr)
		return;

	CString machine, unit, recipe, level, recipe_type, recipe_yn;
	CString command_code, unit_type, recipever, esd, reply_req;
	CString seq_no, mmc_txn_id, eqp_name;

	machine = ExtractFieldValue(msg, _T("MACHINE="));
	unit = ExtractFieldValue(msg, _T("UNIT="));
	recipe = ExtractFieldValue(msg, _T("RECIPE="));
	level = ExtractFieldValue(msg, _T("LEVEL="));
	recipe_type = ExtractFieldValue(msg, _T("RECIPETYPE="));
	recipe_yn = ExtractFieldValue(msg, _T("CURRENT_RECIPE_YN="));
	command_code = ExtractFieldValue(msg, _T("COMMAND_CODE="));
	unit_type = ExtractFieldValue(msg, _T("UNIT_TYPE="));
	recipever = ExtractFieldValue(msg, _T("RECIPEVER="));
	esd = ExtractFieldValue(msg, _T("ESD="));
	reply_req = ExtractFieldValue(msg, _T("REPLY_REQ="));
	seq_no = ExtractFieldValue(msg, _T("SEQ_NO="));
	mmc_txn_id = ExtractFieldValue(msg, _T("MMC_TXN_ID="));

	int rackNo = 0;

	if (!GetRackNoFromUnit(unit, rackNo))
	{
		// 기존 호출부에서 넘겨준 RackNo가 있으면 fallback
		if (RackNo >= 1 && RackNo <= RMS_RACK_COUNT)
			rackNo = RackNo;
	}

	if (rackNo < 1 || rackNo > RMS_RACK_COUNT)
	{
		CString reply;
		reply.Format(
			_T("EPPR_R ADDR=%s,%s EQP=%s RECIPEINFO=[::[]] ESD= ESDINFO=[] SEQ_NO=%s MMC_TXN_ID=%s"),
			m_strRemoteSubjectRMS,
			m_strLocalSubjectRMS,
			m_strEqpRMS,
			seq_no.GetString(),
			mmc_txn_id.GetString()
		);

		m_pApp->Gf_writeRMSLog(_T("[RMS] EPPR invalid rack number"));
		m_pApp->Gf_writeRMSLog(reply);

		pRmsThread->SendTibMessageNoWait((_bstr_t)reply);
		return;
	}

	lpSystemInfo = m_pApp->GetSystemInfo();
	eqp_name = lpSystemInfo->m_sEqpName;

	CString rackRecipeIniPath = GetRmsRackRecipePath(rackNo);
	CString valueSourceIniPath;

	int replyRecipeNo = 0;

	if (recipe_yn.CompareNoCase(_T("Y")) == 0)
	{
		// CURRENT_RECIPE_YN=Y
		// 값은 RACKn_CurModel.ini에서 읽음
		valueSourceIniPath = GetRmsRackCurModelPath(rackNo);

		replyRecipeNo = ReadRecipeNoFromModelInfoIni(valueSourceIniPath);
	}
	else if (recipe_yn.CompareNoCase(_T("N")) == 0)
	{
		// CURRENT_RECIPE_YN=N
		// RECIPE=4 → RMS\Recipe\004.ini에서 값 읽음
		replyRecipeNo = _ttoi(recipe);

		if (replyRecipeNo <= 0)
		{
			CString reply;
			reply.Format(
				_T("EPPR_R ADDR=%s,%s EQP=%s RECIPEINFO=[::[]] ESD= ESDINFO=[] SEQ_NO=%s MMC_TXN_ID=%s"),
				m_strRemoteSubjectRMS,
				m_strLocalSubjectRMS,
				m_strEqpRMS,
				seq_no.GetString(),
				mmc_txn_id.GetString()
			);

			m_pApp->Gf_writeRMSLog(_T("[RMS] EPPR invalid RECIPE number"));
			m_pApp->Gf_writeRMSLog(reply);

			pRmsThread->SendTibMessageNoWait((_bstr_t)reply);
			return;
		}

		valueSourceIniPath = GetSharedRecipePathByNo(replyRecipeNo);
	}
	else
	{
		CString reply;
		reply.Format(
			_T("EPPR_R ADDR=%s,%s EQP=%s RECIPEINFO=[::[]] ESD= ESDINFO=[] SEQ_NO=%s MMC_TXN_ID=%s"),
			m_strRemoteSubjectRMS,
			m_strLocalSubjectRMS,
			m_strEqpRMS,
			seq_no.GetString(),
			mmc_txn_id.GetString()
		);

		m_pApp->Gf_writeRMSLog(_T("[RMS] EPPR invalid CURRENT_RECIPE_YN"));
		m_pApp->Gf_writeRMSLog(reply);

		pRmsThread->SendTibMessageNoWait((_bstr_t)reply);
		return;
	}

	CString recipeMsgSet;
	int paramCount = 0;
	CString errMsg;

	BOOL bBuildOk = BuildEpprRecipeMsgSetFromIni(
		rackRecipeIniPath,
		valueSourceIniPath,
		recipeMsgSet,
		paramCount,
		errMsg
	);

	if (!bBuildOk)
	{
		CString log;
		log.Format(
			_T("[RMS] EPPR build failed. rack=%d rackRecipe=%s valueSource=%s err=%s"),
			rackNo,
			rackRecipeIniPath.GetString(),
			valueSourceIniPath.GetString(),
			errMsg.GetString()
		);
		m_pApp->Gf_writeRMSLog(log);

		CString reply;
		reply.Format(
			_T("EPPR_R ADDR=%s,%s EQP=%s RECIPEINFO=[::[]] ESD= ESDINFO=[] SEQ_NO=%s MMC_TXN_ID=%s"),
			m_strRemoteSubjectRMS,
			m_strLocalSubjectRMS,
			m_strEqpRMS,
			seq_no.GetString(),
			mmc_txn_id.GetString()
		);

		m_pApp->Gf_writeRMSLog(reply);

		pRmsThread->SendTibMessageNoWait((_bstr_t)reply);
		return;
	}

	CString log;
	log.Format(
		_T("[RMS] EPPR build success. rack=%d recipeYN=%s recipeNo=%d paramCount=%d keyFile=%s valueFile=%s"),
		rackNo,
		recipe_yn.GetString(),
		replyRecipeNo,
		paramCount,
		rackRecipeIniPath.GetString(),
		valueSourceIniPath.GetString()
	);
	m_pApp->Gf_writeRMSLog(log);

	CString reply;
	reply.Format(
		_T("EPPR_R ADDR=%s,%s EQP=%s RECIPEINFO=[%s:%s:[%d]:3:U:%s::[0#[%s]]] ESD= ESDINFO=[] SEQ_NO=%s MMC_TXN_ID=%s"),
		m_strRemoteSubjectRMS,
		m_strLocalSubjectRMS,
		m_strEqpRMS,
		eqp_name.Left(eqp_name.GetLength() - 2),
		m_strEqpRMS,
		replyRecipeNo,
		recipe_yn.GetString(),
		recipeMsgSet.GetString(),
		seq_no.GetString(),
		mmc_txn_id.GetString()
	);

	m_pApp->Gf_writeRMSLog(reply);

	VARIANT_BOOL ok = pRmsThread->SendTibMessageNoWait((_bstr_t)reply);
	if (ok == VARIANT_FALSE)
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPPR_R send failed"));
	else
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPPR_R send ok"));
}

// Recipe Parameter Data Send
void CCimNetCommApi::HandleRmsMsg_ERCP(const CString& msg, ICallRMSClass* pRmsThread)
{
	// 0) 안전 체크
	if (pRmsThread == nullptr)
		return;

	CString machine, unit, rcs, mode_code, model, basemodel, wodr, category;
	CString recipe, recipever, oper, nw_cd, nw_description, change_type;
	CString validationinfo, unit_info, reply_req, to_eqp, mmc_txn_id;

	machine			= ExtractFieldValue(msg, _T("MACHINE="));
	unit			= ExtractFieldValue(msg, _T("UNIT="));
	rcs				= ExtractFieldValue(msg, _T("RCS="));
	mode_code		= ExtractFieldValue(msg, _T("MODE_CODE="));
	model			= ExtractFieldValue(msg, _T("MODEL="));
	basemodel		= ExtractFieldValue(msg, _T("BASEMODEL="));
	wodr			= ExtractFieldValue(msg, _T("WODR="));
	category		= ExtractFieldValue(msg, _T("CATEGORY="));
	recipe			= ExtractFieldValue(msg, _T("RECIPE="));
	recipever		= ExtractFieldValue(msg, _T("RECIPEVER="));
	oper			= ExtractFieldValue(msg, _T("OPER="));
	nw_cd			= ExtractFieldValue(msg, _T("NW_CD="));
	nw_description	= ExtractFieldValue(msg, _T("NW_DESCRIPTION="));
	change_type		= ExtractFieldValue(msg, _T("CHANGE_TYPE="));
	validationinfo	= ExtractFieldValue(msg, _T("VALIDATIONINFO="));
	unit_info		= ExtractFieldValue(msg, _T("UNIT_INFO="));
	reply_req		= ExtractFieldValue(msg, _T("REPLY_REQ="));
	to_eqp			= ExtractFieldValue(msg, _T("TO_EQP="));
	mmc_txn_id		= ExtractFieldValue(msg, _T("MMC_TXN_ID="));



	CString reply;
	reply.Format(
		_T("ERCP_R ADDR=%s,%s EQP=%s MACHINE=%s UNIT=%s RCS=%s MODE_CODE=%s MODEL=%s BASEMODEL=%s WODR=%s CATEGORY=%s RECIPE=%s RECIPEVER=%s OPER=%s NW_CD=%s NW_DESCRIPTION=%s CHANGE_TYPE=%s VALIDATIONINFO=%s UNIT_INFO=%s REPLY_REQ=%s TO_EQP=%s RTN_CD=0 ERR_CD=0 ERR_MSG_ENG= ERR_MSG_LOC= MMC_TXN_ID=%s"),
			m_strRemoteSubjectRMS,
			m_strLocalSubjectRMS,
			m_strEqpRMS,
			machine,
			unit,
			rcs,
			mode_code,
			model,
			basemodel,
			wodr,
			category,
			recipe,
			recipever,
			oper,
			nw_cd,
			nw_description,
			change_type,
			validationinfo,
			unit_info,
			reply_req,
			to_eqp,
			mmc_txn_id
	);

	m_pApp->Gf_writeRMSLog(reply);

	VARIANT_BOOL ok = pRmsThread->SendTibMessage((_bstr_t)reply);
	if (ok == VARIANT_FALSE)
		m_pApp->Gf_writeRMSLog(_T("[RMS] ERCP_R send failed"));
	else
		m_pApp->Gf_writeRMSLog(_T("[RMS] ERCP_R send ok"));
}

void CCimNetCommApi::HandleRmsMsg_EPSC(const CString& msg, ICallRMSClass* pRmsThread)
{
	if (pRmsThread == nullptr)
		return;

	CString machine, unit, system, unit_type, operation_type;
	CString command_code, paracount, setting_info, seq_no, mmc_txn_id;

	machine = ExtractFieldValue(msg, _T("MACHINE="));
	unit = ExtractFieldValue(msg, _T("UNIT="));
	system = ExtractFieldValue(msg, _T("SYSTEM="));
	unit_type = ExtractFieldValue(msg, _T("UNIT_TYPE="));
	operation_type = ExtractFieldValue(msg, _T("OPERATION_TYPE="));
	command_code = ExtractFieldValue(msg, _T("COMMAND_CODE="));
	paracount = ExtractFieldValue(msg, _T("PARACOUNT="));

	// nested [] 전체 추출
	setting_info = ExtractFieldValueFullBracket(msg, _T("SETTING_INFO="));

	seq_no = ExtractFieldValue(msg, _T("SEQ_NO="));
	mmc_txn_id = ExtractFieldValue(msg, _T("MMC_TXN_ID="));

	CString reply;

	int rackNo = 0;
	if (!GetRackNoFromUnit(unit, rackNo))
	{
		reply.Format(
			_T("EPSC_R ADDR=%s,%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s CURRENT_RECIPE_YN= COMMAND_CODE=%s PARACOUNT=0 SETTING_INFO=[] ACK=1 ERR_MSG_ENG=Invalid UNIT rack number ERR_MSG_LOC=Invalid UNIT rack number SEQ_NO=%s MMC_TXN_ID=%s"),
			m_strRemoteSubjectRMS,
			m_strLocalSubjectRMS,
			m_strEqpRMS,
			machine,
			unit,
			system,
			unit_type,
			operation_type,
			command_code,
			seq_no,
			mmc_txn_id
		);

		m_pApp->Gf_writeRMSLog(reply);

		VARIANT_BOOL ok = pRmsThread->SendTibMessage((_bstr_t)reply);
		if (ok == VARIANT_FALSE)
			m_pApp->Gf_writeRMSLog(_T("[RMS] EPSC_R send failed"));
		else
			m_pApp->Gf_writeRMSLog(_T("[RMS] EPSC_R send ok"));

		return;
	}

	// 변경된 RMS 폴더 구조 기준
	// .\RMS\RACK3\RACK3_Parameter.ini
	// .\RMS\RACK3\RACK3_Recipe.ini
	CString parameterIniPath = GetRmsRackParameterPath(rackNo);
	CString rackRecipeIniPath = GetRmsRackRecipePath(rackNo);

	if (command_code == _T("I"))
	{
		int paraCount = 0;
		CString stMsg;
		CString errMsg;

		// 중요:
		// 기존 BuildEpscSettingInfoFromParameterIni()는 고정 배열 64개를 기준으로 조회함.
		// 이제는 RACKn_Recipe.ini에 남아 있는 파라미터만 기준으로 조회해야 함.
		BOOL buildOk = BuildEpscSettingInfoFromRackFiles(
			parameterIniPath,
			rackRecipeIniPath,
			stMsg,
			paraCount,
			errMsg
		);

		if (buildOk)
		{
			reply.Format(
				_T("EPSC_R ADDR=%s,%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s CURRENT_RECIPE_YN= COMMAND_CODE=%s PARACOUNT=%d SETTING_INFO=[%s] ACK=0 ERR_MSG_ENG= ERR_MSG_LOC= SEQ_NO=%s MMC_TXN_ID=%s"),
				m_strRemoteSubjectRMS,
				m_strLocalSubjectRMS,
				m_strEqpRMS,
				machine,
				unit,
				system,
				unit_type,
				operation_type,
				command_code,
				paraCount,
				stMsg,
				seq_no,
				mmc_txn_id
			);

			CString log;
			log.Format(
				_T("[EPSC] Rack %d read success. parameterFile=%s recipeFile=%s paraCount=%d"),
				rackNo,
				parameterIniPath.GetString(),
				rackRecipeIniPath.GetString(),
				paraCount
			);
			m_pApp->Gf_writeRMSLog(log);
		}
		else
		{
			reply.Format(
				_T("EPSC_R ADDR=%s,%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s CURRENT_RECIPE_YN= COMMAND_CODE=%s PARACOUNT=0 SETTING_INFO=[] ACK=1 ERR_MSG_ENG=%s ERR_MSG_LOC=%s SEQ_NO=%s MMC_TXN_ID=%s"),
				m_strRemoteSubjectRMS,
				m_strLocalSubjectRMS,
				m_strEqpRMS,
				machine,
				unit,
				system,
				unit_type,
				operation_type,
				command_code,
				errMsg.GetString(),
				errMsg.GetString(),
				seq_no,
				mmc_txn_id
			);

			CString log;
			log.Format(
				_T("[EPSC] Rack %d read failed. parameterFile=%s recipeFile=%s err=%s"),
				rackNo,
				parameterIniPath.GetString(),
				rackRecipeIniPath.GetString(),
				errMsg.GetString()
			);
			m_pApp->Gf_writeRMSLog(log);
		}
	}
	else if (command_code == _T("S"))
	{
		int appliedCount = 0;
		CString errMsg;

		BOOL applied = ApplyEpscSettingInfoToRackFiles(
			parameterIniPath,
			rackRecipeIniPath,
			setting_info,
			appliedCount,
			errMsg
		);

		if (applied)
		{
			reply.Format(
				_T("EPSC_R ADDR=%s,%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s CURRENT_RECIPE_YN= COMMAND_CODE=%s PARACOUNT=%d SETTING_INFO=[] ACK=0 ERR_MSG_ENG= ERR_MSG_LOC= SEQ_NO=%s MMC_TXN_ID=%s"),
				m_strRemoteSubjectRMS,
				m_strLocalSubjectRMS,
				m_strEqpRMS,
				machine,
				unit,
				system,
				unit_type,
				operation_type,
				command_code,
				appliedCount,
				seq_no,
				mmc_txn_id
			);

			CString log;
			log.Format(
				_T("[EPSC] Rack %d update success. parameterFile=%s recipeFile=%s count=%d"),
				rackNo,
				parameterIniPath.GetString(),
				rackRecipeIniPath.GetString(),
				appliedCount
			);
			m_pApp->Gf_writeRMSLog(log);
		}
		else
		{
			reply.Format(
				_T("EPSC_R ADDR=%s,%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s CURRENT_RECIPE_YN= COMMAND_CODE=%s PARACOUNT=%d SETTING_INFO=[] ACK=1 ERR_MSG_ENG=%s ERR_MSG_LOC=%s SEQ_NO=%s MMC_TXN_ID=%s"),
				m_strRemoteSubjectRMS,
				m_strLocalSubjectRMS,
				m_strEqpRMS,
				machine,
				unit,
				system,
				unit_type,
				operation_type,
				command_code,
				appliedCount,
				errMsg.GetString(),
				errMsg.GetString(),
				seq_no,
				mmc_txn_id
			);

			CString log;
			log.Format(
				_T("[EPSC] Rack %d update failed. parameterFile=%s recipeFile=%s err=%s"),
				rackNo,
				parameterIniPath.GetString(),
				rackRecipeIniPath.GetString(),
				errMsg.GetString()
			);
			m_pApp->Gf_writeRMSLog(log);
		}
	}
	else
	{
		reply.Format(
			_T("EPSC_R ADDR=%s,%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s CURRENT_RECIPE_YN= COMMAND_CODE=%s PARACOUNT=%s SETTING_INFO=[] ACK=1 ERR_MSG_ENG=Unsupported COMMAND_CODE ERR_MSG_LOC=Unsupported COMMAND_CODE SEQ_NO=%s MMC_TXN_ID=%s"),
			m_strRemoteSubjectRMS,
			m_strLocalSubjectRMS,
			m_strEqpRMS,
			machine,
			unit,
			system,
			unit_type,
			operation_type,
			command_code,
			paracount,
			seq_no,
			mmc_txn_id
		);
	}

	m_pApp->Gf_writeRMSLog(reply);

	VARIANT_BOOL ok = pRmsThread->SendTibMessage((_bstr_t)reply);
	if (ok == VARIANT_FALSE)
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPSC_R send failed"));
	else
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPSC_R send ok"));
}

void CCimNetCommApi::HandleRmsMsg_EPDC(const CString& msg, ICallRMSClass* pRmsThread)
{
	if (pRmsThread == nullptr)
		return;

	CString addr, eqp, machine, unit, system, unit_type, operation_type;
	CString paracount, delete_info, seq_no, mmc_txn_id;

	addr = ExtractFieldValue(msg, _T("ADDR="));
	eqp = ExtractFieldValue(msg, _T("EQP="));
	machine = ExtractFieldValue(msg, _T("MACHINE="));
	unit = ExtractFieldValue(msg, _T("UNIT="));
	system = ExtractFieldValue(msg, _T("SYSTEM="));
	unit_type = ExtractFieldValue(msg, _T("UNIT_TYPE="));
	operation_type = ExtractFieldValue(msg, _T("OPERATION_TYPE="));
	paracount = ExtractFieldValue(msg, _T("PARACOUNT="));

	// DELETE_INFO=[:[MODEL_NB^DIMMING_SEL^PWM_FREQ...]] 전체 추출
	delete_info = ExtractFieldValueFullBracket_EPDC(msg, _T("DELETE_INFO="));

	seq_no = ExtractFieldValue(msg, _T("SEQ_NO="));
	mmc_txn_id = ExtractFieldValue(msg, _T("MMC_TXN_ID="));

	CString reply;

	// UNIT 끝자리로 Rack 번호 추출
	// 예: UNIT=W4AMAL04HV0101 -> rackNo=1
	int rackNo = 0;

	if (!GetRackNoFromUnit(unit, rackNo))
	{
		reply.Format(
			_T("EPDC_R ADDR=%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s ACK=1 ERR_MSG_ENG=Invalid UNIT rack number ERR_MSG_LOC=Invalid UNIT rack number SEQ_NO=%s MMC_TXN_ID=%s"),
			addr,
			eqp,
			machine,
			unit,
			system,
			unit_type,
			operation_type,
			seq_no,
			mmc_txn_id
		);

		m_pApp->Gf_writeRMSLog(reply);

		VARIANT_BOOL ok = pRmsThread->SendTibMessageNoWait((_bstr_t)reply);
		if (ok == VARIANT_FALSE)
			m_pApp->Gf_writeRMSLog(_T("[RMS] EPDC_R send failed"));
		else
			m_pApp->Gf_writeRMSLog(_T("[RMS] EPDC_R send ok"));

		return;
	}

	// 변경된 폴더 구조 기준
	// .\RMS\RACK1\RACK1_Parameter.ini
	// .\RMS\RACK1\RACK1_Recipe.ini
	CString rackParameterIniPath = GetRmsRackParameterPath(rackNo);
	CString rackRecipeIniPath = GetRmsRackRecipePath(rackNo);

	CString errMsg;

	if (!FileExistsSimple(rackParameterIniPath))
	{
		errMsg.Format(_T("Rack parameter file not found. file=%s"), rackParameterIniPath.GetString());

		reply.Format(
			_T("EPDC_R ADDR=%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s ACK=1 ERR_MSG_ENG=%s ERR_MSG_LOC=%s SEQ_NO=%s MMC_TXN_ID=%s"),
			addr,
			eqp,
			machine,
			unit,
			system,
			unit_type,
			operation_type,
			errMsg.GetString(),
			errMsg.GetString(),
			seq_no,
			mmc_txn_id
		);

		m_pApp->Gf_writeRMSLog(reply);

		VARIANT_BOOL ok = pRmsThread->SendTibMessageNoWait((_bstr_t)reply);
		if (ok == VARIANT_FALSE)
			m_pApp->Gf_writeRMSLog(_T("[RMS] EPDC_R send failed"));
		else
			m_pApp->Gf_writeRMSLog(_T("[RMS] EPDC_R send ok"));

		return;
	}

	if (!FileExistsSimple(rackRecipeIniPath))
	{
		errMsg.Format(_T("Rack recipe file not found. file=%s"), rackRecipeIniPath.GetString());

		reply.Format(
			_T("EPDC_R ADDR=%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s ACK=1 ERR_MSG_ENG=%s ERR_MSG_LOC=%s SEQ_NO=%s MMC_TXN_ID=%s"),
			addr,
			eqp,
			machine,
			unit,
			system,
			unit_type,
			operation_type,
			errMsg.GetString(),
			errMsg.GetString(),
			seq_no,
			mmc_txn_id
		);

		m_pApp->Gf_writeRMSLog(reply);

		VARIANT_BOOL ok = pRmsThread->SendTibMessageNoWait((_bstr_t)reply);
		if (ok == VARIANT_FALSE)
			m_pApp->Gf_writeRMSLog(_T("[RMS] EPDC_R send failed"));
		else
			m_pApp->Gf_writeRMSLog(_T("[RMS] EPDC_R send ok"));

		return;
	}

	int deletedCount = 0;

	BOOL applied = ApplyEpdcDeleteInfoToRackFiles(
		rackParameterIniPath,
		rackRecipeIniPath,
		delete_info,
		deletedCount,
		errMsg
	);

	if (applied)
	{
		reply.Format(
			_T("EPDC_R ADDR=%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s ACK=0 ERR_MSG_ENG= ERR_MSG_LOC= SEQ_NO=%s MMC_TXN_ID=%s"),
			addr,
			eqp,
			machine,
			unit,
			system,
			unit_type,
			operation_type,
			seq_no,
			mmc_txn_id
		);

		CString log;
		log.Format(
			_T("[EPDC] Rack %d delete success. parameterFile=%s recipeFile=%s deletedCount=%d DELETE_INFO=%s"),
			rackNo,
			rackParameterIniPath.GetString(),
			rackRecipeIniPath.GetString(),
			deletedCount,
			delete_info.GetString()
		);
		m_pApp->Gf_writeRMSLog(log);
	}
	else
	{
		reply.Format(
			_T("EPDC_R ADDR=%s EQP=%s MACHINE=%s UNIT=%s SYSTEM=%s UNIT_TYPE=%s OPERATION_TYPE=%s ACK=1 ERR_MSG_ENG=%s ERR_MSG_LOC=%s SEQ_NO=%s MMC_TXN_ID=%s"),
			addr,
			eqp,
			machine,
			unit,
			system,
			unit_type,
			operation_type,
			errMsg.GetString(),
			errMsg.GetString(),
			seq_no,
			mmc_txn_id
		);

		CString log;
		log.Format(
			_T("[EPDC] Rack %d delete failed. parameterFile=%s recipeFile=%s err=%s DELETE_INFO=%s"),
			rackNo,
			rackParameterIniPath.GetString(),
			rackRecipeIniPath.GetString(),
			errMsg.GetString(),
			delete_info.GetString()
		);
		m_pApp->Gf_writeRMSLog(log);
	}

	// EPDC_R 응답 전송
	m_pApp->Gf_writeRMSLog(reply);

	VARIANT_BOOL ok = pRmsThread->SendTibMessageNoWait((_bstr_t)reply);
	if (ok == VARIANT_FALSE)
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPDC_R send failed"));
	else
		m_pApp->Gf_writeRMSLog(_T("[RMS] EPDC_R send ok"));
}

void CCimNetCommApi::HandleRmsMsg_EPCC(const CString& msg, ICallRMSClass* pRmsThread)
{

}



CString CCimNetCommApi::ExtractFieldValue(const CString& msg, LPCTSTR keyWithEq)
{
	CString key(keyWithEq);
	int pos = msg.Find(key);
	if (pos < 0) return _T("");

	int start = pos + key.GetLength();
	if (start >= msg.GetLength()) return _T("");

	// 값 시작 지점에서 공백은 스킵
	while (start < msg.GetLength() && msg[start] == _T(' '))
		start++;

	if (start >= msg.GetLength()) return _T("");

	// 값이 [ 로 시작하면 ] 까지 통째로
	if (msg[start] == _T('['))
	{
		int endBracket = msg.Find(_T(']'), start);
		if (endBracket < 0)
		{
			// 닫는 ]가 없으면 끝까지
			return msg.Mid(start);
		}
		return msg.Mid(start, endBracket - start + 1); // ']' 포함
	}

	// 일반 값: 공백 전까지
	int end = msg.Find(_T(' '), start);
	if (end < 0) end = msg.GetLength();

	return msg.Mid(start, end - start);
}

CString CCimNetCommApi::NormalizeRecipeNo3Digit(const CString& recipe)
{
	CString text = recipe;
	text.Trim();

	// 혹시 "001.ini" 같은 값이 들어와도 방어
	if (text.Right(4).CompareNoCase(_T(".ini")) == 0)
		text = text.Left(text.GetLength() - 4);

	int no = _ttoi(text);

	CString result;
	result.Format(_T("%03d"), no);   // 1 -> 001, 11 -> 011, 123 -> 123
	return result;
}