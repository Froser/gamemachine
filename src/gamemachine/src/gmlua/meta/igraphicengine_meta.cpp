#include "stdafx.h"
#include "igraphicengine_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "ilight_meta.h"
#include "gmglyphmanager_meta.h"

#define NAME "IGraphicEngine"

BEGIN_NS
namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(IGraphicEngineProxy)
	{
		GM_LUA_PROXY_FUNC(addLight);
		GM_LUA_PROXY_FUNC(getCamera);
		GM_LUA_PROXY_FUNC(getDefaultFramebuffers);
		GM_LUA_PROXY_FUNC(getGlyphManager);
	};

	GM_PRIVATE_OBJECT_ALIGNED(GMCameraLookAtProxy)
	{
		GMVec3 lookAt;
		GMVec3 position;
		GMVec3 up;
	};

	GM_PRIVATE_OBJECT_UNALIGNED(GMCameraProxy)
	{
		GM_LUA_PROXY_FUNC(lookAt);
		GM_LUA_PROXY_FUNC(setPerspective);
		GM_LUA_PROXY_FUNC(setOrtho);
	};

	GM_PRIVATE_OBJECT_UNALIGNED(IFramebuffersProxy)
	{
		GM_LUA_PROXY_FUNC(clear);
	};


	/*
	 * addLight([self], IGameHandler)
	 */
	GM_LUA_PROXY_IMPL(IGraphicEngineProxy, addLight)
	{
		GMLuaArguments args(L, NAME ".addLight", { GMMetaMemberType::Object, GMMetaMemberType::Object });
		IGraphicEngineProxy self(L);
		ILightProxy light(L);
		args.getArgument(0, &self);
		args.getArgument(1, &light);
		if (self)
		{
			light.setAutoRelease(false);
			self->addLight(light.get());
		}
		return gm::GMReturnValues();
	}

	/*
	 * getCamera([self])
	 */
	GM_LUA_PROXY_IMPL(IGraphicEngineProxy, getCamera)
	{
		GMLuaArguments args(L, NAME ".getCamera", { GMMetaMemberType::Object });
		IGraphicEngineProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			GMCamera& camera = self->getCamera();
			GMCameraProxy proxy(L);
			proxy.set(&camera);
			return gm::GMReturnValues(L, proxy);
		}
		return gm::GMReturnValues();
	}

	/*
	 * getCamera([self])
	 */
	GM_LUA_PROXY_IMPL(IGraphicEngineProxy, getDefaultFramebuffers)
	{
		GMLuaArguments args(L, NAME ".getDefaultFramebuffers", { GMMetaMemberType::Object });
		IGraphicEngineProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			IFramebuffers* defaultFramebuffers = self->getDefaultFramebuffers();
			IFramebuffersProxy proxy(L);
			proxy.set(defaultFramebuffers);
			return gm::GMReturnValues(L, proxy);
		}
		return gm::GMReturnValues();
	}

	/*
	 * getGlyphManager([self])
	 */
	GM_LUA_PROXY_IMPL(IGraphicEngineProxy, getGlyphManager)
	{
		GMLuaArguments args(L, NAME ".getGlyphManager", { GMMetaMemberType::Object });
		IGraphicEngineProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			GMGlyphManager* glyphManager = self->getGlyphManager();
			GMGlyphManagerProxy proxy(L);
			proxy.set(glyphManager);
			return gm::GMReturnValues(L, proxy);
		}
		return gm::GMReturnValues();
	}

	bool IGraphicEngineProxy::registerMeta()
	{
		GM_META_FUNCTION(addLight);
		GM_META_FUNCTION(getCamera);
		GM_META_FUNCTION(getDefaultFramebuffers);
		GM_META_FUNCTION(getGlyphManager);
		return Base::registerMeta();
	}

	IGraphicEngineProxy::~IGraphicEngineProxy()
	{

	}

	IGraphicEngineProxy::IGraphicEngineProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	//////////////////////////////////////////////////////////////////////////
