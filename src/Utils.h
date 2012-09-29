#pragma once

#define STRT2A(str) \
   ((LPCSTR)CStringA(CString(str)))

#define STRA2T(str) \
   ((LPCTSTR)CString(CStringA(str)))

void TokenizeString(LPCTSTR str, LPCTSTR sep, CStringArray& toks);

void RemoveDuplicates(CStringArray& arr);

int StringWordMatch(LPCTSTR strA, LPCTSTR strB);
