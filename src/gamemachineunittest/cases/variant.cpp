#include "stdafx.h"
#include "variant.h"
#include <gmvariant.h>

void cases::Variant::addToUnitTest(UnitTest& ut)
{
	ut.addTestCase("Construct i32", []() {
		gm::GMVariant v_i32(-100);
		return v_i32.toInt() == -100;
	});

	ut.addTestCase("Construct i64", []() {
		gm::GMVariant v_i64(100ll);
		return v_i64.toInt64() == 100ll;
	});

	ut.addTestCase("Construct uint", []() {
		gm::GMVariant v_u(100u);
		return v_u.toUInt() == 100u;
	});

	ut.addTestCase("Construct float", []() {
		gm::GMVariant v_f(1.0f);
		return v_f.toFloat() == 1.f;
	});

	ut.addTestCase("Construct bool", []() {
		gm::GMVariant v_b(true);
		return v_b.toBool();
	});

	ut.addTestCase("Construct string", []() {
		gm::GMVariant v_str(gm::GMString(L"hello"));
		return v_str.toString() == L"hello";
	});

	ut.addTestCase("Construct pointer", []() {
		const char* hello = "hello";
		gm::GMVariant v_p((void*)hello);
		return v_p.toPointer() == hello;
	});

	ut.addTestCase("Construct vec2", []() {
		gm::GMVariant v_v2 = GMVec2(0, 1);
		return v_v2.toVec2() == GMVec2(0, 1);
	});

	ut.addTestCase("Construct vec3", []() {
		gm::GMVariant v_v3 = GMVec3(0, 1, 2);
		return v_v3.toVec3() == GMVec3(0, 1, 2);
	});

	ut.addTestCase("Construct vec4", []() {
		gm::GMVariant v_v4 = GMVec4(0, 1, 2, 3);
		return v_v4.toVec4() == GMVec4(0, 1, 2, 3);
	});

	ut.addTestCase("Construct quat", []() {
		gm::GMVariant v_q = GMQuat(0, 0, 0, 1);
		return v_q.toQuat() == GMQuat(0, 0, 0, 1);
	});
	
	ut.addTestCase("Construct mat4", []() {
		gm::GMVariant v_mat = Identity<GMMat4>();
		return v_mat.toMat4()[0] == Identity<GMMat4>()[0] &&
			v_mat.toMat4()[1] == Identity<GMMat4>()[1] && 
			v_mat.toMat4()[2] == Identity<GMMat4>()[2] && 
			v_mat.toMat4()[3] == Identity<GMMat4>()[3];
	});

	ut.addTestCase("Left value copy construct", []() {
		gm::GMVariant v1 = 5;
		gm::GMVariant v2 = v1;
		return v2.toInt() == 5;
	});

	ut.addTestCase("Left value copy construct string", []() {
		gm::GMVariant v1 = gm::GMString("Hello");
		gm::GMVariant v2 = v1;
		return v2.toString() == gm::GMString("Hello");
	});

	ut.addTestCase("Right value copy construct", []() {
		gm::GMVariant v1 = std::move(gm::GMVariant(gm::GMString("Hello")));
		return v1.toString() == gm::GMString("Hello");
	});
}
