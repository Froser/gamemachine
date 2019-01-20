#ifndef __GM_LUA_GMBUFFER_META_H__
#define __GM_LUA_GMBUFFER_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMBufferProxy)
	{
		GM_LUA_PROXY(GMBuffer);
		GM_LUA_PROXY_FUNC(__gc);
	};

	class GMBufferProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMBufferProxy, GMBuffer)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif