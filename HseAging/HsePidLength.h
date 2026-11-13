#pragma once
// PID LENGTH shared policy (HseAging MFC)
// Include AFTER HseAging.h / IniProfile declarations.
// The system INI location is resolved by the project's existing Read_SysIniFile.
namespace HsePidLength
{
    const int kDefault = 14;
    const int kMin = 1;
    const int kMax = 64;

    inline bool ParseSetting(const CString& value, int& parsed)
    {
        CString s(value);
        s.Trim();
        if (s.IsEmpty() || s.GetLength() > 3) return false;
        for (int i = 0; i < s.GetLength(); ++i)
            if (s[i] < _T('0') || s[i] > _T('9')) return false;
        int n = _ttoi(s);
        if (n < kMin || n > kMax) return false;
        parsed = n;
        return true;
    }

    inline int Required()
    {
        CString configured;
        Read_SysIniFile(_T("SYSTEM"), _T("PID_LENGTH"), &configured);
        int length = kDefault;
        if (ParseSetting(configured, length)) return length;
        return kDefault;
    }

    inline bool Valid(const CString& value)
    {
        CString pid(value);
        pid.Trim();
        return !pid.IsEmpty() && pid.GetLength() == Required();
    }
}
