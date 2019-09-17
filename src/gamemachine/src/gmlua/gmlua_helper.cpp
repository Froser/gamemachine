#include "stdafx.h"
#include "gmlua.h"

BEGIN_NS

namespace
{
	void getMetaTableAndName(const GMObject& obj, GMString& metatableName, const GMObject** metatable)
	{
		// 看看成员是否有__name，如果有，设置它为元表，否则，设置自己为元表
		static const GMString s_metatableNameKw = L"__name";

		auto meta = obj.meta();
		for (const auto& member : *meta)
		{
			if (member.first == s_metatableNameKw)
				metatableName = *static_cast<GMString*>(member.second.ptr);
		}
		if (metatable)
			*metatable = &obj;
	}
}

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
	void pushObject(const GMObject& obj);
	void setMembers(const GMObject& obj);
	void setMetaTables(const GMObject& obj);
	GMVariant getScalar(GMint32 index);
	bool getObject(GMint32 index, REF GMObject* objRef);
	GMint32 getIndexInStack(GMint32 offset, GMint32 index, OUT GMMetaMemberType* type = nullptr);
	void checkType(const GMVariant& v, GMMetaMemberType mt, GMint32 index, const GMString& invoker);

	//! 是否某个栈上的变量是一个向量，如果不是则范围0，如果是则返回向量的维度(2-4)。
	GMint32 getVec(GMint32 index, GMfloat values[4]);
	bool getMat4(GMint32 index, REF GMFloat4 (&values)[4]);
	GMLuaReference ref();
};

template <typename T, GMsize_t sz>
bool contains(const T(&arr)[sz], const T& t)
{
	for (GMsize_t i = 0; i < sz; ++i)
	{
		if (arr[i] == t)
			return true;
	}
	return false;
}

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
		GMObject* obj = var.toObject();
		if (obj)
			pushObject(*obj);
		else
			lua_pushnil(L);
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

void GMLuaArgumentsPrivate::pushObject(const GMObject& obj)
{
	lua_newtable(L);
	setMetaTables(obj);
	setMembers(obj);
}

void GMLuaArgumentsPrivate::setMetaTables(const GMObject& obj)
{
	static const GMString s_overrideList[] =
	{
		L"__index",
		L"__newindex",
		L"__gc",
	};

	auto tableIdx = lua_gettop(L);
	GMString metatableName;
	const GMObject* metaTable = nullptr;
	getMetaTableAndName(obj, metatableName, &metaTable);
	GM_ASSERT(metaTable);

	luaL_newmetatable(L, metatableName.toStdString().c_str());
	if (!metatableName.isEmpty())
	{
		// 存在meta数据
		for (const auto& member : *metaTable->meta())
		{
			// 只为几个默认的meta写方法
			if (contains(s_overrideList, member.first) &&
				member.second.type == GMMetaMemberType::Function)
			{
				std::string name = member.first.toStdString();
				lua_pushstring(L, name.c_str());
				lua_pushcfunction(L, (GMLuaCFunction)(member.second.ptr));
				lua_rawset(L, -3);
			}
		}
	}

	lua_setmetatable(L, tableIdx);
}

void GMLuaArgumentsPrivate::setMembers(const GMObject& obj)
{
	GM_ASSERT(lua_istable(L, -1));
	auto meta = obj.meta();
	if (meta)
	{
		for (const auto& member : *meta)
		{
			// 将非元表函数放入表格
			if (!member.first.startsWith(L"__") || member.second.type != GMMetaMemberType::Function)
			{
				push(member.first); //key
				if (member.second.type == GMMetaMemberType::Function)
				{
					lua_pushcfunction(L, (GMLuaCFunction)(member.second.ptr));
				}
				else if (member.second.type == GMMetaMemberType::Pointer)
				{
					GM_STATIC_ASSERT(sizeof(lua_Integer) >= sizeof(void*), "Pointer size incompatible.");
					GM_STATIC_ASSERT_SIZE(GMsize_t, sizeof(void*));
					lua_Integer address = lua_tointeger(L, -1);
					lua_pushinteger(L, *static_cast<GMsize_t*>(member.second.ptr));
				}
				else if (member.second.type == GMMetaMemberType::Object)
				{
					push(*reinterpret_cast<GMObject**>(member.second.ptr));
				}
				else
				{
					push(member.second);
				}

				GM_ASSERT(lua_istable(L, -3));
				lua_settable(L, -3);
			}
		}
	}
}

void GMLuaArgumentsPrivate::pushMatrix(const GMMat4& v)
{
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

bool GMLuaArgumentsPrivate::getObject(GMint32 index, REF GMObject* objRef)
{
	if (!objRef)
		return false;

	const GMMeta* meta = objRef->meta();
	if (!meta)
		return false;

	if (!lua_istable(L, index))
	{
		luaL_error(L, "Lua object type is %s. Table type is expected.", lua_typename(L, lua_type(L, index)));
		return false;
	}
	
	bool found = false;

	// 之后可能要用到的临时变量
	GMfloat t[4];
	GMFloat16 f16;

	// 开始遍历entries
	lua_pushnil(L);
	while (lua_next(L, index))
	{
		GM_ASSERT(lua_isstring(L, -2));
		const char* key = lua_tostring(L, -2);
		for (const auto member : *meta)
		{
			if (member.first == key)
			{
				switch (member.second.type)
				{
				case GMMetaMemberType::String:
					*(static_cast<GMString*>(member.second.ptr)) = lua_tostring(L, -1);
					found = true;
					break;
				case GMMetaMemberType::Float:
					*(static_cast<GMfloat*>(member.second.ptr)) = lua_tonumber(L, -1);
					found = true;
					break;
				case GMMetaMemberType::Boolean:
					*(static_cast<bool*>(member.second.ptr)) = !!lua_toboolean(L, -1);
					found = true;
					break;
				case GMMetaMemberType::Function:
				{
					*(static_cast<GMLuaReference*>(member.second.ptr)) = ref();
					found = true;
					break;
				}
				case GMMetaMemberType::Int:
				{
					if (lua_isfunction(L, -1))
						*(static_cast<GMLuaReference*>(member.second.ptr)) = ref();
					else
						*(static_cast<GMint32*>(member.second.ptr)) = lua_tointeger(L, -1);
					break;
				}
				case GMMetaMemberType::Vector2:
				{
					GMVec2& v = *static_cast<GMVec2*>(member.second.ptr);
					GMint32 top = lua_gettop(L);
					if (getVec(top, t))
					{
						v = GMVec2(t[0], t[1]);
						found = true;
					}
					break;
				}
				case GMMetaMemberType::Vector3:
				{
					GMVec3& v = *static_cast<GMVec3*>(member.second.ptr);
					GMint32 top = lua_gettop(L);
					if (getVec(top, t))
					{
						v = GMVec3(t[0], t[1], t[2]);
						found = true;
					}
					break;
				}
				case GMMetaMemberType::Vector4:
				{
					GMVec4& v = *static_cast<GMVec4*>(member.second.ptr);
					GMint32 top = lua_gettop(L);
					if (getVec(top, t))
					{
						v = GMVec4(t[0], t[1], t[2], t[3]);
						found = true;
					}
					break;
				}
				case GMMetaMemberType::Matrix4x4:
				{
					GMMat4& mat = *static_cast<GMMat4*>(member.second.ptr);
					GMint32 top = lua_gettop(L);
					if (getMat4(top, f16.v_))
					{
						mat.setFloat16(f16);
						found = true;
					}
					break;
				}
				case GMMetaMemberType::Object:
				{
					bool b = getObject(lua_gettop(L), *static_cast<GMObject**>(member.second.ptr));
					if (!b)
						found = false;
					break;
				}
				case GMMetaMemberType::Pointer:
				{
					GM_STATIC_ASSERT_SIZE(GMsize_t, sizeof(void*));
					lua_Integer address = lua_tointeger(L, -1);
					*(static_cast<GMsize_t*>(member.second.ptr)) = address;
					found = true;
					break;
				}
				default:
					break;
				}
			}
		}

		lua_pop(L, 1); // 移除value，使用key来进行下一个迭代
	}

	return found;
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
		luaL_error(L, "Type is not match at index %i in %s. The type of passed variable is %s.", index, ivk.c_str(), lua_typename(L, lua_type(L, index)));
	}
}

GMint32 GMLuaArgumentsPrivate::getVec(GMint32 index, GMfloat values[4])
{
	if (lua_istable(L, index))
	{
		GM_ASSERT(values);
		// 此时此stack已经是个表了
		// 检查里面的索引 0,1,2,3是否为数字
		GMint32 i;
		for (i = 1; i < 5; ++i)
		{
			lua_geti(L, index, i);
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
		// 此时此stack已经是个表了
		// 检查里面的索引 0,1,2,3是否为vec4
		GMint32 i;
		for (i = 1; i < 5; ++i)
		{
			GMfloat v[4];
			lua_geti(L, index, i); // 先装载这个vec object，放在了栈顶
			GMint32 top = lua_gettop(L);
			if (4 != getVec(top, v)) // vec object放在了下一个位置
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

GMLuaReference GMLuaArgumentsPrivate::ref()
{
	if (lua_isfunction(L, -1))
	{
		GMLuaReference r = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushnil(L);
		return r;
	}

	luaL_error(L, "Current top of the stack is not a function.");
	return 0;
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

GMVariant GMLuaArguments::getArgument(GMint32 index, REF GMObject* objRef)
{
	D(d);
	if (!d->types.empty()) // 如果types不为空，表示获取的是传入参数
	{
		// 先从index中找到索引
		GMMetaMemberType type = GMMetaMemberType::Invalid;
		GMint32 i = d->getIndexInStack(0, index, &type);

		GMVariant v;
		if (type != GMMetaMemberType::Object)
			v = d->getScalar(i);
		else
			v = d->getObject(i, objRef);

		// 检查这个类型是否与期望的一致
		if (type != GMMetaMemberType::Object)
		{
			d->checkType(v, type, index, d->invoker);
		}
		else
		{
			if (!objRef)
				luaL_error(d->L, "Type is not match. The expected type is object.");
		}

		return v;
	}
	else // 如果types为空，表示获取返回值
	{
		GMint32 i = d->getIndexInStack(0, index);

		GMVariant v;
		if (!objRef)
			v = d->getScalar(i);
		else
			v = d->getObject(i, objRef);

		return v;
	}
}

void GMLuaArguments::pushArgument(const GMVariant& arg)
{
	D(d);
	d->push(arg);
}

END_NS