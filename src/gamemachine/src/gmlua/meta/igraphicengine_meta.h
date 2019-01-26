#ifndef __GM_LUA_IGRAPHICENGINE_META_H__
#define __GM_LUA_IGRAPHICENGINE_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IGraphicEngineProxy)
	{
		GM_LUA_PROXY_FUNC(addLight);
		GM_LUA_PROXY_FUNC(getCamera);
		GM_LUA_PROXY_FUNC(getDefaultFramebuffers);
		GM_LUA_PROXY_FUNC(getGlyphManager);
	};

	class IGraphicEngineProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IGraphicEngine, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(IGraphicEngineProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(GMCameraLookAtProxy)
	{
		GMVec3 lookAt;
		GMVec3 position;
		GMVec3 up;
	};

	class GMCameraLookAtProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMCameraLookAtProxy)

	public:
		virtual bool registerMeta() override;

	public:
		GMCameraLookAt toCameraLookAt();
	};

	GM_PRIVATE_OBJECT(GMCameraProxy)
	{
		GM_LUA_PROXY_FUNC(lookAt);
		GM_LUA_PROXY_FUNC(setPerspective);
		GM_LUA_PROXY_FUNC(setOrtho);
	};

	class GMCameraProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(GMCamera, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMCameraProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(IFramebuffersProxy)
	{
		GM_LUA_PROXY_FUNC(clear);
	};

	class IFramebuffersProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IFramebuffers, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(IFramebuffersProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif