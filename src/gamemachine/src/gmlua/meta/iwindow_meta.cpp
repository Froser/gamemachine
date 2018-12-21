#include "stdafx.h"
#include "iwindow_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

#define NAME "GMWindow"

using namespace luaapi;

namespace
{
	struct GMWindowDescEx : public GMWindowDesc
	{
		GMRectF rc;
	};

	class GMWindowDescProxy : public GMObject
	{
		GM_DECLARE_PRIVATE_FROM_STRUCT(GMWindowDescProxy, GMWindowDescEx)

	public:
		const GMWindowDesc& get()
		{
			D(d);
			GM_ASSERT(false);
			return *d;
		}

	protected:
		virtual bool registerMeta() override
		{
			GM_META(createNewWindow);
			GM_META(windowName);
			GM_META_WITH_TYPE(dwStyle, GMMetaMemberType::Int);
			GM_META_WITH_TYPE(dwExStyle, GMMetaMemberType::Int);
			GM_META_WITH_TYPE(rc, GMMetaMemberType::Vector4);
			GM_META(samples);
			return true;
		}
	};

	// {{BEGIN META FUNCTION}}
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

const char* IWindow_Meta::Name = NAME;

void IWindow_Meta::registerFunctions(GMLua* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int IWindow_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}

//////////////////////////////////////////////////////////////////////////
GMWindowProxy::GMWindowProxy(IWindow* window)
{
	D(d);
	d->window = window;
}

bool GMWindowProxy::registerMeta()
{
	D(d);
	GM_META(window);
	GM_META_FUNCTION(create);
	return true;
}

void GMWindowProxy::create(const GMWindowDesc& desc)
{
	D(d);
	if (d->window)
		d->window->create(desc);
}

GMLuaFunctionReturn GM_PRIVATE_NAME(GMWindowProxy)::create(GMLuaCoreState* L)
{
	/************************************************************************/
	/* create([self], GMWindowDesc)                                         */
	/************************************************************************/
	static const GMString s_invoker(L".create");
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".create");
	GMWindowProxy window(nullptr);
	GMWindowDescProxy desc;
	GMArgumentHelper::popArgumentAsObject(L, desc, s_invoker); //GMWindowDesc
	GMArgumentHelper::popArgumentAsObject(L, window, s_invoker); //self
	window.create(desc.get());
	return GMReturnValues();
}
//////////////////////////////////////////////////////////////////////////
