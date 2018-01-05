#undef __STRICT_ANSI__
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include "stdafx.h"
#include "debug.h"
#include <cwchar>

inline void format_timeW(GMwchar* in)
{
#if GM_WINDOWS
	SYSTEMTIME time = { 0 };
	GetLocalTime(&time);
	swprintf(in, L"%d-%02d-%02d %02d:%02d:%02d",
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute,
		time.wSecond
	);
#endif
}

inline void format_timeA(char* in)
{
#if GM_WINDOWS
	SYSTEMTIME time = { 0 };
	GetLocalTime(&time);
	sprintf(in, "%d-%02d-%02d %02d:%02d:%02d",
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute,
		time.wSecond
	);
#endif
}

#define f_timeW(t) GMwchar t[LINE_MAX]; format_timeW(t);
#define f_timeA(t) char t[LINE_MAX]; format_timeA(t);

#define printW(format, tag) \
	D(d);															\
	GMwchar out[LINE_MAX];											\
	va_list ap;														\
	va_start(ap, format);											\
	if (d && d->debugger)											\
	{																\
		GMwchar buf[LINE_MAX];										\
		vswprintf(buf, format, ap);									\
		f_timeW(t);													\
		wsprintf(out, L"[" L ## #tag L"]%s: %s", t, buf);			\
		d->debugger->tag(out);										\
	}																\
	else															\
	{																\
		GMwchar buf[LINE_MAX];										\
		vswprintf(buf, format, ap);									\
		f_timeW(t);													\
		wprintf(out, L"[" L ## #tag L"]%s: %s", t, buf);			\
	}																\
	va_end(ap);

#define printA(format, tag) \
	D(d);															\
	char out[LINE_MAX];												\
	va_list ap;														\
	va_start(ap, format);											\
	if (d && d->debugger)											\
	{																\
		char buf[LINE_MAX];											\
		vsprintf(buf, format, ap);									\
		f_timeA(t);													\
		sprintf(out, "[" #tag "]%s: %s", t, buf);					\
		d->debugger->tag(out);										\
	}																\
	else															\
	{																\
		char buf[LINE_MAX];											\
		vsprintf(buf, format, ap);									\
		f_timeA(t);													\
		printf(out, "[" #tag "]%s: %s", t, buf);					\
	}																\
	va_end(ap);

void GMDebugger::info(const GMwchar *format, ...)
{
	printW(format, info);
}

void GMDebugger::info(const char* format, ...)
{
	printA(format, info);
}

void GMDebugger::error(const GMwchar *format, ...)
{
	printW(format, error);
}

void GMDebugger::error(const char* format, ...)
{
	printA(format, error);
}

void GMDebugger::warning(const GMwchar *format, ...)
{
	printW(format, warning);
}

void GMDebugger::warning(const char* format, ...)
{
	printA(format, warning);
}

#ifdef _DEBUG
void GMDebugger::debug(const GMwchar *format, ...)
{
	printW(format, debug);
}

void GMDebugger::debug(const char* format, ...)
{
	printA(format, debug);
}
#endif

Map<size_t, void*> HookFactory::g_hooks;
