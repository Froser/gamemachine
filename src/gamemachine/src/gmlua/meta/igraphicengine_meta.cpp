#include "stdafx.h"
#include "igraphicengine_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "ilight_meta.h"
#include "gmglyphmanager_meta.h"

#define NAME "IGraphicEngine"

using namespace gm::luaapi;

/*
 * addLight([self], IGameHandler)
 */
GM_LUA_PROXY_IMPL(IGraphicEngineProxy, addLight)
{
	static const GMString s_invoker = NAME ".addLight";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".addLight");
	IGraphicEngineProxy self(L);
	ILightProxy light(L);
	GMArgumentHelper::popArgumentAsObject(L, light, s_invoker); //IGameHandler
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		light.setAutoRelease(false);
		self->addLight(light.get());
	}
	return GMReturnValues();
}

/*
 * getCamera([self])
 */
GM_LUA_PROXY_IMPL(IGraphicEngineProxy, getCamera)
{
	static const GMString s_invoker = NAME ".getCamera";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getCamera");
	IGraphicEngineProxy self(L);
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		GMCamera& camera = self->getCamera();
		GMCameraProxy proxy(L);
		proxy.set(&camera);
		return GMReturnValues(L, proxy);
	}
	return GMReturnValues();
}

/*
 * getCamera([self])
 */
GM_LUA_PROXY_IMPL(IGraphicEngineProxy, getDefaultFramebuffers)
{
	static const GMString s_invoker = NAME ".getDefaultFramebuffers";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getDefaultFramebuffers");
	IGraphicEngineProxy self(L);
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		IFramebuffers* defaultFramebuffers = self->getDefaultFramebuffers();
		IFramebuffersProxy proxy(L);
		proxy.set(defaultFramebuffers);
		return GMReturnValues(L, proxy);
	}
	return GMReturnValues();
}

/*
 * getGlyphManager([self])
 */
GM_LUA_PROXY_IMPL(IGraphicEngineProxy, getGlyphManager)
{
	static const GMString s_invoker = NAME ".getGlyphManager";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getGlyphManager");
	IGraphicEngineProxy self(L);
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		GMGlyphManager* glyphManager = self->getGlyphManager();
		GMGlyphManagerProxy proxy(L);
		proxy.set(glyphManager);
		return GMReturnValues(L, proxy);
	}
	return GMReturnValues();
}

bool IGraphicEngineProxy::registerMeta()
{
	GM_META_FUNCTION(addLight);
	GM_META_FUNCTION(getCamera);
	GM_META_FUNCTION(getDefaultFramebuffers);
	GM_META_FUNCTION(getGlyphManager);
	return Base::registerMeta();
}

//////////////////////////////////////////////////////////////////////////

#define CAMERA_NAME "GMCamera"

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

/*
 * lookAt([self], GMCameraLookAt)
 */
GM_LUA_PROXY_IMPL(GMCameraProxy, lookAt)
{
	static const GMString s_invoker = CAMERA_NAME ".lookAt";
	GM_LUA_CHECK_ARG_COUNT(L, 2, CAMERA_NAME ".lookAt");
	GMCameraProxy self(L);
	GMCameraLookAtProxy lookAt;
	GMArgumentHelper::popArgumentAsObject(L, lookAt, s_invoker); //IGameHandler
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->lookAt(lookAt.toCameraLookAt());
	return GMReturnValues();
}

/*
 * setPerspective([self], fovy, aspect, near, far)
 */
GM_LUA_PROXY_IMPL(GMCameraProxy, setPerspective)
{
	static const GMString s_invoker = CAMERA_NAME ".setPerspective";
	GM_LUA_CHECK_ARG_COUNT(L, 5, CAMERA_NAME ".setPerspective");
	GMCameraProxy self(L);
	GMfloat f = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //far
	GMfloat n = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //near
	GMfloat aspect = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //aspect
	GMfloat fovy = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //fovy
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setPerspective(fovy, aspect, n, f);
	return GMReturnValues();
}

/*
 * setOrtho([self], left, right, bottom, top, near, far)
 */
GM_LUA_PROXY_IMPL(GMCameraProxy, setOrtho)
{
	static const GMString s_invoker = CAMERA_NAME ".setPerspective";
	GM_LUA_CHECK_ARG_COUNT(L, 7, CAMERA_NAME ".setPerspective");
	GMCameraProxy self(L);
	GMfloat f = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //far
	GMfloat n = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //near
	GMfloat top = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //top
	GMfloat bottom = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //bottom
	GMfloat right = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //right
	GMfloat left = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //left
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setOrtho(left, right, bottom, top, n, f);
	return GMReturnValues();
}

bool GMCameraProxy::registerMeta()
{
	GM_META_FUNCTION(lookAt);
	GM_META_FUNCTION(setPerspective);
	GM_META_FUNCTION(setOrtho);
	return Base::registerMeta();
}

//////////////////////////////////////////////////////////////////////////
#define IFRAMEBUFFERS_NAME "IFramebuffers"

/*
* clear([self], clearType)
*/
GM_LUA_PROXY_IMPL(IFramebuffersProxy, clear)
{
	static const GMString s_invoker = IFRAMEBUFFERS_NAME ".clear";
	GM_LUA_CHECK_ARG_COUNT_GT(L, 1, IFRAMEBUFFERS_NAME ".clear");
	IFramebuffersProxy self(L);
	GMFramebuffersClearType type = (GMArgumentHelper::getArgumentsCount(L) == 2) ? 
		(static_cast<GMFramebuffersClearType>(GMArgumentHelper::popArgument(L, s_invoker).toInt())) : 
		(GMFramebuffersClearType::All); // clearType
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->clear(type);
	return GMReturnValues();
}

bool IFramebuffersProxy::registerMeta()
{
	GM_META_FUNCTION(clear);
	return Base::registerMeta();
}