#undef NAME
#define NAME "GMCamera"

	bool GMCameraLookAtProxy::registerMeta()
	{
		GM_META(lookAt);
		GM_META(position);
		GM_META(up);
		return true;
	}

	GMCameraLookAt GMCameraLookAtProxy::toCameraLookAt()
	{
		D(d);
		GMCameraLookAt l(d->lookAt, d->position, d->up);
		return l;
	}

	GMCameraLookAtProxy::GMCameraLookAtProxy()
	{
		GM_CREATE_DATA();
	}

	GMCameraLookAtProxy::~GMCameraLookAtProxy()
	{

	}

	/*
	 * lookAt([self], GMCameraLookAt)
	 */
	GM_LUA_PROXY_IMPL(GMCameraProxy, lookAt)
	{
		GMLuaArguments args(L, NAME ".lookAt", { GMMetaMemberType::Object, GMMetaMemberType::Object });
		GMCameraProxy self(L);
		GMCameraLookAtProxy lookAt;
		args.getArgument(0, &self);
		args.getArgument(1, &lookAt);
		if (self)
			self->lookAt(lookAt.toCameraLookAt());
		return gm::GMReturnValues();
	}

	/*
	 * setPerspective([self], fovy, aspect, near, far)
	 */
	GM_LUA_PROXY_IMPL(GMCameraProxy, setPerspective)
	{
		static const GMString s_invoker = NAME ".setPerspective";
		GMLuaArguments args(L, NAME ".setPerspective", { GMMetaMemberType::Object, GMMetaMemberType::Float, GMMetaMemberType::Float, GMMetaMemberType::Float, GMMetaMemberType::Float });
		GMCameraProxy self(L);
		args.getArgument(0, &self);
		GMfloat fovy = args.getArgument(1).toFloat();
		GMfloat aspect = args.getArgument(2).toFloat();
		GMfloat n = args.getArgument(3).toFloat();
		GMfloat f = args.getArgument(4).toFloat();
		if (self)
			self->setPerspective(fovy, aspect, n, f);
		return gm::GMReturnValues();
	}

	/*
	 * setOrtho([self], left, right, bottom, top, near, far)
	 */
	GM_LUA_PROXY_IMPL(GMCameraProxy, setOrtho)
	{
		GMLuaArguments args(L, NAME ".setOrtho", { GMMetaMemberType::Object, GMMetaMemberType::Float, GMMetaMemberType::Float, GMMetaMemberType::Float, GMMetaMemberType::Float, GMMetaMemberType::Float, GMMetaMemberType::Float });
		GMCameraProxy self(L);
		args.getArgument(0, &self);
		GMfloat left = args.getArgument(1).toFloat();
		GMfloat right = args.getArgument(2).toFloat();
		GMfloat bottom = args.getArgument(3).toFloat();
		GMfloat top = args.getArgument(4).toFloat();
		GMfloat n = args.getArgument(5).toFloat();
		GMfloat f = args.getArgument(6).toFloat();
		if (self)
			self->setOrtho(left, right, bottom, top, n, f);
		return gm::GMReturnValues();
	}

	bool GMCameraProxy::registerMeta()
	{
		GM_META_FUNCTION(lookAt);
		GM_META_FUNCTION(setPerspective);
		GM_META_FUNCTION(setOrtho);
		return Base::registerMeta();
	}

	GMCameraProxy::GMCameraProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

#undef NAME

	//////////////////////////////////////////////////////////////////////////
#define NAME "IFramebuffers"

/*
* clear([self], clearType)
*/
	GM_LUA_PROXY_IMPL(IFramebuffersProxy, clear)
	{
		static const GMString s_invoker = NAME ".clear";
		GMLuaArguments args(L, NAME ".clear", { GMMetaMemberType::Object, GMMetaMemberType::Int });
		IFramebuffersProxy self(L);
		args.getArgument(0, &self);
		GMFramebuffersClearType type = static_cast<GMFramebuffersClearType>(args.getArgument(1).toInt());
		if (self)
			self->clear(type);
		return gm::GMReturnValues();
	}

	bool IFramebuffersProxy::registerMeta()
	{
		GM_META_FUNCTION(clear);
		return Base::registerMeta();
	}

	IFramebuffersProxy::IFramebuffersProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

}
#undef NAME

END_NS