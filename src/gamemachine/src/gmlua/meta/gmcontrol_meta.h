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

	class GMControlProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMControlProxy, GMControl, GMObjectProxy)

	public:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	class GMControlButtonProxy : public GMControlProxy
	{
		GM_LUA_PROXY_OBJ(GMControlButtonProxy, GMControlButton, GMControlProxy)

	public:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif