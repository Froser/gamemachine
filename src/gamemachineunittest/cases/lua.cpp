#include "stdafx.h"
#include "lua.h"
#include <linearmath.h>

#define VECTOR2_EQUALS(V, x, y)			( V.getX() == (x) && V.getY() == (y) )
#define VECTOR3_EQUALS(V, x, y, z)		( V.getX() == (x) && V.getY() == (y) && V.getZ() == (z) )
#define VECTOR4_EQUALS(V, x, y, z, w)	( V.getX() == (x) && V.getY() == (y) && V.getZ() == (z) && V.getW() == (w) )

namespace
{
	GM_PRIVATE_OBJECT(LuaObject)
	{
		gm::GMint i;
		gm::GMfloat f;
		bool b;
		gm::GMString str;
		GMVec2 v2;
		GMVec3 v3;
		GMVec4 v4;
		GMMat4 m;
	};

	class LuaObject : public gm::GMObject
	{
		GM_DECLARE_PRIVATE(LuaObject)
		GM_DECLARE_PROPERTY(i, i, gm::GMint)
		GM_DECLARE_PROPERTY(f, f, gm::GMfloat)
		GM_DECLARE_PROPERTY(b, b, bool)
		GM_DECLARE_PROPERTY(str, str, gm::GMString)
		GM_DECLARE_PROPERTY(v2, v2, GMVec2)
		GM_DECLARE_PROPERTY(v3, v3, GMVec3)
		GM_DECLARE_PROPERTY(v4, v4, GMVec4)
		GM_DECLARE_PROPERTY(m, m, GMMat4)

	public:
		virtual bool registerMeta() override;
	};

	bool LuaObject::registerMeta()
	{
		GM_META(i);
		GM_META(f);
		GM_META(b);
		GM_META(str);
		GM_META(v2);
		GM_META(v3);
		GM_META(v4);
		GM_META(m);
		return true;
	}

	const char* s_code =
		"i = 5;"
		"f = 1.2;"
		"b = false;"
		"str = [[gamemachine]];"
		"meta = {};"
		"meta.i = 5;"
		"meta.f = 1.2;"
		"meta.b = false;"
		"meta.str = [[gamemachine]];"
		"meta.v2 = {1, 2, 3};" //故意越界
		"meta.v3 = {1, 2, 3};"
		"meta.v4 = {1, 2, 3, 4};"
		"meta.m = { {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16} }"
		;

	const char* s_invoke =
		"dummy = [[gamemachine]];\n"
		"function dummy_function(prefix)\n"
		"  return prefix .. dummy;\n"
		"end"
		;

	const char* s_invoke2 =
		"function dummy_function2(i)"
		"  local tb = {};"
		"  tb.i = 256;"
		"  tb.v4 = {6, 7, 8, 9 + i};"
		"  return tb;"
		"end";

	const char* s_invoke3 =
		"function dummy_function3(i)"
		"  local tb = {};"
		"  tb.str = i.str;"
		"  return tb;"
		"end";
}

cases::Lua::Lua()
{
	m_obj.reset(new LuaObject());
	auto lr = m_lua.runString(s_code);
	GM_ASSERT(lr.state == gm::GMLuaStates::Ok);
}

void cases::Lua::addToUnitTest(UnitTest& ut)
{
	ut.addTestCase("获取全局变量int64", [&]() {
		auto value = m_lua.getGlobal("i");
		return value.toInt64() == 5;
	});

	ut.addTestCase("获取全局变量float", [&]() {
		auto value = m_lua.getGlobal("f");
		return FuzzyCompare(value.toFloat(), 1.2f);
	});

	ut.addTestCase("获取全局变量bool", [&]() {
		auto value = m_lua.getGlobal("b");
		return !value.toBool();
	});

	ut.addTestCase("获取全局变量string", [&]() {
		auto value = m_lua.getGlobal("str");
		return value.toString() == "gamemachine";
	});

	ut.addTestCase("获取全局变量并赋值给GMObject", [&]() {
		m_lua.getGlobal("meta", *m_obj);
		LuaObject* obj = gm::gm_cast<LuaObject*>(m_obj.get());

		bool bScalar =
			obj->geti() == 5
			&& FuzzyCompare(obj->getf(), 1.2f)
			&& obj->getb() == false
			&& obj->getstr() == "gamemachine";

		bool bV2 = VECTOR2_EQUALS(obj->getv2(), 1, 2);
		bool bV3 = VECTOR3_EQUALS(obj->getv3(), 1, 2, 3);
		bool bV4 = VECTOR4_EQUALS(obj->getv4(), 1, 2, 3, 4);
		bool bM = VECTOR4_EQUALS(obj->getm()[0], 1, 2, 3, 4)
			&& VECTOR4_EQUALS(obj->getm()[1], 5, 6, 7, 8)
			&& VECTOR4_EQUALS(obj->getm()[2], 9, 10, 11, 12)
			&& VECTOR4_EQUALS(obj->getm()[3], 13, 14, 15, 16);

		return bScalar && bV2 && bV3 && bV4 && bM;
	});

	ut.addTestCase("C/C++调用语句，调用Lua方法，传入标量，获取标量", [&]() {
		gm::GMLuaResult lr = m_lua.runString(s_invoke);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;

		auto dummy = m_lua.getGlobal("dummy").toString();
		if (dummy != "gamemachine")
			return false;

		gm::GMVariant ret;
		lr = m_lua.protectedCall("dummy_function", {"hello "}, &ret, 1);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;
		return ret.toString() == "hello gamemachine";
	});

	ut.addTestCase("C/C++调用语句，调用Lua方法，传入标量，获取对象", [&]() {
		gm::GMLuaResult lr = m_lua.runString(s_invoke2);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;

		LuaObject ret;
		lr = m_lua.protectedCall("dummy_function2", { 5 }, &ret, 1);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;
		return VECTOR4_EQUALS(ret.getv4(), 6, 7, 8, 14) && ret.geti() == 256;
	});

	ut.addTestCase("C/C++调用语句，调用Lua方法，传入对象，获取对象", [&]() {
		gm::GMLuaResult lr = m_lua.runString(s_invoke3);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;

		LuaObject arg;
		arg.setstr("Howdy!");
		LuaObject ret;
		lr = m_lua.protectedCall("dummy_function3", { &arg }, &ret, 1);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;
		return ret.getstr() == "Howdy!" && ret.geti() == 0 && ret.getb() == false; //...未赋值的部分为默认值
	});
}