#ifndef ANTILEECH_WX_H
#define ANTILEECH_WX_H

#include <wx/defs.h>
#include <string.h>

#define LPCTSTR		const wxChar* 
#define BOOL		bool
//#define _T(var)		wxT(var)	//defined in wxWidgets
#define DWORD		wxUint32
#define UINT		wxUint16
#define WINAPI
#define HINSTANCE
#define LPVOID		void*
#define PBYTE		unsigned char*
//#define wchar_t
#define TCHAR		wxChar
#define _TINT		wxInt32

#define StrCmpI _tcsicmp

#define _istdigit(var)		iswdigit(var)
#define _istcntrl(var)		iswcntrl(var)
#define _istpunct(var)		iswpunct(var)
#define _istspace(var)		iswspace(var)
#define _istxdigit(var)		iswxdigit(var)
inline float _tstof(const wchar_t* str){
	wchar_t** ptail = NULL;
	return wcstof(str, ptail);
}
inline void tolowers(wxChar* str){
	int i = 0;
	do{
		str[i] = towlower(str[i]);
	}while(str[++i]);
}
#define _tcsstr(haystack, needle)	wcsstr(haystack, needle)
#define _tcslen(var)		wcslen(var)
#define StrStr(a, b)		wcsstr(a, b)
inline LPCTSTR StrStrI(LPCTSTR haystack, LPCTSTR needle){
	//wchar_t* haystacki = new wchar_t[wcslen(haystack) + 1];
	//wchar_t* needlei = new wchar_t[wcslen(needle) + 1];
	wchar_t* haystacki = new wchar_t[512];
	wchar_t* needlei = new wchar_t[512];
	int i = 0;
	do{
		haystacki[i] = towlower(haystack[i]);
		if(i = 511)
			break;
	}while(haystack[++i]);
	i = 0;
	do{
		needlei[i] = towlower(needle[i]);
		if(i = 511)
			break;
	}while(needle[++i]);
	const wchar_t* ret = wcsstr(haystacki, needlei);
	if(ret != NULL)
		ret = ret - haystacki + haystack;
	delete[] haystacki;
	delete[] needlei;
	return ret;
}
#define _tcsicmp(a, b)		wcscasecmp(a, b)

/*
//greansea
int _istcntrl(wxChar c);
int _istpunct(wxChar c);
int _istspace(wxChar c);
int _istdigit(wxChar c);
int _istxdigit(wxChar c);
double _tstof(LPCTSTR str);
void tolowers(wxChar* str);
LPCTSTR _tcsstr(LPCTSTR a, LPCTSTR b);
size_t _tcslen(LPCTSTR str);
LPCTSTR StrStr(LPCTSTR a, LPCTSTR b);
LPCTSTR StrStrI(LPCTSTR a, LPCTSTR b);
int _tcsicmp(LPCTSTR a, LPCTSTR b);
*/

#endif
