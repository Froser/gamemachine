#ifndef __GM_LUA_GMGLYPHMANAGER_META_H__
#define __GM_LUA_GMGLYPHMANAGER_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmglyphmanager.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(GMGlyphManagerProxy);
	class GMGlyphManagerProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMGlyphManager, GMObjectProxy)
		GM_DECLARE_PRIVATE(GMGlyphManagerProxy)
		GM_DECLARE_BASE(GMObjectProxy)

	public:
		GMGlyphManagerProxy(GMLuaCoreState* l, GMObject* handler = nullptr);
		~GMGlyphManagerProxy();

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif