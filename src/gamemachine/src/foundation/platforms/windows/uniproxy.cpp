#include "stdafx.h"
#include "foundation/gmunibuffer.h"
#include <usp10.h>

GM_PRIVATE_OBJECT(GMUniProxy_Windows)
{
	SCRIPT_STRING_ANALYSIS analysis = 0;
	bool analyseRequired = true;
	GMUniBuffer* unibuffer = nullptr;
	const IRenderContext* context = nullptr;
};

class GMUniProxy_Windows : public IUniProxy
{
	GM_DECLARE_PRIVATE(GMUniProxy_Windows)

public:
	GMUniProxy_Windows(GMUniBuffer* buf);
	~GMUniProxy_Windows();

public:
	virtual void setContext(const IRenderContext* context) override;
	virtual bool analyze() override;
	virtual bool CPtoX(GMint cp, bool trail, GMint* x) override;
	virtual bool XtoCP(GMint x, GMint* cp, bool* trail) override;
	virtual void getPriorItemPos(GMint cP, GMint* prior) override;
	virtual void getNextItemPos(GMint cP, GMint* prior) override;
	virtual void setDirty() override;

	// Dll Handle
private:
	static void initialize();
	static void uninitialize();

	typedef HRESULT(WINAPI* FuncScriptApplyDigitSubstitution)(
		const SCRIPT_DIGITSUBSTITUTE*,
		SCRIPT_CONTROL*,
		SCRIPT_STATE*
		);

	typedef HRESULT(WINAPI* FuncScriptStringAnalyse)(
		HDC,
		const void*,
		int,
		int,
		int,
		DWORD,
		int,
		SCRIPT_CONTROL*,
		SCRIPT_STATE*,
		const int*,
		SCRIPT_TABDEF*,
		const BYTE*,
		SCRIPT_STRING_ANALYSIS*
		);

	typedef HRESULT(WINAPI* FuncScriptStringCPtoX)(
		SCRIPT_STRING_ANALYSIS,
		int,
		BOOL,
		int*
		);

	typedef HRESULT(WINAPI* FuncScriptStringXtoCP)(
		SCRIPT_STRING_ANALYSIS,
		int,
		int*,
		int*
		);

	typedef HRESULT(WINAPI* FuncScriptStringFree)(
		SCRIPT_STRING_ANALYSIS*
		);

	typedef const SCRIPT_LOGATTR* (WINAPI* FuncScriptString_pLogAttr)(
		SCRIPT_STRING_ANALYSIS
		);

	typedef const int* (WINAPI* FuncScriptString_pcOutChars)(
		SCRIPT_STRING_ANALYSIS
		);

	static FuncScriptApplyDigitSubstitution GM_ScriptApplyDigitSubstitution;
	static FuncScriptStringAnalyse GM_ScriptStringAnalyse;
	static FuncScriptStringCPtoX GM_ScriptStringCPtoX;
	static FuncScriptStringXtoCP GM_ScriptStringXtoCP;
	static FuncScriptStringFree GM_ScriptStringFree;
	static FuncScriptString_pLogAttr GM_ScriptString_pLogAttr;
	static FuncScriptString_pcOutChars GM_ScriptString_pcOutChars;

	static HRESULT WINAPI Dummy_ScriptApplyDigitSubstitution(
		const SCRIPT_DIGITSUBSTITUTE*,
		SCRIPT_CONTROL*,
		SCRIPT_STATE*
	)
	{
		return E_NOTIMPL;
	}

	static HRESULT WINAPI Dummy_ScriptStringAnalyse(
		HDC,
		const void*,
		int,
		int,
		int,
		DWORD,
		int,
		SCRIPT_CONTROL*,
		SCRIPT_STATE*,
		const int*,
		SCRIPT_TABDEF*,
		const BYTE*,
		SCRIPT_STRING_ANALYSIS*
	)
	{
		return E_NOTIMPL;
	}

	static HRESULT WINAPI Dummy_ScriptStringCPtoX(
		SCRIPT_STRING_ANALYSIS,
		int,
		BOOL,
		int*
	)
	{
		return E_NOTIMPL;
	}

	static HRESULT WINAPI Dummy_ScriptStringXtoCP(
		SCRIPT_STRING_ANALYSIS,
		int,
		int*,
		int*
	)
	{
		return E_NOTIMPL;
	}

