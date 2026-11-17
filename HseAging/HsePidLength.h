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

    // PID length alone does not prevent mixed RACK/PID barcode data.
    // Do not truncate invalid scans; reject them before PID save/PCHK/MES/Summary.
    inline bool Valid(const CString& value)
    {
        CString pid(value);
        pid.Trim();
        if (pid.IsEmpty() || pid.GetLength() != Required())
            return false;

        CString upper(pid);
        upper.MakeUpper();
        // RACK01CH64P88S can be 14 characters, but it is never a product PID.
        if (upper.Find(_T("RACK")) >= 0)
            return false;

        // Reject pasted spaces, separators, control characters, etc.
        for (int i = 0; i < upper.GetLength(); ++i)
        {
            const TCHAR c = upper[i];
            if (!((c >= _T('A') && c <= _T('Z')) ||
                (c >= _T('0') && c <= _T('9'))))
                return false;
        }
        return true;
    }
}
