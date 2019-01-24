#ifndef __GM_LUA_GMOBJECT_META_H__
#define __GM_LUA_GMOBJECT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMObjectProxy)
	{
		GM_LUA_PROXY(GMObject);
		GM_LUA_PROXY_FUNC(connect);
	};

	class GMObjectProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMObjectProxy, GMObject)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif