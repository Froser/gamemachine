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
		GM_LUA_PROXY_FUNC(__gc);
	};

	class GMControlProxy : public GMObjectProxy
	{
	public:
		using GMObjectProxy::GMObjectProxy;

		GM_DECLARE_PRIVATE(GMControlProxy)

	public:
		GMControl* get()
		{
			return gm_cast<GMControl*>(GMObjectProxy::get());
		}

	public:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	class GMControlButtonProxy : public GMControlProxy
	{
	public:
		using GMControlProxy::GMControlProxy;

	public:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif