#include "stdafx.h"
#include "assert.h"
#include <sstream>

BEGIN_NS
#if GM_WINDOWS
void Assert::noop()
{
}

void Assert::gm_assert(const char *assertion, const char *file, int line)
{
#if defined(GM_WINDOWS) && defined(GM_DEBUG)
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
END_NS
#endif