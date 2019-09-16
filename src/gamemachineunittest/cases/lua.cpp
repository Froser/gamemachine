#include "stdafx.h"
#include "lua.h"
#include <linearmath.h>

#define VECTOR2_EQUALS(V, x, y)			( V.getX() == (x) && V.getY() == (y) )
#define VECTOR3_EQUALS(V, x, y, z)		( V.getX() == (x) && V.getY() == (y) && V.getZ() == (z) )
#define VECTOR4_EQUALS(V, x, y, z, w)	( V.getX() == (x) && V.getY() == (y) && V.getZ() == (z) && V.getW() == (w) )

namespace
{
	GM_PRIVATE_OBJECT_ALIGNED(LuaObject)
	{
		gm::GMint32 i;
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
		GM_DECLARE_EMBEDDED_PROPERTY(i, i)
		GM_DECLARE_EMBEDDED_PROPERTY(f, f)
		GM_DECLARE_EMBEDDED_PROPERTY(b, b)
		GM_DECLARE_EMBEDDED_PROPERTY(str, str)
		GM_DECLARE_EMBEDDED_PROPERTY(v2, v2)
		GM_DECLARE_EMBEDDED_PROPERTY(v3, v3)
		GM_DECLARE_EMBEDDED_PROPERTY(v4, v4)
		GM_DECLARE_EMBEDDED_PROPERTY(m, m)

	public:
		LuaObject();

		virtual bool registerMeta() override;
	};

	LuaObject::LuaObject()
	{
		GM_CREATE_DATA();
	}

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

	std::string g_s;
	LuaObject g_o;
	GMVec2 g_v2;
	GMVec3 g_v3;
	GMVec4 g_v4;
	bool g_b;
	gm::GMfloat g_f;
	GMMat4 g_m;

	extern "C"
	{
		// 参数列表：
		// string, vec2, vec3, vec4, bool, float, matrix
		int testScalar(gm::GMLuaCoreState* l)
		{
			gm::GMLuaArguments args(l, "testScalar",
			{
				gm::GMMetaMemberType::String,
				gm::GMMetaMemberType::Vector2,
				gm::GMMetaMemberType::Vector3,
				gm::GMMetaMemberType::Vector4,
				gm::GMMetaMemberType::Boolean,
				gm::GMMetaMemberType::Float,
				gm::GMMetaMemberType::Matrix4x4
			});

			// 在这里添加unittest
			g_s = args.getArgument(0).toString().toStdString();
			g_v2 = args.getArgument(1).toVec2();
			g_v3 = args.getArgument(2).toVec3();
			g_v4 = args.getArgument(3).toVec4();
			g_b = args.getArgument(4).toBool();
			g_f = args.getArgument(5).toFloat();
			g_m = args.getArgument(6).toMat4();

			return 0;
		}

		int dummyMultiResults(gm::GMLuaCoreState* l)
		{
			return gm::GMReturnValues(
				l,
				gm::GMVariant(1),
				gm::GMVariant(true),
				gm::GMVariant(L"GM")
			);
		}
	}
}

cases::Lua::Lua()
{
	m_obj.reset(new LuaObject());
	auto lr = m_lua.runString(s_code);
	GM_ASSERT(lr.state == gm::GMLuaStates::Ok);
}

