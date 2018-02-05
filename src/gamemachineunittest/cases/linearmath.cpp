#include "stdafx.h"
#include <functional>
#include "linearmath.h"

#define VECTOR2_EQUALS(V, x, y)			( V.getX() == (x) && V.getY() == (y) )
#define VECTOR3_EQUALS(V, x, y, z)		( V.getX() == (x) && V.getY() == (y) && V.getZ() == (z) )
#define VECTOR4_EQUALS(V, x, y, z, w)	( V.getX() == (x) && V.getY() == (y) && V.getZ() == (z) && V.getW() == (w) )

void cases::LinearMath::addToUnitTest(UnitTest& ut)
{
	// GMVec2
	ut.addTestCase("GMVec2::GMVec2(float)", []() {
		GMVec2 V(1);
		return VECTOR2_EQUALS(V, 1, 1);
	});

	ut.addTestCase("GMVec2::GMVec2(float, float)", []() {
		GMVec2 V(1, 2);
		return VECTOR2_EQUALS(V, 1, 2);
	});

	ut.addTestCase("GMVec2::GMVec2(GMVec2)", []() {
		GMVec2 V(1, 2);
		GMVec2 R(V);
		return VECTOR2_EQUALS(R, 1, 2);
	});

	ut.addTestCase("GMVec2::LoadFloat4", []() {
		GMVec2 V(1, 2);
		GMFloat4 f4;
		V.loadFloat4(f4);
		return f4[0] == 1 && f4[1] == 2;
	});

	ut.addTestCase("GMVec2::SetFloat4", []() {
		GMVec2 V(1, 2);
		GMFloat4 f4;
		V.loadFloat4(f4);
		f4[1] = 3;
		V.setFloat4(f4);
		return VECTOR2_EQUALS(V, 1, 3);
	});

	// GMVec3
	ut.addTestCase("GMVec3::GMVec3(float)", []() {
		GMVec3 V(1);
		return VECTOR3_EQUALS(V, 1, 1, 1);
	});

	ut.addTestCase("GMVec3::GMVec3(float, float, float)", []() {
		GMVec3 V(1, 2, 3);
		return VECTOR3_EQUALS(V, 1, 2, 3);
	});

	ut.addTestCase("GMVec3::GMVec3(GMVec3)", []() {
		GMVec3 V(1, 2, 3);
		GMVec3 R(V);
		return VECTOR3_EQUALS(R, 1, 2, 3);
	});

	ut.addTestCase("GMVec3::LoadFloat4", []() {
		GMVec3 V(1, 2, 3);
		GMFloat4 f4;
		V.loadFloat4(f4);
		return f4[0] == 1 && f4[1] == 2 && f4[2] == 3;
	});

	ut.addTestCase("GMVec3::SetFloat4", []() {
		GMVec3 V(1, 2, 3);
		GMFloat4 f4;
		V.loadFloat4(f4);
		f4[2] = 4;
		V.setFloat4(f4);
		return VECTOR3_EQUALS(V, 1, 2, 4);
	});

	// GMVec4
	ut.addTestCase("GMVec4::GMVec4(float)", []() {
		GMVec4 V(1);
		return VECTOR4_EQUALS(V, 1, 1, 1, 1);
	});

	ut.addTestCase("GMVec4::GMVec4(float, float, float, float)", []() {
		GMVec4 V(1, 2, 3, 4);
		return VECTOR4_EQUALS(V, 1, 2, 3, 4);
	});

	ut.addTestCase("GMVec4::GMVec4(GMVec4)", []() {
		GMVec4 V(1, 2, 3, 4);
		GMVec4 R(V);
		return VECTOR4_EQUALS(R, 1, 2, 3, 4);
	});

	ut.addTestCase("GMVec4::LoadFloat4", []() {
		GMVec4 V(1, 2, 3, 4);
		GMFloat4 f4;
		V.loadFloat4(f4);
		return f4[0] == 1 && f4[1] == 2 && f4[2] == 3 && f4[3] == 4;
	});

	ut.addTestCase("GMVec4::SetFloat4", []() {
		GMVec4 V(1, 2, 3, 4);
		GMFloat4 f4;
		V.loadFloat4(f4);
		f4[3] = 5;
		V.setFloat4(f4);
		return V.getX() == 1 && V.getY() == 2 && V.getZ() == 3 && V.getW() == 5;
	});

	// Zero
	ut.addTestCase("GMVec2 Zero", []() {
		typedef GMVec2 TestType;
		TestType V = Zero<TestType>();
		return VECTOR2_EQUALS(V, 0, 0);
	});

	ut.addTestCase("GMVec3 Zero", []() {
		typedef GMVec3 TestType;
		TestType V = Zero<TestType>();
		return VECTOR3_EQUALS(V, 0, 0, 0);
	});

	ut.addTestCase("GMVec4 Zero", []() {
		typedef GMVec4 TestType;
		TestType V = Zero<TestType>();
		return VECTOR4_EQUALS(V, 0, 0, 0, 0);
	});

	// Operators
	// + -
	ut.addTestCase("GMVec2 - GMVec2", []() {
		GMVec2 v1(4, 3), v2(3, 2);
		GMVec2 V = v1 - v2;
		return VECTOR2_EQUALS(V, 1, 1);
	});

	ut.addTestCase("GMVec2 + GMVec2", []() {
		GMVec2 v1(4, 3), v2(3, 2);
		GMVec2 V = v1 + v2;
		return VECTOR2_EQUALS(V, 7, 5);
	});

	ut.addTestCase("GMVec3 - GMVec3", []() {
		GMVec3 v1(4, 3, 2), v2(3, 2, 1);
		GMVec3 V = v1 - v2;
		return VECTOR3_EQUALS(V, 1, 1, 1);
	});

	ut.addTestCase("GMVec3 + GMVec3", []() {
		GMVec3 v1(4, 3, 2), v2(3, 2, 1);
		GMVec3 V = v1 + v2;
		return VECTOR3_EQUALS(V, 7, 5, 3);
	});

	ut.addTestCase("GMVec4 - GMVec4", []() {
		GMVec4 v1(4, 3, 2, 1), v2(3, 2, 1, 0);
		GMVec4 V = v1 - v2;
		return VECTOR4_EQUALS(V, 1, 1, 1, 1);
	});

	ut.addTestCase("GMVec4 + GMVec4", []() {
		GMVec4 v1(4, 3, 2, 1), v2(3, 2, 1, 0);
		GMVec4 V = v1 + v2;
		return VECTOR4_EQUALS(V, 7, 5, 3, 1);
	});


	// *
	ut.addTestCase("GMVec3 * GMVec3", []() {
		GMVec3 v1(4, 3, 2), v2(3, 2, 1);
		GMVec3 V = v1 * v2;
		return VECTOR3_EQUALS(V, 12, 6, 2);
	});

	ut.addTestCase("GMVec3 * float", []() {
		GMVec3 v1(4, 3, 2);
		gm::GMfloat m = 3;
		GMVec3 V = v1 * m;
		return VECTOR3_EQUALS(V, 12, 9, 6);
	});

	ut.addTestCase("GMVec3 / float", []() {
		GMVec3 v1(8, 4, 2);
		gm::GMfloat m = 2;
		GMVec3 V = v1 / m;
		return VECTOR3_EQUALS(V, 4, 2, 1);
	});

	ut.addTestCase("GMVec3 /=", []() {
		GMVec3 V(8, 4, 2);
		V /= 2;
		return VECTOR3_EQUALS(V, 4, 2, 1);
	});

	ut.addTestCase("GMVec4 * float", []() {
		GMVec4 v1(4, 3, 2, 1);
		gm::GMfloat m = 3;
		GMVec4 V = v1 * m;
		return VECTOR4_EQUALS(V, 12, 9, 6, 3);
	});

	ut.addTestCase("GMVec3 ==", []() {
		GMVec4 V1(16, 8, 4, 2), V2(16, 8, 4, 2);
		return V1 == V2;
	});

	// ValuePointer
	ut.addTestCase("ValuePointer(GMVec2)", []() {
		GMVec2 V1(16, 8);
		gm::GMfloat* ptr = ValuePointer(V1);
		return ptr[0] == 16 && ptr[1] == 8;
	});

	ut.addTestCase("ValuePointer(GMVec3)", []() {
		GMVec3 V1(16, 8, 4);
		gm::GMfloat* ptr = ValuePointer(V1);
		return ptr[0] == 16 && ptr[1] == 8 && ptr[2] == 4;
	});

	ut.addTestCase("ValuePointer(GMVec4)", []() {
		GMVec4 V1(16, 8, 4, 2);
		gm::GMfloat* ptr = ValuePointer(V1);
		return ptr[0] == 16 && ptr[1] == 8 && ptr[2] == 4 && ptr[3] == 2;
	});

	// CopyToArray
	ut.addTestCase("CopyToArray(GMVec3)", []() {
		gm::GMfloat ptr[3];
		GMVec3 V1(16, 8, 4);
		CopyToArray(V1, ptr);
		return ptr[0] == 16 && ptr[1] == 8 && ptr[2] == 4;
	});

	ut.addTestCase("CopyToArray(GMVec4)", []() {
		gm::GMfloat ptr[4];
		GMVec4 V1(16, 8, 4, 2);
		CopyToArray(V1, ptr);
		return ptr[0] == 16 && ptr[1] == 8 && ptr[2] == 4 && ptr[3] == 2;
	});

	// Matrix
	ut.addTestCase("Identity<GMMat4>()", []() {
		GMMat4 M = Identity<GMMat4>();
		return
			VECTOR4_EQUALS(M[0], 1, 0, 0, 0) &&
			VECTOR4_EQUALS(M[1], 0, 1, 0, 0) &&
			VECTOR4_EQUALS(M[2], 0, 0, 1, 0) &&
			VECTOR4_EQUALS(M[3], 0, 0, 0, 1);
	});

	ut.addTestCase("Matrix SetFloat16()", []() {
		GMFloat16 f16;
		f16[0] = GMFloat4(1, 2, 3, 4);
		f16[1] = GMFloat4(5, 6, 7, 8);
		f16[2] = GMFloat4(9, 10, 11, 12);
		f16[3] = GMFloat4(13, 14, 15, 16);
		GMMat4 M;
		M.setFloat16(f16);
		return
			VECTOR4_EQUALS(M[0], 1, 2, 3, 4) &&
			VECTOR4_EQUALS(M[1], 5, 6, 7, 8) &&
			VECTOR4_EQUALS(M[2], 9, 10, 11, 12) &&
			VECTOR4_EQUALS(M[3], 13, 14, 15, 16);
	});

	ut.addTestCase("GMMat4::GMMat4(GMMat4)", []() {
		GMMat4 M = Identity<GMMat4>();
		GMMat4 N(M);
		return
			VECTOR4_EQUALS(N[0], 1, 0, 0, 0) &&
			VECTOR4_EQUALS(N[1], 0, 1, 0, 0) &&
			VECTOR4_EQUALS(N[2], 0, 0, 1, 0) &&
			VECTOR4_EQUALS(N[3], 0, 0, 0, 1);
	});

	// Calculations
	ut.addTestCase("Inhomogeneous()", []() {
		GMVec4 T(2, 4, 8, 2);
		GMVec3 V = Inhomogeneous(T);
		return V == GMVec3(1, 2, 4);
	});

	ut.addTestCase("Inhomogeneous()", []() {
		GMFloat16 f16;
		f16[0] = GMFloat4(1, 2, 3, 4);
		f16[1] = GMFloat4(5, 6, 7, 8);
		f16[2] = GMFloat4(9, 10, 11, 12);
		f16[3] = GMFloat4(13, 14, 15, 16);
		GMMat4 M;
		M.setFloat16(f16);
		GMMat4 R = Inhomogeneous(M);
		return
			VECTOR4_EQUALS(R[0], 1, 2, 3, 0) &&
			VECTOR4_EQUALS(R[1], 5, 6, 7, 0) &&
			VECTOR4_EQUALS(R[2], 9, 10, 11, 0) &&
			VECTOR4_EQUALS(R[3], 0, 0, 0, 1);
	});
}