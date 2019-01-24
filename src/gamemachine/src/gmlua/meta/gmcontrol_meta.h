#ifndef __GM_LUA_GMCONTROL_META_H__
#define __GM_LUA_GMCONTROL_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmcontrols.h>
#include <gmcontrolbutton.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_LUA_REGISTER(GMControlButton_Meta);

	GM_PRIVATE_OBJECT(GMControlProxy)
	{
		GM_LUA_PROXY(GMControl);
		GM_LUA_PROXY_EXTENDS(GMObjectProxy);
		GM_LUA_PROXY_FUNC(__gc);
	};

	class GMControlProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMControlProxy, GMControl)

	public:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(GMControlButtonProxy)
	{
		GM_LUA_PROXY(GMControlButton);
		GM_LUA_PROXY_EXTENDS(GMControlProxy);
	};

	class GMControlButtonProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMControlButtonProxy, GMControlButton)

	public:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif