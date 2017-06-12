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

static inline void printA(const char* format, ...)
{
	f_timeA(t);
	printf("%s: ", t);
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("\n");
}

void GMDebugger::info(const GMWchar *format, ...)
{
	D(d);
	if (d->debugger)
	{
		GMWchar buf[LINE_MAX];
		f_timeW(t);
		wsprintf(buf, _L("%Ls: "), t);
		va_list ap;
		va_start(ap, format);
		vswprintf(buf, LINE_MAX, format, ap);
		va_end(ap);
		d->debugger->info(buf);
	}
	else
	{
		f_timeW(t);
		wprintf(_L("%Ls: "), t);
		va_list ap;
		va_start(ap, format);
		vwprintf(format, ap);
		va_end(ap);
		wprintf(_L("\n"));
	}
}

void GMDebugger::info(const char* format, ...)
{
	D(d);
	if (d->debugger)
	{
		char buf[LINE_MAX];
		f_timeA(t);
		sprintf_s(buf, "%s: ", t);
		va_list ap;
		va_start(ap, format);
		vsprintf_s(buf, format, ap);
		va_end(ap);
		d->debugger->info(buf);
	}
	else
	{
		f_timeA(t);
		printf("%s: ", t);
		va_list ap;
		va_start(ap, format);
		vprintf(format, ap);
		va_end(ap);
		printf("\n");
	}
}

void GMDebugger::error(const GMWchar *format, ...)
{
	D(d);
	if (d->debugger)
	{
		GMWchar buf[LINE_MAX];
		f_timeW(t);
		wsprintf(buf, _L("%Ls: "), t);
		va_list ap;
		va_start(ap, format);
		vswprintf(buf, LINE_MAX, format, ap);
		va_end(ap);
		d->debugger->error(buf);
	}
	else
	{
		f_timeW(t);
		wprintf(_L("%Ls: "), t);
		va_list ap;
		va_start(ap, format);
		vwprintf(format, ap);
		va_end(ap);
		wprintf(_L("\n"));
	}
}

void GMDebugger::error(const char* format, ...)
{
	D(d);
	if (d->debugger)
	{
		char buf[LINE_MAX];
		f_timeA(t);
		sprintf_s(buf, "%s: ", t);
		va_list ap;
		va_start(ap, format);
		vsprintf_s(buf, format, ap);
		va_end(ap);
		d->debugger->error(buf);
	}
	else
	{
		f_timeA(t);
		printf("%s: ", t);
		va_list ap;
		va_start(ap, format);
		vprintf(format, ap);
		va_end(ap);
		printf("\n");
	}
}

void GMDebugger::warning(const GMWchar *format, ...)
{
	D(d);
	if (d->debugger)
	{
		GMWchar buf[LINE_MAX];
		f_timeW(t);
		wsprintf(buf, _L("%Ls: "), t);
		va_list ap;
		va_start(ap, format);
		vswprintf(buf, LINE_MAX, format, ap);
		va_end(ap);
		d->debugger->warning(buf);
	}
	else
	{
		f_timeW(t);
		wprintf(_L("%Ls: "), t);
		va_list ap;
		va_start(ap, format);
		vwprintf(format, ap);
		va_end(ap);
		wprintf(_L("\n"));
	}
}

void GMDebugger::warning(const char* format, ...)
{
	D(d);
	if (d->debugger)
	{
		char buf[LINE_MAX];
		f_timeA(t);
		sprintf_s(buf, "%s: ", t);
		va_list ap;
		va_start(ap, format);
		vsprintf_s(buf, format, ap);
		va_end(ap);
		d->debugger->warning(buf);
	}
	else
	{
		f_timeA(t);
		printf("%s: ", t);
		va_list ap;
		va_start(ap, format);
		vprintf(format, ap);
		va_end(ap);
		printf("\n");
	}
}

#ifdef _DEBUG
void GMDebugger::debug(const GMWchar *format, ...)
{
	D(d);
	if (d->debugger)
	{
		GMWchar buf[LINE_MAX];
		f_timeW(t);
		wsprintf(buf, _L("%Ls: "), t);
		va_list ap;
		va_start(ap, format);
		vswprintf(buf, LINE_MAX, format, ap);
		va_end(ap);
		d->debugger->debug(buf);
	}
	else
	{
		f_timeW(t);
		wprintf(_L("%Ls: "), t);
		va_list ap;
		va_start(ap, format);
		vwprintf(format, ap);
		va_end(ap);
		wprintf(_L("\n"));
	}
}

void GMDebugger::debug(const char* format, ...)
{
	D(d);
	if (d->debugger)
	{
		char buf[LINE_MAX];
		f_timeA(t);
		sprintf_s(buf, "%s: ", t);
		va_list ap;
		va_start(ap, format);
		vsprintf_s(buf, format, ap);
		va_end(ap);
		d->debugger->debug(buf);
	}
	else
	{
		f_timeA(t);
		printf("%s: ", t);
		va_list ap;
		va_start(ap, format);
		vprintf(format, ap);
		va_end(ap);
		printf("\n");
	}
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