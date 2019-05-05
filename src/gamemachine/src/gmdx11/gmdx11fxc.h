#ifndef __GMDX11FXC_H__
#define __GMDX11FXC_H__
#include <gmcommon.h>
BEGIN_NS

#if !GM_WINDOWS
#error This file is only used in Windows.
#endif

struct GMDx11FXCDescription
{
	GMString fxc;
	GMString code;
	GMString tempDir;
	GMint32 optimizationLevel; // from 0-3
	bool treatWarningsAsErrors;
	bool debug;
};

class GMDx11FXC
{
public:
	void init();
	bool compile(const GMDx11FXCDescription&);

public:
	inline bool isAvailable() GM_NOEXCEPT
	{
		return m_available;
	}

private:
	bool findFXC();
	bool fillDescription(GMDx11FXCDescription*);

private:
	GMString m_fxc;
	bool m_available = false;
};

END_NS
#endif