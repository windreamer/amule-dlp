//Author:	greensea <gs@bbxy.net>
#include "antiLeech_wx.h" //Modified by Bill Lee.

int _istcntrl(wxChar c){
	return ((c >= 0x00 && c <= 0x1F) || c == 0x7F) ? 1 : 0;
}
int _istpunct(wxChar c){
	return ispunct(c);
}
int _istspace(wxChar c){
	return isspace(c);
}
int _istdigit(wxChar c){
	return isdigit(c);
}
int _istxdigit(wxChar c){
	return isxdigit(c);
}
double _tstof(LPCTSTR str){
	return atof(str);
}

void tolowers(wxChar* str){
	int i = 0;
	while (str[i] != 0){
		str[i] = tolower(str[i]);
		i++;
	}
}

LPCTSTR _tcsstr(LPCTSTR a, LPCTSTR b){
	return strstr(a, b);
}

size_t _tcslen(LPCTSTR str){
	return strlen(str);
}

LPCTSTR StrStr(LPCTSTR a, LPCTSTR b){
	return strstr(a, b);
}

LPCTSTR StrStrI(LPCTSTR a, LPCTSTR b){
	wxChar aa[501] = {0};
	wxChar bb[501] = {0};
	LPCTSTR ret;

	strncpy(aa, a, 500);
	strncpy(bb, b, 500);

	tolowers(aa);
	tolowers(bb);
	ret = strstr(aa, bb);
	
	if (ret != NULL) {
		ret = (LPCTSTR)(ret - aa + a);
	}

	return ret;	
}

int _tcsicmp(LPCTSTR a, LPCTSTR b){
	wxChar aa[501] = {0};
	wxChar bb[501] = {0};
	int ret;
	
	strncpy(aa, a, 500); strncpy(bb, b, 500);
	tolowers(aa); tolowers(bb);
	
	ret = strcmp(aa, bb);

	return ret;
}
