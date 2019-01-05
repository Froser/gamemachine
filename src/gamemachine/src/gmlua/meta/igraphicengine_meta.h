#ifndef __GM_LUA_IGRAPHICENGINE_META_H__
#define __GM_LUA_IGRAPHICENGINE_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IGraphicEngineProxy)
	{
		GM_LUA_PROXY(IGraphicEngine);
		GM_LUA_PROXY_FUNC(addLight);
		GM_LUA_PROXY_FUNC(getCamera);
		GM_LUA_PROXY_FUNC(getDefaultFramebuffers);
	};

	class IGraphicEngineProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(IGraphicEngineProxy, IGraphicEngine)

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
		GM_LUA_PROXY(GMCamera);
		GM_LUA_PROXY_FUNC(lookAt);
		GM_LUA_PROXY_FUNC(setPerspective);
		GM_LUA_PROXY_FUNC(setOrtho);
	};

	class GMCameraProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMCameraProxy, GMCamera)

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(IFramebuffersProxy)
	{
		GM_LUA_PROXY(IFramebuffers);
		GM_LUA_PROXY_FUNC(clear);
	};

	class IFramebuffersProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(IFramebuffersProxy, IFramebuffers)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif