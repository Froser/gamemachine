#ifndef __GMUI_DEF_H__
#define __GMUI_DEF_H__

#if !GM_WINDOWS
#error Windows system is required.
#endif

#include <windef.h>

typedef WNDPROC GMUIWindowProc;
typedef HINSTANCE GMUIInstance;
typedef LPCTSTR GMUIStringPtr;
typedef LRESULT LongResult;
typedef WPARAM UintPtr;
typedef LPARAM LongPtr;


#endif