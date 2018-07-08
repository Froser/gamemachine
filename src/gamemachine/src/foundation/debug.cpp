#undef __STRICT_ANSI__
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include "stdafx.h"
#include "debug.h"
#include <cwchar>
#include <chrono>
#include <ctime>

namespace
{
	GMString getFormattedTime()
	{
		auto now = std::chrono::system_clock::now();
		std::time_t std_now = std::chrono::system_clock::to_time_t(now);
		std::tm* localTime = std::localtime(&std_now);
		std::string r = std::asctime(localTime);
		return r.substr(0, r.length() - 1);
	}
}

void GMDebugger::info(const GMString& string, const std::initializer_list<GMString>& arguments)
{
	print(string, L"info", &(IDebugOutput::info), arguments);
}

void GMDebugger::warning(const GMString& string, const std::initializer_list<GMString>& arguments)
{
	print(string, L"warning", &(IDebugOutput::warning), arguments);
}

void GMDebugger::debug(const GMString& string, const std::initializer_list<GMString>& arguments)
{
#if _DEBUG
	print(string, L"debug", &(IDebugOutput::debug), arguments);
#endif
}

void GMDebugger::error(const GMString& string, const std::initializer_list<GMString>& arguments)
{
	print(string, L"error", &(IDebugOutput::error), arguments);
}

void GMDebugger::print(
	GMString string,
	const GMString& prefix,
	IDebugOutput::Method method,
	const std::initializer_list<GMString>& arguments)
{
	D(d);
	GMint i = 0;
	for (decltype(auto) argument : arguments)
	{
		string = string.replace(L"{" + GMString(i++) + L"}", argument);
	}
	GMString s = getFormattedTime() + L": [" + prefix + L"] " + std::move(string);
	s += "\n";

	if (d->debugger)
	{
		(d->debugger->*method)(s);
	}
	else
	{
#if GM_WINDOWS
		OutputDebugStringW(s.toStdWString().c_str());
#endif
	}
}

Map<GMsize_t, void*> HookFactory::g_hooks;
