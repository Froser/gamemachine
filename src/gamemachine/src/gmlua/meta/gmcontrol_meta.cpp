#include "stdafx.h"
#include "gmcontrol_meta.h"
#include "gmwidget_meta.h"

BEGIN_NS

namespace luaapi
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_FUNC(GMControlButton_createControl)
	{
		GMLuaArguments args(L, "GMControlButton.createControl", {
			GMMetaMemberType::Object,
			GMMetaMemberType::String,
			GMMetaMemberType::Int,
			GMMetaMemberType::Int,
			GMMetaMemberType::Int,
			GMMetaMemberType::Int
		});
		GMWidgetProxy widget(L);
		args.getArgument(0, &widget);
		GMString text = args.getArgument(1).toString();
		GMint32 x = args.getArgument(2).toInt();
		GMint32 y = args.getArgument(3).toInt();
		GMint32 width = args.getArgument(4).toInt();
		GMint32 height = args.getArgument(5).toInt();
		if (widget)
		{
			GMControlButton* c = GMControlButton::createControl(widget.get(), text, x, y, width, height, false);
			GMControlButtonProxy control(L, c);
			return gm::GMReturnValues(L, control);
		}
		return gm::GMReturnValues();
	}
	// {{END META FUNCTION}}

	GMLuaReg g_meta_gmcontrolbutton[] = {
		// {{BEGIN META DECLARATIONS}}
		{ "createControl", GMControlButton_createControl },
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};

	GM_LUA_REGISTER_IMPL(GMControlButton_Meta, "GMControlButton", g_meta_gmcontrolbutton);

	//////////////////////////////////////////////////////////////////////////

	bool GMControlProxy::registerMeta()
	{
		D(d);
		return GMObjectProxy::registerMeta();
	}

	bool GMControlButtonProxy::registerMeta()
	{
		D(d);
		return GMControlProxy::registerMeta();
	}
}
END_NS