void cases::Lua::addToUnitTest(UnitTest& ut)
{
	ut.addTestCase("GMLua: 获取全局变量int64", [&]() {
		auto value = m_lua.getFromGlobal("i");
		return value.toInt64() == 5;
	});

	ut.addTestCase("GMLua: 获取全局变量float", [&]() {
		auto value = m_lua.getFromGlobal("f");
		return FuzzyCompare(value.toFloat(), 1.2f);
	});

	ut.addTestCase("GMLua: 获取全局变量bool", [&]() {
		auto value = m_lua.getFromGlobal("b");
		return !value.toBool();
	});

	ut.addTestCase("GMLua: 获取全局变量string", [&]() {
		auto value = m_lua.getFromGlobal("str");
		return value.toString() == "gamemachine";
	});

	ut.addTestCase("GMLua: 获取全局变量并赋值给GMObject", [&]() {
		m_lua.getFromGlobal("meta", *m_obj);
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

	ut.addTestCase("GMLua: C/C++调用语句，调用Lua方法，传入标量，获取标量", [&]() {
		gm::GMLuaResult lr = m_lua.runString(s_invoke);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;

		auto dummy = m_lua.getFromGlobal("dummy").toString();
		if (dummy != "gamemachine")
			return false;

		gm::GMVariant ret;
		lr = m_lua.protectedCall("dummy_function", {"hello "}, &ret, 1);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;
		return ret.toString() == "hello gamemachine";
	});

	ut.addTestCase("GMLua: C/C++调用语句，调用Lua方法，传入标量，获取对象", [&]() {
		gm::GMLuaResult lr = m_lua.runString(s_invoke2);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;

		LuaObject retObj;
		gm::GMVariant ret = &retObj;
		lr = m_lua.protectedCall("dummy_function2", { 5 }, &ret, 1);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;
		return VECTOR4_EQUALS(ret.objectCast<LuaObject*>()->getv4(), 6, 7, 8, 14) && ret.objectCast<LuaObject*>()->geti() == 256;
	});

	/*
	ut.addTestCase("GMLua: C/C++调用语句，调用Lua方法，传入对象，获取对象", [&]() {
		gm::GMLuaResult lr = m_lua.runString(s_invoke3);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;

		LuaObject arg;
		arg.setstr("Howdy!");
		LuaObject retObj;
		gm::GMVariant ret = &retObj;

		lr = m_lua.protectedCall("dummy_function3", { &arg }, &ret, 1);
		if (lr.state != gm::GMLuaStates::Ok)
			return false;
		return ret.objectCast<LuaObject*>()->getstr() == "Howdy!" && ret.objectCast<LuaObject*>()->geti() == 0 && ret.objectCast<LuaObject*>()->getb() == false; //...未赋值的部分为默认值
	});
	*/

	class DummyInterface : public gm::GMLuaFunctionRegister
	{
	public:
		virtual void registerFunctions(gm::GMLua* l) override
		{
			setRegisterFunction(l, "UnitTest", regCallback, true);
		}

		static int regCallback(gm::GMLuaCoreState* l)
		{
			static gm::GMLuaReg r[] = {
				{ "testScalar", testScalar },
				{ "multi", dummyMultiResults },
				{ 0 }
			};
			newLibrary(l, r);
			return 1;
		}
	};

	ut.addTestCase("GMLua: Lua调用C/C++方法，获取变量", [&]() {
		DummyInterface d;
		d.registerFunctions(&m_lua);
		auto r = m_lua.runString(
			"UnitTest.testScalar([[gamemachine]], {1, 2}, {3, 4, 5}, {6, 7, 8, 9}, true, 16.0, { {1, 2, 3, 4}, {1, 2, 3, 4}, {1, 2, 3, 4}, {1, 2, 3, 4} })");
		if (r.state != gm::GMLuaStates::Ok)
			return false;

		return g_s == "gamemachine" 
			// && VECTOR4_EQUALS(g_o.getv4(), 1, 2, 3, 4)
			&& VECTOR2_EQUALS(g_v2, 1, 2)
			&& VECTOR3_EQUALS(g_v3, 3, 4, 5)
			&& VECTOR4_EQUALS(g_v4, 6, 7, 8, 9)
			&& g_b
			&& FuzzyCompare(g_f, 16.f)
			&& VECTOR4_EQUALS(g_m[0], 1, 2, 3, 4)
			&& VECTOR4_EQUALS(g_m[1], 1, 2, 3, 4)
			&& VECTOR4_EQUALS(g_m[2], 1, 2, 3, 4)
			&& VECTOR4_EQUALS(g_m[3], 1, 2, 3, 4)
			;
	});

	ut.addTestCase("GMLua: Lua调用C/C++方法，获取多个返回值", [&]() {
		auto r = m_lua.runString("g_i, g_b, g_s = UnitTest.multi();");
		if (r.state != gm::GMLuaStates::Ok)
			return false;

		auto i = m_lua.getFromGlobal("g_i").toInt();
		auto b = m_lua.getFromGlobal("g_b").toBool();
		auto s = m_lua.getFromGlobal("g_s").toString();
		return i == 1 && b && s == "GM";
	});
}