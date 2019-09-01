#include "stdafx.h"
#include "gmcontrol_meta.h"
#include "gmwidget_meta.h"

BEGIN_NS

namespace luaapi
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_FUNC(GMControlButton_createControl)
	{
		static const GMString s_invoker = L"GMControlButton.createControl";
		GM_LUA_CHECK_ARG_COUNT(L, 6, "GMControlButton.createControl");
		GMWidgetProxy widget(L);
		GMint32 height = GMArgumentHelper::popArgument(L, s_invoker).toInt(); // height
		GMint32 width = GMArgumentHelper::popArgument(L, s_invoker).toInt(); // width
		GMint32 y = GMArgumentHelper::popArgument(L, s_invoker).toInt(); // y
		GMint32 x = GMArgumentHelper::popArgument(L, s_invoker).toInt(); // x
		GMString text = GMArgumentHelper::popArgumentAsString(L, s_invoker); //text
		GMArgumentHelper::popArgumentAsObject(L, widget, s_invoker); //widget
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