#ifndef __GM_LUA_GMBUFFER_META_H__
#define __GM_LUA_GMBUFFER_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	class GMBufferProxy : public GMAnyProxy<GMBuffer>
	{
		GM_LUA_PROXY_OBJ(GMBuffer, GMAnyProxy)
	};
}

END_NS
#endif