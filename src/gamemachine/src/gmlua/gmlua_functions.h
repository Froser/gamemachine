#ifndef __GMLUA_FUNCTIONS_H__
#define __GMLUA_FUNCTIONS_H__
#include <gmcommon.h>
#include "gmlua.h"
BEGIN_NS

#define GM_LUA_CHECK_ARG_COUNT(L, count, invoker) \
	if (gm::luaapi::GMArgumentHelper::getArgumentsCount(L) != count) \
	{ gm_error(L"GMLua : Error occurs while invoking {0}. Reason: Wrong argument count. {1} is expected.", { invoker, gm::GMString(count) }); return 0; }

namespace luaapi
{
	struct GMArgumentHelper
	{
		//! 获取函数当前未被获取的参数数量。
		/*!
		  若要获取某方法传入的所有参数的数量，则此方法必须在调用其他popArgument*方法前被调用。因为每次调用popArgument*，都会减少参数数量的值。<BR>
		  可以将此方法写在while循环中，处理所有参数，直到未被处理的参数个数为0。
		  \param L Lua状态对象。
		  \return 返回当前未被获取的参数数量。
		*/
		static GMint getArgumentsCount(GMLuaCoreState* L);
		static const char* popArgumentAsString(GMLuaCoreState* L, const char* invoker);
		static GMVariant popArgument(GMLuaCoreState* L, const char* invoker);
		static GMVariant popArgumentAsVec2(GMLuaCoreState* L, const char* invoker);
		static GMVariant popArgumentAsVec3(GMLuaCoreState* L, const char* invoker);
		static GMVariant popArgumentAsVec4(GMLuaCoreState* L, const char* invoker);
		static GMVariant popArgumentAsMat4(GMLuaCoreState* L, const char* invoker);
		static bool popArgumentAsObject(GMLuaCoreState* L, REF GMObject& obj, const char* invoker);
	};
}

END_NS
#endif