	static HRESULT WINAPI Dummy_ScriptStringFree(
		SCRIPT_STRING_ANALYSIS*
	)
	{
		return E_NOTIMPL;
	}

	static const SCRIPT_LOGATTR*  WINAPI Dummy_ScriptString_pLogAttr(
		SCRIPT_STRING_ANALYSIS
	)
	{
		return NULL;
	}

	static const int*  WINAPI Dummy_ScriptString_pcOutChars(
		SCRIPT_STRING_ANALYSIS
	)
	{
		return 0;
	}

private:
	static HMODULE s_dll;
	static GMint s_initCount;
};

HMODULE GMUniProxy_Windows::s_dll = 0;
GMint GMUniProxy_Windows::s_initCount = 0;
GMUniProxy_Windows::FuncScriptApplyDigitSubstitution GMUniProxy_Windows::GM_ScriptApplyDigitSubstitution = GMUniProxy_Windows::Dummy_ScriptApplyDigitSubstitution;
GMUniProxy_Windows::FuncScriptStringAnalyse GMUniProxy_Windows::GM_ScriptStringAnalyse = GMUniProxy_Windows::Dummy_ScriptStringAnalyse;
GMUniProxy_Windows::FuncScriptStringCPtoX GMUniProxy_Windows::GM_ScriptStringCPtoX = GMUniProxy_Windows::Dummy_ScriptStringCPtoX;
GMUniProxy_Windows::FuncScriptStringXtoCP GMUniProxy_Windows::GM_ScriptStringXtoCP = GMUniProxy_Windows::Dummy_ScriptStringXtoCP;
GMUniProxy_Windows::FuncScriptStringFree GMUniProxy_Windows::GM_ScriptStringFree = GMUniProxy_Windows::Dummy_ScriptStringFree;
GMUniProxy_Windows::FuncScriptString_pLogAttr GMUniProxy_Windows::GM_ScriptString_pLogAttr = GMUniProxy_Windows::Dummy_ScriptString_pLogAttr;
GMUniProxy_Windows::FuncScriptString_pcOutChars GMUniProxy_Windows::GM_ScriptString_pcOutChars = GMUniProxy_Windows::Dummy_ScriptString_pcOutChars;

inline void GM_GetProcAddress(HMODULE module, LPCSTR procName, OUT void** address)
{
	*address = (FARPROC)::GetProcAddress(module, procName);
}

GMUniProxy_Windows::GMUniProxy_Windows(GMUniBuffer* buf)
{
	initialize();
	D(d);
	d->unibuffer = buf;
}

GMUniProxy_Windows::~GMUniProxy_Windows()
{
	uninitialize();
}

void GMUniProxy_Windows::initialize()
{
	++s_initCount;
	if (!s_dll)
	{
		s_dll = ::LoadLibrary(L"usp10.dll");
		if (!s_dll)
			return;

		GM_GetProcAddress(s_dll, "ScriptApplyDigitSubstitution", (void**)&GMUniProxy_Windows::GM_ScriptApplyDigitSubstitution);
		GM_GetProcAddress(s_dll, "ScriptStringAnalyse", (void**)&GMUniProxy_Windows::GM_ScriptStringAnalyse);
		GM_GetProcAddress(s_dll, "ScriptStringCPtoX", (void**)&GMUniProxy_Windows::GM_ScriptStringCPtoX);
		GM_GetProcAddress(s_dll, "ScriptStringXtoCP", (void**)&GMUniProxy_Windows::GM_ScriptStringXtoCP);
		GM_GetProcAddress(s_dll, "ScriptStringFree", (void**)&GMUniProxy_Windows::GM_ScriptStringFree);
		GM_GetProcAddress(s_dll, "ScriptString_pLogAttr", (void**)&GMUniProxy_Windows::GM_ScriptString_pLogAttr);
		GM_GetProcAddress(s_dll, "ScriptString_pcOutChars", (void**)&GMUniProxy_Windows::GM_ScriptString_pcOutChars);
	}
}

void GMUniProxy_Windows::uninitialize()
{
	--s_initCount;
	if (!s_initCount)
	{
		::FreeLibrary(s_dll);
		s_dll = 0;
	}
}

void GMUniProxy_Windows::setContext(const IRenderContext* context)
{
	D(d);
	d->context = context;
}

