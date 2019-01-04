#ifndef __GM_LUA_IGRAPHICENGINE_META_H__
#define __GM_LUA_IGRAPHICENGINE_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IGraphicEngineProxy)
	{
		GM_LUA_PROXY;
		IGraphicEngine* engine = nullptr;
		GM_LUA_META_FUNCTION(addLight);
		GM_LUA_META_FUNCTION(getCamera);
		GM_LUA_META_FUNCTION(getDefaultFramebuffers);
	};

	class IGraphicEngineProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(IGraphicEngineProxy, IGraphicEngine, engine)

	public:
		IGraphicEngineProxy(IGraphicEngine* engine = nullptr);

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
		GM_LUA_PROXY;
		GMCamera* camera = nullptr;
		GM_LUA_META_FUNCTION(lookAt);
		GM_LUA_META_FUNCTION(setPerspective);
		GM_LUA_META_FUNCTION(setOrtho);
	};

	class GMCameraProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMCameraProxy, GMCamera, camera)

	public:
		GMCameraProxy(GMCamera* camera = nullptr);

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(IFramebuffersProxy)
	{
		GM_LUA_PROXY;
		IFramebuffers* framebuffers = nullptr;
		GM_LUA_META_FUNCTION(clear);
	};

	class IFramebuffersProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(IFramebuffersProxy, IFramebuffers, framebuffers)

	public:
		IFramebuffersProxy(IFramebuffers* framebuffers = nullptr);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif