#include "stdafx.h"
#include "debug.h"

inline void format_timeW(GMWchar* in)
{
#if _WINDOWS
	SYSTEMTIME time = { 0 };
	GetLocalTime(&time);
	swprintf_s(in, LINE_MAX, _L("%d-%02d-%02d %02d:%02d:%02d"),
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
#if _WINDOWS
	SYSTEMTIME time = { 0 };
	GetLocalTime(&time);
	sprintf_s(in, LINE_MAX, "%d-%02d-%02d %02d:%02d:%02d",
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute,
		time.wSecond
	);
#endif
}

#define f_timeW(t) GMWchar t[LINE_MAX]; format_timeW(t);
#define f_timeA(t) char t[LINE_MAX]; format_timeA(t);

#define printW(format, tag) \
	D(d);															\
	GMWchar out[LINE_MAX];											\
	va_list ap;														\
	va_start(ap, format);											\
	if (d->debugger)													\
	{																\
	GMWchar buf[LINE_MAX];											\
	vswprintf(buf, LINE_MAX, format, ap);							\
	f_timeW(t);														\
		wsprintf(out, _L("[") _L(#tag) _L("]%s: %s"), t, buf);		\
		d->debugger->tag(out);										\
	}																\
	else															\
	{																\
		GMWchar buf[LINE_MAX];										\
		vswprintf(buf, LINE_MAX, format, ap);						\
		f_timeW(t);													\
		wprintf(out, _L("[") _L(#tag) _L("]%s: %s"), t, buf);		\
		d->debugger->tag(out);										\
	}																\
	va_end(ap);

#define printA(format, tag) \
	D(d);															\
	char out[LINE_MAX];												\
	va_list ap;														\
	va_start(ap, format);											\
	if (d->debugger)													\
	{																\
		char buf[LINE_MAX];											\
		vsprintf_s(buf, format, ap);								\
		f_timeW(t);													\
		sprintf_s(out, "[" #tag "]%s: %s", t, buf);					\
		d->debugger->tag(out);										\
	}																\
	else															\
	{																\
		char buf[LINE_MAX];											\
		vsprintf_s(buf, format, ap);								\
		f_timeW(t);													\
		printf(out, "[" #tag "]%s: %s", t, buf);					\
		d->debugger->tag(out);										\
	}																\
	va_end(ap);

void GMDebugger::info(const GMWchar *format, ...)
{
	printW(format, info);
}

void GMDebugger::info(const char* format, ...)
{
	printA(format, info);
}

void GMDebugger::error(const GMWchar *format, ...)
{
	printW(format, error);
}

void GMDebugger::error(const char* format, ...)
{
	printA(format, error);
}

void GMDebugger::warning(const GMWchar *format, ...)
{
	printW(format, warning);
}

void GMDebugger::warning(const char* format, ...)
{
	printA(format, warning);
}

#ifdef _DEBUG
void GMDebugger::debug(const GMWchar *format, ...)
{
	printW(format, debug);
}

void GMDebugger::debug(const char* format, ...)
{
	printA(format, debug);
}
#endif
//////////////////////////////////////////////////////////////////////////
Hooks& Hooks::instance()
{
	static Hooks hooks;
	return hooks;
}

void Hooks::invoke0(const GMString& identifier)
{
	auto hooks = instance().m_hooks0[identifier];
	for (auto iter = hooks.begin(); iter != hooks.end(); iter++)
	{
		(*iter)();
	}
}

void Hooks::install(const GMString& identifier, Hook0 hook)
{
	auto& hooks = instance().m_hooks0[identifier];
	hooks.insert(hook);
}

void Hooks::invoke1(const GMString& identifier, void* arg1)
{
	auto hooks = instance().m_hooks1[identifier];
	for (auto iter = hooks.begin(); iter != hooks.end(); iter++)
	{
		(*iter)(arg1);
	}
}

void Hooks::install(const GMString& identifier, Hook1 hook)
{
	auto& hooks = instance().m_hooks1[identifier];
	hooks.insert(hook);
}

void Hooks::invoke2(const GMString& identifier, void* arg1, void* arg2)
{
	auto hooks = instance().m_hooks2[identifier];
	for (auto iter = hooks.begin(); iter != hooks.end(); iter++)
	{
		(*iter)(arg1, arg2);
	}
}

void Hooks::install(const GMString& identifier, Hook2 hook)
{
	auto& hooks = instance().m_hooks2[identifier];
	hooks.insert(hook);
}