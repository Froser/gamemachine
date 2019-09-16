﻿#include "stdafx.h"
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
GM_PRIVATE_OBJECT_UNALIGNED(GMLuaArguments)
{
	GMLuaCoreState* L;
	Vector<GMMetaMemberType> types;
	GMint32 totalSize = 0;
	Vector<GMint32> indices;
	GMString invoker;

	void calculateSize();
	void pushVector(const GMVec2& v);
	void pushVector(const GMVec3& v);
	void pushVector(const GMVec4& v);
	void pushMatrix(const GMMat4& v);
	void push(const GMVariant& var);
	GMVariant getScalar(GMint32 index);
	GMint32 getIndexInStack(GMint32 offset, GMint32 index, OUT GMMetaMemberType* type = nullptr);
	void checkType(const GMVariant& v, GMMetaMemberType mt, GMint32 index, const GMString& invoker);

	//! 是否某个栈上的变量是一个向量，如果不是则范围0，如果是则返回向量的维度(2-4)。
	GMint32 getVec(GMint32 index, GMfloat values[4]);
	bool getMat4(GMint32 index, REF GMFloat4 (&values)[4]);
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
		++totalSize;
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

GMVariant GMLuaArgumentsPrivate::getScalar(GMint32 index)
{
	if (lua_isinteger(L, index))
		return static_cast<GMint32>(lua_tointeger(L, index));
	if (lua_isnumber(L, index))
		return lua_tonumber(L, index);
	if (lua_isstring(L, index))
		return GMString(lua_tostring(L, index));
	if (lua_isboolean(L, index))
		return lua_toboolean(L, index) ? true : false;

	GMfloat values[4];
	if (GMint32 v = getVec(index, values))
	{
		if (v == 2) return GMVec2(values[0], values[1]);
		if (v == 3) return GMVec3(values[0], values[1], values[2]);
		if (v == 4) return GMVec4(values[0], values[1], values[2], values[3]);
	}

	GMFloat16 f16;
	if (getMat4(index, f16.v_))
	{
		GMMat4 m;
		m.setFloat16(f16);
		return m;
	}
	gm_error(gm_dbg_wrap("GMLua (pop): type not supported"));
	return GMVariant();
}

GMint32 GMLuaArgumentsPrivate::getIndexInStack(GMint32 offset, GMint32 index, OUT GMMetaMemberType* t)
{
	if (t)
		*t = types[index];
	return offset + index + 1;
}

void GMLuaArgumentsPrivate::checkType(const GMVariant& v, GMMetaMemberType mt, GMint32 index, const GMString& invoker)
{
	bool suc = false;
	switch (mt)
	{
	case GMMetaMemberType::Int:
	case GMMetaMemberType::Float:
		suc = v.isFloat() || v.isInt() || v.isInt64();
		break;
	case GMMetaMemberType::Vector2:
		suc = v.isVec2();
		break;
	case GMMetaMemberType::Vector3:
		suc = v.isVec3();
		break;
	case GMMetaMemberType::Vector4:
		suc = v.isVec4();
		break;
	case GMMetaMemberType::Matrix4x4:
		suc = v.isMat4();
		break;
	case GMMetaMemberType::String:
		suc = v.isString();
		break;
	case GMMetaMemberType::Boolean:
		suc = v.isBool();
		break;
	case GMMetaMemberType::Object:
	case GMMetaMemberType::Pointer:
	case GMMetaMemberType::Function:
	case GMMetaMemberType::Invalid:
	default:
		break;
	}

	std::string ivk = invoker.toStdString();
	if (!suc)
	{
		luaL_error(L, "Type is not match at index %i in %s", index, ivk.c_str());
	}
}

GMint32 GMLuaArgumentsPrivate::getVec(GMint32 index, GMfloat values[4])
{
	if (lua_istable(L, index))
	{
		GM_ASSERT(values);
		// 检查里面的index 0,1,2,3是否为数字
		GMint32 i;
		for (i = 1; i < 5; ++i)
		{
			GMint32 type = lua_geti(L, index, i);
			GMint32 top = lua_gettop(L);
			int ian = lua_isnumber(L, top);
			if (!ian) // 如果不是数字，则直接返回
			{
				lua_pop(L, 1);
				break;
			}
			values[i - 1] = lua_tonumber(L, top);
			lua_pop(L, 1);
		}

		// vec2-vec4
		GMint32 dimension = i - 1;
		if (dimension < 2 || dimension > 4)
			return 0;

		return dimension; // 返回维度
	}
	return 0;
}

bool GMLuaArgumentsPrivate::getMat4(GMint32 index, REF GMFloat4(&values)[4])
{
	if (lua_istable(L, index))
	{
		// 检查里面的index 0,1,2,3是否为vec4
		GMint32 i;
		for (i = 1; i < 5; ++i)
		{
			GMfloat v[4];
			lua_geti(L, lua_gettop(L), i); // 先装载这个vec object
			if (4 != getVec(index + 1, v)) // vec object放在了下一个位置
			{
				lua_pop(L, 1);
				return false; // 如果中途失败，values的值其实是写到一半，被破坏的。
			}
			lua_pop(L, 1);
			values[i - 1] = GMFloat4(v[0], v[1], v[2], v[3]);
		}
		return true;
	}
	return false;
}

GMLuaArguments::GMLuaArguments(GMLuaCoreState* l, const GMString& invoker, std::initializer_list<GMMetaMemberType> types)
{
	GM_CREATE_DATA();
	D(d);
	d->types = types;
	d->L = l;
	d->calculateSize();
	d->invoker = invoker;

	GMsize_t sz = d->types.size();
	if (sz > 0)
	{
		// 如果存在size，那么说明是准备获取参数，此时检查参数个数。
		if (lua_gettop(l) < sz)
		{
			// 如果传的参数与实际接收的不一致，那么直接报错
			std::string ivk = invoker.toStdString();
			luaL_error(l, "Arguments count not match. The expected count is %i but current is %i.", static_cast<GMint32>(sz), lua_gettop(l));
		}
	}
}

GMLuaArguments::~GMLuaArguments()
{

}

bool GMLuaArguments::isValid()
{
	D(d);
	return lua_gettop(d->L) == d->totalSize;
}

GMVariant GMLuaArguments::getArgument(GMint32 index)
{
	D(d);
	// 先从index中找到索引
	GMMetaMemberType type = GMMetaMemberType::Invalid;
	GMint32 i = d->getIndexInStack(0, index, &type);
	
	GMVariant v;
	if (type != GMMetaMemberType::Object)
		v = d->getScalar(i);
	else
		// v = d->getObject(i);

	// 检查这个类型是否与期望的一致
	d->checkType(v, type, index, d->invoker);
	return v;
}

void GMLuaArguments::pushArgument(const GMVariant& arg)
{
	D(d);
	d->push(arg);
}

END_NS