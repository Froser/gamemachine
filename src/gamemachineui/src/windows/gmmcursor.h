#ifndef __GMUICURSOR_H__
#define __GMUICURSOR_H__
#include <gmui.h>
#include <defines.h>
#include <gamemachine.h>

BEGIN_UI_NS

GM_PRIVATE_OBJECT(GMUICursor)
{
	HCURSOR cursor = NULL;
};

class GMUICursor : public gm::IGMCursor
{
	DECLARE_PRIVATE(GMUICursor)

public:
	GMUICursor() = default;

public:
	virtual bool createCursor(void* hinstance, const gm::GMImage& cursorImg) override;
};

END_UI_NS
#endif