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

	bool isValid();
	GMVariant getArgument(GMint32 index);
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