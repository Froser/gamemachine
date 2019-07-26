#ifndef __GM_LUA_GMSHADER_META_H__
#define __GM_LUA_GMSHADER_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmshader.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(GMShaderProxy);
	class GMShaderProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(GMShader, GMAnyProxy)
		GM_DECLARE_PRIVATE(GMShaderProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		GMShaderProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);
		~GMShaderProxy();

	protected:
		virtual bool registerMeta() override;
	};

	// 以下是GMShader各种属性
	GM_PRIVATE_CLASS(GMMaterialProxy);
	class GMMaterialProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(GMMaterial, GMAnyProxy)
		GM_DECLARE_PRIVATE(GMMaterialProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		GMMaterialProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);

	protected:
		virtual bool registerMeta() override;
	};

}

END_NS
#endif