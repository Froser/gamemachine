#include "stdafx.h"
#include "log.h"
#include <iostream>

Log& Log::getInstance()
{
	static Log s_log;
	return s_log;
}

Log::Log()
{

}

void Log::time(char* in)
{
#ifdef _WINDOWS
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

void Log::info(const char* output)
{
	char tm[64];
	time(tm);

	std::cout << "INFO [";
	std::cout << tm;
	std::cout << "] ";
	std::cout << output << std::endl;
}

void Log::warning(const char* output)
{
	char tm[64];
	time(tm);

	std::cout << "WARNING [";
	std::cout << tm;
	std::cout << "] ";
	std::cout << output << std::endl;
}