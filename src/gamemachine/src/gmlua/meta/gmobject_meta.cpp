#include "stdafx.h"
#include "gmobject_meta.h"
#include <gmlua.h>
#include <gmobject.h>

using namespace luaapi;

#define NAME "GMObject"
#define GMOBJECT_PROXY_META GM_META(__detached); GM_META_WITH_TYPE(__handler, GMMetaMemberType::Pointer); GM_META(__name);

//////////////////////////////////////////////////////////////////////////
bool GMObjectProxy::registerMeta()
{
	D(d);
	GMOBJECT_PROXY_META;
	GM_META_FUNCTION(connect);
	return true;
}

/*
 * connect([self], sender, signal, callback)
 */
GM_LUA_PROXY_IMPL(GMObjectProxy, connect)
{
	static const GMString s_invoker = NAME ".connect";
	GM_LUA_CHECK_ARG_COUNT(L, 4, NAME ".connect");
	GMObjectProxy self, sender;
	GMLuaReference callback = GMArgumentHelper::popArgument(L, s_invoker).toInt(); // callback
	GMString signal = GMArgumentHelper::popArgumentAsString(L, s_invoker); //signal
	GMArgumentHelper::popArgumentAsObject(L, sender, s_invoker); //sender
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		self->connect(*sender.get(), signal, [L, callback](GMObject* s, GMObject* r) {
			GMObjectProxy sender(s), receiver(r);
			GMLua l(L);
			lua_rawgeti(L, LUA_REGISTRYINDEX, callback);
			l.protectedCall(nullptr, { sender, receiver } );
		});
	}
	return GMReturnValues();
}
//////////////////////////////////////////////////////////////////////////
