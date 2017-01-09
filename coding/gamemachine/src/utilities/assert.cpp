#include "stdafx.h"
#include "assert.h"
#include <sstream>

void Assert::noop()
{
}

void Assert::assert(const char *assertion, const char *file, int line)
{
#if defined(_WINDOWS) && defined(_DEBUG)
	std::wstringstream buf, wfile;
	buf << assertion;
	wfile << file;
	int reportMode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
	_CrtSetReportMode(_CRT_ERROR, reportMode);
	int ret = _CrtDbgReportW(_CRT_ERROR, wfile.str().c_str(),
		line, L"", buf.str().c_str());
	if (ret == 0 && reportMode & _CRTDBG_MODE_WNDW)
		return;
	else if (ret == 1)
		_CrtDbgBreak();
#endif
}

void Assert::log_assert(const char *assertion, const char *file, int line)
{
#if defined(_WINDOWS) && defined(_DEBUG)
	char output[512] = "Assertion failed: ";
	strcat(output, assertion);
	strcat(output, "\r\n file:");
	strcat(output, file);
	strcat(output, "\r\n line:");

	char strLine[10];
	sprintf(strLine, "%d", line);
	strcat(output, strLine);

	LOG_WARNING(output);
#endif
}

void Assert::log_assert(const char *assertion, const char* msg, const char *file, int line)
{
#if defined(_WINDOWS) && defined(_DEBUG)
	char output[512] = "Assertion failed: ";
	strcat(output, assertion);
	strcat(output, "\r\n");
	strcat(output, msg);
	strcat(output, "\r\n file:");
	strcat(output, file);
	strcat(output, "\r\n line:");

	char strLine[10];
	sprintf(strLine, "%d", line);
	strcat(output, strLine);

	LOG_WARNING(output);
#endif
}

void Assert::log_assert(const char *assertion, const std::string& msg, const char *file, int line)
{
	log_assert(assertion, msg.c_str(), file, line);
}