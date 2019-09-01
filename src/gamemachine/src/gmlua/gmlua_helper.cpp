#include "stdafx.h"
#include "gmlua.h"

BEGIN_NS

void GMReturnValues::pushArgument(const GMVariant& arg)
{
	if (arg.isObject())
	{
		m_L.pushNewTable(*arg.toObject());
	}
	else
	{
		GMLuaArguments args(m_L.getLuaCoreState());
		args.pushArgument(arg);
	}
}

//////////////////////////////////////////////////////////////////////////
GMint32 lua_Sizeof(GMMetaMemberType type)
{
	switch (type)
	{
	case GMMetaMemberType::Vector2:
		return 2;
	case GMMetaMemberType::Vector3:
		return 3;
	case GMMetaMemberType::Vector4:
		return 4;
	case GMMetaMemberType::Matrix4x4:
		return 16;
	default:
		return 1;
	}
}

GM_PRIVATE_OBJECT_UNALIGNED(GMLuaArguments)
{
	GMLuaCoreState* L;
	std::initializer_list<GMMetaMemberType> types;
	GMint32 totalSize = 0;
	Vector<GMint32> indices;

	void calculateSize();
	void pushVector(const GMVec2& v);
	void pushVector(const GMVec3& v);
	void pushVector(const GMVec4& v);
	void pushMatrix(const GMMat4& v);
	void push(const GMVariant& var);
};

template <typename T>
void __pushVector(const T& v, GMLuaCoreState* l)
{
	GM_CHECK_LUA_STACK_BALANCE_(l, 1);
	lua_newtable(l);
	GMFloat4 f4;
	v.loadFloat4(f4);
	for (GMint32 i = 0; i < T::length(); i++)
	{
		lua_pushnumber(l, i);
		lua_pushnumber(l, f4[i]);
		GM_ASSERT(lua_istable(l, -3));
		lua_settable(l, -3);
	}
}

void GMLuaArgumentsPrivate::calculateSize()
{
	totalSize = 0;
	indices.resize(types.size());
	GMint32 idx = 0;
	for (auto& type : types)
	{
		totalSize += lua_Sizeof(type);
		indices[idx++] = totalSize;
	}
}

void GMLuaArgumentsPrivate::pushVector(const GMVec2& v)
{
	__pushVector(v, L);
}

void GMLuaArgumentsPrivate::pushVector(const GMVec3& v)
{
	__pushVector(v, L);
}

void GMLuaArgumentsPrivate::pushVector(const GMVec4& v)
{
	__pushVector(v, L);
}

void GMLuaArgumentsPrivate::push(const GMVariant& var)
{
	GM_CHECK_LUA_STACK_BALANCE(1);
	if (var.isInt() || var.isInt64())
	{
		lua_pushinteger(L, var.toInt64());
	}
	else if (var.isUInt())
	{
		lua_pushinteger(L, var.toUInt());
	}
	else if (var.isFloat())
	{
		lua_pushnumber(L, var.toFloat());
	}
	else if (var.isBool())
	{
		lua_pushboolean(L, var.toBool());
	}
	else if (var.isString())
	{
		std::string str = var.toString().toStdString();
		lua_pushstring(L, str.c_str());
	}
	else if (var.isObject())
	{
		GM_ASSERT(false);
		//pushNewTable(*var.toObject());
	}
	else if (var.isPointer())
	{
		GM_STATIC_ASSERT(sizeof(lua_Integer) >= sizeof(void*), "Pointer size incompatible.");
		lua_pushinteger(L, (lua_Integer)var.toPointer());
	}
	else if (var.isVec2())
	{
		pushVector(var.toVec2());
	}
	else if (var.isVec3())
	{
		pushVector(var.toVec3());
	}
	else if (var.isVec4())
	{
		pushVector(var.toVec4());
	}
	else if (var.isMat4())
	{
		pushMatrix(var.toMat4());
	}
	else
	{
		gm_error(gm_dbg_wrap("GMLua (push): variant type not supported"));
		GM_ASSERT(false);
	}
}

void GMLuaArgumentsPrivate::pushMatrix(const GMMat4& v)
{
	GM_CHECK_LUA_STACK_BALANCE(1);
	lua_newtable(L);
	for (GMint32 i = 0; i < GMMat4::length(); i++)
	{
		lua_pushnumber(L, i);
		pushVector(v[i]);
		GM_ASSERT(lua_istable(L, -3));
		lua_settable(L, -3);
	}
}

GMLuaArguments::GMLuaArguments(GMLuaCoreState* l, std::initializer_list<GMMetaMemberType> types)
{
	GM_CREATE_DATA();
	D(d);
	d->types = types;
	d->L = l;
	d->calculateSize();
}

GMLuaArguments::~GMLuaArguments()
{

}

bool GMLuaArguments::isValid()
{
	D(d);
	return lua_gettop(d->L) == d->totalSize;
}

GMVariant GMLuaArguments::getArgument(GMint32 index, const GMString& invoker)
{
	// 先从index中找到索引
	return GMVariant();
}

void GMLuaArguments::pushArgument(const GMVariant& arg)
{
	D(d);
	d->push(arg);
}

END_NS