#ifndef __GM_LUA_IWINDOW_META_H__
#define __GM_LUA_IWINDOW_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMWindowProxy)
	{
		IWindow* window = nullptr;

		GM_META_METHOD_IMPL GMLuaFunctionReturn create(GMLuaCoreState* L);
	};

	class GMWindowProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMWindowProxy)

	public:
		GMWindowProxy(IWindow* window);

	protected:
		virtual bool registerMeta() override;
	};

	class IWindow_Meta : public GMLuaFunctionRegister
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