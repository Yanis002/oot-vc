#ifndef _WSTRING_H
#define _WSTRING_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t wcslen(const wchar_t*);
wchar_t* wcscpy(wchar_t*, const wchar_t*);
wchar_t* wcsncpy(wchar_t*, const wchar_t*, size_t);
wchar_t* wcscat(wchar_t*, const wchar_t*);
int wcscmp(const wchar_t*, const wchar_t*);
wchar_t* wcschr(const wchar_t*, wchar_t);

#ifdef __cplusplus
}
#endif

#endif
