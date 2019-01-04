#ifndef __GM_LUA_GAMEOBJECT_META_H__
#define __GM_LUA_GAMEOBJECT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmgameobject.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMGameObjectProxy)
	{
		GM_LUA_PROXY;
		GMGameObject* gameObj = nullptr;
		GMString __name = "GMGameObject";
		GM_LUA_PROXY_FUNC(__gc);
		GM_LUA_PROXY_FUNC(setAsset);
		GM_LUA_PROXY_FUNC(setTranslation);
		GM_LUA_PROXY_FUNC(setRotation);
		GM_LUA_PROXY_FUNC(setScaling);
	};

	class GMGameObjectProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMGameObjectProxy, GMGameObject, gameObj)

	public:
		GMGameObjectProxy(GMGameObject* gameObj = nullptr);

	protected:
		virtual bool registerMeta() override;
	};

	class GMGameObject_Meta : public GMLuaFunctionRegister
	{
	public:
		virtual void registerFunctions(GMLua* L) override;

	private:
		static int regCallback(GMLuaCoreState *L);
		static const char* Name;
	};
}

END_NS
#endif