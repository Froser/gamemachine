#ifndef __GMLUA_HELPER_H__
#define __GMLUA_HELPER_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_CLASS(GMLuaArguments);
class GM_EXPORT GMLuaArguments
{
	GM_DECLARE_PRIVATE(GMLuaArguments);

public:
	GMLuaArguments(GMLuaCoreState* l, const GMString& invoker = L"", std::initializer_list<GMMetaMemberType> types = {});
	~GMLuaArguments();

	//! 获取在某个位置的参数。
	/*!
	  获取某个位置的参数。如果指定了类型列表，表示获取传入的参数。此时如果参数类型与参数列表不一致，那么会产生lua错误。<BR>
	  如果没有指定类型列表，则表示获取的是返回值。
	  \param index 参数的索引。从0开始算起。
	  \param objRef 对象的引用。如果参数类型是个对象，那么它不得为空，否则会产生一个lua错误。
	  \return 参数的值。如果得到的是一个对象，对象通过objRef返回，而函数返回值表示对象是否完全匹配此参数。
	*/
	GMVariant getArgument(GMint32 index, REF GMObject* objRef = nullptr);
	void pushArgument(const GMVariant& arg);
};

struct GM_EXPORT GMReturnValues
{
public:
	template <typename... VariantType>
	GMReturnValues(GMLuaCoreState* L, VariantType&&... args)
		: m_size(sizeof...(args))
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

END_NS
#endif