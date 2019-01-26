#ifndef __GM_LUA_GMGLYPHMANAGER_META_H__
#define __GM_LUA_GMGLYPHMANAGER_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmglyphmanager.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMGlyphManagerProxy)
	{
		GM_LUA_PROXY_FUNC(addFontByMemory);
		GM_LUA_PROXY_FUNC(setEN);
		GM_LUA_PROXY_FUNC(setCN);
	};

	class GMGlyphManagerProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMGlyphManager, GMObjectProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMGlyphManagerProxy, GMObjectProxy)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif