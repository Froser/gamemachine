#ifndef __GM_LUA_IGRAPHICENGINE_META_H__
#define __GM_LUA_IGRAPHICENGINE_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(IGraphicEngineProxy);
	class IGraphicEngineProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IGraphicEngine, GMAnyProxy)
		GM_DECLARE_PRIVATE(IGraphicEngineProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		IGraphicEngineProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);
		~IGraphicEngineProxy();

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_CLASS(GMCameraLookAtProxy);
	class GMCameraLookAtProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMCameraLookAtProxy)

	public:
		virtual bool registerMeta() override;

	public:
		GMCameraLookAt toCameraLookAt();
	};

	GM_PRIVATE_CLASS(GMCameraProxy);
	class GMCameraProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(GMCamera, GMAnyProxy)
		GM_DECLARE_PRIVATE(GMCameraProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		GMCameraProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_CLASS(IFramebuffersProxy);
	class IFramebuffersProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IFramebuffers, GMAnyProxy)
		GM_DECLARE_PRIVATE(IFramebuffersProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		IFramebuffersProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif