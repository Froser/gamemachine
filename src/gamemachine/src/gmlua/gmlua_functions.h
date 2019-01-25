#ifndef __GMLUA_FUNCTIONS_H__
#define __GMLUA_FUNCTIONS_H__
#include <gmcommon.h>
#include "gmlua.h"
BEGIN_NS

#define GM_LUA_CHECK_ARG_COUNT(L, count, invoker) \
	{ auto argc = gm::luaapi::GMArgumentHelper::getArgumentsCount(L); if ( argc != count) \
	{ gm_error(gm_dbg_wrap(" Error occurs while invoking {0}. Reason: Wrong argument count. {1} is expected but argument count is {2}."), invoker, gm::GMString(count), gm::GMString(argc) ); return 0; } }

#define GM_LUA_CHECK_ARG_COUNT_GT(L, count, invoker) \
	{ auto argc = gm::luaapi::GMArgumentHelper::getArgumentsCount(L); if ( argc != count) \
	{ gm_error(gm_dbg_wrap(" Error occurs while invoking {0}. Reason: Wrong argument count. More than {1} is expected but argument count is {2}."), invoker, gm::GMString(count), gm::GMString(argc) ); return 0; } }

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
		static GMint32 getArgumentsCount(GMLuaCoreState* L);
		static const char* popArgumentAsString(GMLuaCoreState* L, const GMString& invoker);
		static GMVariant peekArgument(GMLuaCoreState* L, GMint32 index, const GMString& invoker);
		static GMVariant popArgument(GMLuaCoreState* L, const GMString& invoker);
		static GMVariant popArgumentAsVec2(GMLuaCoreState* L, const GMString& invoker);
		static GMVariant popArgumentAsVec3(GMLuaCoreState* L, const GMString& invoker);
		static GMVariant popArgumentAsVec4(GMLuaCoreState* L, const GMString& invoker);
		static GMVariant popArgumentAsMat4(GMLuaCoreState* L, const GMString& invoker);
		static bool popArgumentAsObject(GMLuaCoreState* L, REF GMObject& obj, const GMString& invoker);
	};

	struct GMReturnValues
	{
	public:
		template <typename... VariantType>
		GMReturnValues(GMLuaCoreState* L, VariantType&&... args)
			: m_size (sizeof...(args))
			, m_L(L)
		{
			pushArgument((args)...);
		}

		GMReturnValues()
			: m_size(0)
		{}

		GMReturnValues(const GMReturnValues& rhs)
			: m_size(rhs.m_size)
			, m_L(const_cast<GMReturnValues&>(rhs).m_L.getLuaCoreState())
		{}

		operator GMint32()
		{
			return gm_sizet_to_int(m_size);
		}

	private:
		template <typename... VariantType>
		void pushArgument(const GMVariant& arg, VariantType&&... args)
		{
			pushArgument(arg);
			pushArgument(std::forward<VariantType>(args)...);
		}

		void pushArgument(const GMVariant& arg);

	private:
		GMsize_t m_size = 0;
		GMLua m_L;
	};


	// Lua模板类，用于模拟一些容器
	#define __META(memberName) \
	{ \
		GM_STATIC_ASSERT(static_cast<gm::GMMetaMemberType>( gm::GMMetaMemberTypeGetter<decltype(memberName)>::Type ) != gm::GMMetaMemberType::Invalid, "Invalid Meta type"); \
		gm::GMObject::data()->meta[#memberName] = { static_cast<gm::GMMetaMemberType>( gm::GMMetaMemberTypeGetter<decltype(memberName)>::Type ), sizeof(memberName), &memberName }; \
	}

	#define __META_FUNCTION(memberName) \
		gm::GMObject::data()->meta[#memberName] = { GMMetaMemberType::Function, 0, (void*)&memberName };

	template <typename T>
	class GMLuaVector : public GMObject
	{
		typedef Vector<T> ContainerType;
	public:
		GMLuaVector() = default;
		GMLuaVector(ContainerType* container)
			: value(container)
		{
		}

		virtual bool registerMeta() override
		{
			__META(__name);
			__META(value);
			__META_FUNCTION(__index);
			__META_FUNCTION(__gc);
			__META_FUNCTION(size);
			return true;
		}

		static GMFunctionReturn __gc(GMLuaCoreState* l)
		{
			static const GMString s_invoker = "GMLuaVector.__gc";
			GMLuaVector self;
			GMArgumentHelper::popArgumentAsObject(l, self, s_invoker); //self
			self.release();
			return GMReturnValues();
		}

		static GMFunctionReturn __index(GMLuaCoreState* l)
		{
			static const GMString s_invoker = "GMLuaVector.__index";
			GMLuaVector self;
			GMVariant i = GMArgumentHelper::popArgument(l, s_invoker); //i
			GMArgumentHelper::popArgumentAsObject(l, self, s_invoker); //self
			return GMReturnValues(l, (*(self.value))[i.toInt() - 1]);
		}

		static GMFunctionReturn size(GMLuaCoreState* l)
		{
			static const GMString s_invoker = "GMLuaVector.size";
			GMLuaVector self;
			GMArgumentHelper::popArgumentAsObject(l, self, s_invoker); //self
			return GMReturnValues(l, gm_sizet_to_int(self.value->size()));
		}

	private:
		void release()
		{
			if (value)
				GM_delete(value);
		}

	private:
		GMString __name = L"GMLuaVector";
		ContainerType* value = nullptr;
	};

	#undef __META
	#undef __META_FUNCTION

}

END_NS
#endif