bool GMUniProxy_Windows::analyze()
{
	D(d);
	if (d->analysis)
		GM_ScriptStringFree(&d->analysis);

	SCRIPT_CONTROL scriptControl;
	SCRIPT_STATE scriptState;
	GM_ZeroMemory(&scriptControl, sizeof(scriptControl));
	GM_ZeroMemory(&scriptState, sizeof(scriptState));
	if (FAILED(GM_ScriptApplyDigitSubstitution(NULL, &scriptControl, &scriptState)))
		return false;

	HDC hDC = GetDC(d->context->getWindow()->getWindowHandle());
	const GMString& buffer = d->unibuffer->getBuffer();
	HRESULT hr = GM_ScriptStringAnalyse(
		hDC,
		buffer.toStdWString().c_str(),
		buffer.length() + 1, //将NULL计算在内
		buffer.length() * 3 / 2 + 16, //MSDN推荐大小
		-1, //UNICODE
		SSA_BREAK | SSA_GLYPHS | SSA_FALLBACK | SSA_LINK,
		0,
		&scriptControl,
		&scriptState,
		NULL,
		NULL,
		NULL,
		&d->analysis
	);
	if (SUCCEEDED(hr))
		d->analyseRequired = false;
	return SUCCEEDED(hr);
}

bool GMUniProxy_Windows::CPtoX(GMint cp, bool trail, GMint* x)
{
	GM_ASSERT(x);
	*x = 0;

	D(d);
	bool succeed = true;
	if (d->analyseRequired)
		succeed = analyze();

	if (succeed)
	{
		HRESULT hr = GM_ScriptStringCPtoX(d->analysis, cp, trail ? TRUE : FALSE, x);
		return SUCCEEDED(hr);
	}

	return succeed;
}

bool GMUniProxy_Windows::XtoCP(GMint x, GMint* cp, bool* trail)
{
	GM_ASSERT(cp && trail);
	*cp = 0;
	*trail = false;

	D(d);
	bool succeed = true;
	if (d->analyseRequired)
		succeed = analyze();

	GMint len = (GMint)d->unibuffer->getBuffer().length();
	if (*cp == -1 && *trail)
	{
		*cp = 0;
		*trail = false;
	}
	else if (*cp > len && !*trail)
	{
		*cp = len;
		*trail = true;
	}
	return succeed;
}

void GMUniProxy_Windows::getPriorItemPos(GMint cp, GMint* prior)
{
	D(d);
	*prior = cp;
	if (d->analyseRequired)
	{
		if (!analyze())
			return;
	}

	const SCRIPT_LOGATTR* logAttr = GM_ScriptString_pLogAttr(d->analysis);
	if (!logAttr)
		return;

	const GMint* pInitial = GM_ScriptString_pcOutChars(d->analysis);
	if (!pInitial)
		return;

	GMint initial = *pInitial;
	if (cp - 1 < initial)
		initial = cp - 1;
	for (GMint i = initial; i > 0; --i)
	{
		if (logAttr[i].fWordStop ||
			(!logAttr[i].fWhiteSpace && logAttr[i - 1].fWhiteSpace))
		{
			*prior = i;
			return;
		}
	}
	*prior = 0;
}

void GMUniProxy_Windows::getNextItemPos(GMint cp, GMint* prior)
{
	D(d);
	*prior = cp;
	if (d->analyseRequired)
	{
		if (!analyze())
			return;
	}

	const SCRIPT_LOGATTR* logAttr = GM_ScriptString_pLogAttr(d->analysis);
	if (!logAttr)
		return;

	const GMint* pInitial = GM_ScriptString_pcOutChars(d->analysis);
	if (!pInitial)
		return;

	GMint initial = *pInitial;
	if (cp + 1 < initial)
		initial = cp + 1;

	GMint i = initial;
	GMint limit = *pInitial;
	while (limit > 0 && i < limit - 1)
	{
		if (logAttr[i].fWordStop)
		{
			*prior = i;
			return;
		}
		else if (logAttr[i].fWhiteSpace && !logAttr[i + 1].fWhiteSpace)
		{
			*prior = i + 1;
			return;
		}

		++i;
	}
	*prior = *pInitial - 1;
}

void GMUniProxy_Windows::setDirty()
{
	D(d);
	d->analyseRequired = true;
}

void GMUniProxyFactory::createUniProxy(GMUniBuffer* buffer, OUT IUniProxy** proxy)
{
	*proxy = new GMUniProxy_Windows(buffer);
}