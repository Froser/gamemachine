#ifndef __GM_LUA_GMSHADER_META_H__
#define __GM_LUA_GMSHADER_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmshader.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMShaderProxy)
	{
		GM_LUA_PROXY_FUNC(__index);
	};

	class GMShaderProxy : public GMAnyProxy<GMShader>
	{
		GM_LUA_PROXY_OBJ(GMShader, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMShaderProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};

	// 以下是GMShader各种属性
	GM_PRIVATE_OBJECT(GMMaterialProxy)
	{
		GM_LUA_PROXY_FUNC(__index);
		GM_LUA_PROXY_FUNC(__newindex);
	};

	class GMMaterialProxy : public GMAnyProxy<GMMaterial>
	{
		GM_LUA_PROXY_OBJ(GMMaterial, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMMaterialProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};

}

END_NS
#endif