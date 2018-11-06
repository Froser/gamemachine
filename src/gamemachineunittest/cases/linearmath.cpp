#include "stdafx.h"
#include <functional>
#include "linearmath.h"

#define VECTOR2_EQUALS(V, x, y)			( V.getX() == (x) && V.getY() == (y) )
#define VECTOR3_EQUALS(V, x, y, z)		( V.getX() == (x) && V.getY() == (y) && V.getZ() == (z) )
#define VECTOR4_EQUALS(V, x, y, z, w)	( V.getX() == (x) && V.getY() == (y) && V.getZ() == (z) && V.getW() == (w) )

#define VECTOR3_FUZZY_EQUALS(V, x, y, z)	(FuzzyCompare(V.getX(), (x)) && FuzzyCompare(V.getY(), (y)) && FuzzyCompare(V.getZ(), (z)) )
#define VECTOR4_FUZZY_EQUALS(V, x, y, z, w)	(FuzzyCompare(V.getX(), (x)) && FuzzyCompare(V.getY(), (y)) && FuzzyCompare(V.getZ(), (z)) && FuzzyCompare(V.getW(), (w)))

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

	// GMQuat
	ut.addTestCase("GMQuat::GMQuat(GMVec3, GMVec3)", []() {
		GMQuat Q(GMVec3(1, 0, 0), GMVec3(0, 1, 0));
		return VECTOR4_EQUALS(Q, 0, 0, 0.707106769f, 0.707106769f);
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

	ut.addTestCase("GMVec3 +=", []() {
		GMVec3 v1(4, 3, 2), v2(3, 2, 1);
		v1 += v2;
		return VECTOR3_EQUALS(v1, 7, 5, 3);
	});

	ut.addTestCase("GMVec3 -=", []() {
		GMVec3 v1(4, 3, 2), v2(3, 2, 1);
		v1 -= v2;
		return VECTOR3_EQUALS(v1, 1, 1, 1);
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

	ut.addTestCase("GMQuat * GMQuat", []() {
		GMQuat Q = Rotate(-PI / 3, GMVec3(1, 0, 0)) * Rotate(PI / 6, GMVec3(0, 1, 0));
		return VECTOR4_FUZZY_EQUALS(Q, -0.482962f, 0.224143f, 0.129409f, 0.836516f);
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

	ut.addTestCase("GMVec4 / float", []() {
		GMVec4 v1(16, 8, 4, 2);
		gm::GMfloat m = 2;
		GMVec4 V = v1 / m;
		return VECTOR4_EQUALS(V, 8, 4, 2, 1);
	});

	ut.addTestCase("GMVec4 * float", []() {
		GMVec4 v1(4, 3, 2, 1);
		gm::GMfloat m = 3;
		GMVec4 V = v1 * m;
		return VECTOR4_EQUALS(V, 12, 9, 6, 3);
	});

	ut.addTestCase("GMVec2 ==", []() {
		GMVec2 V1(16, 8), V2(16, 8);
		return V1 == V2;
	});

	ut.addTestCase("GMVec3 ==", []() {
		GMVec3 V1(16, 8, 4), V2(16, 8, 4);
		return V1 == V2;
	});

	ut.addTestCase("GMVec4 ==", []() {
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

	ut.addTestCase("Matrix * Float", []() {
		GMFloat16 f16;
		f16[0] = GMFloat4(1, 2, 3, 4);
		f16[1] = GMFloat4(5, 6, 7, 8);
		f16[2] = GMFloat4(9, 10, 11, 12);
		f16[3] = GMFloat4(13, 14, 15, 16);
		GMMat4 M;
		M.setFloat16(f16);
		M = 2 * M;
		return
			VECTOR4_EQUALS(M[0], 2, 4, 6, 8) &&
			VECTOR4_EQUALS(M[1], 10, 12, 14, 16) &&
			VECTOR4_EQUALS(M[2], 18, 20, 22, 24) &&
			VECTOR4_EQUALS(M[3], 26, 28, 30, 32);
	});

	// Make, Combine
	ut.addTestCase("MakeVector3", []() {
		gm::GMfloat A[] = { 1, 2, 3 };
		GMVec3 V = MakeVector3(A);
		return VECTOR3_EQUALS(V, 1, 2, 3);
	});

	ut.addTestCase("MakeVector3(float*)", []() {
		GMVec4 V(4, 3, 2, 1);
		GMVec3 R = MakeVector3(V);
		return VECTOR3_EQUALS(R, 4, 3, 2);
	});

	ut.addTestCase("MakeVector3(GMVec4)", []() {
		GMVec3 V1(1, 2, 3);
		GMVec4 V2(4, 3, 2, 1);
		GMVec4 R = CombineVector4(V1, V2);
		return VECTOR4_EQUALS(R, 1, 2, 3, 1);
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

	ut.addTestCase("GMMat4 * GMMat4", []() {
		GMFloat16 f16_1, f16_2;
		f16_1[0] = GMFloat4(1, 2, 3, 4);
		f16_1[1] = GMFloat4(5, 6, 7, 8);
		f16_1[2] = GMFloat4(9, 10, 11, 12);
		f16_1[3] = GMFloat4(13, 14, 15, 16);

		f16_2[0] = GMFloat4(17, 18, 19, 20);
		f16_2[1] = GMFloat4(21, 22, 23, 24);
		f16_2[2] = GMFloat4(25, 26, 27, 28);
		f16_2[3] = GMFloat4(29, 30, 31, 32);
		GMMat4 M1, M2;
		M1.setFloat16(f16_1);
		M2.setFloat16(f16_2);
		GMMat4 R = M1 * M2;
		return
			VECTOR4_EQUALS(R[0], 250, 260, 270, 280) &&
			VECTOR4_EQUALS(R[1], 618, 644, 670, 696) &&
			VECTOR4_EQUALS(R[2], 986, 1028, 1070, 1112) &&
			VECTOR4_EQUALS(R[3], 1354, 1412, 1470, 1528);
	});

	ut.addTestCase("GMMat4 + GMMat4", []() {
		GMFloat16 f16_1, f16_2;
		f16_1[0] = GMFloat4(1, 2, 3, 4);
		f16_1[1] = GMFloat4(5, 6, 7, 8);
		f16_1[2] = GMFloat4(9, 10, 11, 12);
		f16_1[3] = GMFloat4(13, 14, 15, 16);

		f16_2[0] = GMFloat4(17, 18, 19, 20);
		f16_2[1] = GMFloat4(21, 22, 23, 24);
		f16_2[2] = GMFloat4(25, 26, 27, 28);
		f16_2[3] = GMFloat4(29, 30, 31, 32);
		GMMat4 M1, M2;
		M1.setFloat16(f16_1);
		M2.setFloat16(f16_2);
		GMMat4 R = M1 + M2;
		return
			VECTOR4_EQUALS(R[0], 18, 20, 22, 24) &&
			VECTOR4_EQUALS(R[1], 26, 28, 30, 32) &&
			VECTOR4_EQUALS(R[2], 34, 36, 38, 40) &&
			VECTOR4_EQUALS(R[3], 42, 44, 46, 48);
	});

	ut.addTestCase("GMVec4 * GMMat4", []() {
		GMVec4 V(1, 2, 3, 4);

		GMFloat16 f16;
		f16[0] = GMFloat4(1, 2, 3, 4);
		f16[1] = GMFloat4(5, 6, 7, 8);
		f16[2] = GMFloat4(9, 10, 11, 12);
		f16[3] = GMFloat4(13, 14, 15, 16);
		GMMat4 M;
		M.setFloat16(f16);
		GMVec4 R = V * M;
		return VECTOR4_EQUALS(R, 90, 100, 110, 120);
	});

	ut.addTestCase("GMVec4 * GMMat4", []() {
		GMVec4 V(1, 2, 3, 4);

		GMFloat16 f16;
		f16[0] = GMFloat4(1, 2, 3, 4);
		f16[1] = GMFloat4(5, 6, 7, 8);
		f16[2] = GMFloat4(9, 10, 11, 12);
		f16[3] = GMFloat4(13, 14, 15, 16);
		GMMat4 M;
		M.setFloat16(f16);
		GMVec4 R = V * M;
		return VECTOR4_EQUALS(R, 90, 100, 110, 120);
	});

	ut.addTestCase("QuatToMatrix()", []() {
		// 注意，只有单位四元数才有意义。这里只进行矩阵的计算。
		GMQuat Q(1, 2, 3, 4);
		GMMat4 M = QuatToMatrix(Q);
		return
			VECTOR4_EQUALS(M[0], -25, 28, -10, 0) &&
			VECTOR4_EQUALS(M[1], -20, -19, 20, 0) &&
			VECTOR4_EQUALS(M[2], 22, 4, -9, 0) &&
			VECTOR4_EQUALS(M[3], 0, 0, 0, 1);
	});

	// TODO LookAt, Ortho, Perspective
	ut.addTestCase("LookAt()", []() {
		GMMat4 V = LookAt(GMVec3(0, 0, 0), GMVec3(1, 1, 1), GMVec3(0, 1, 0));
		return
			VECTOR4_FUZZY_EQUALS(V[0], 0.707106769f, -0.408248276f, 0.577350259f, 0.000000000f) &&
			VECTOR4_FUZZY_EQUALS(V[1], 0.000000000f, 0.816496551f, 0.577350259f, 0.000000000f) &&
			VECTOR4_FUZZY_EQUALS(V[2], -0.707106769f, -0.408248276f, 0.577350259f, 0.000000000f) &&
			VECTOR4_FUZZY_EQUALS(V[3], 0.000000000f, 0.000000000f, 0.000000000f, 1.00000000f);
	});

	ut.addTestCase("Ortho()", []() {
		GMMat4 P = Ortho(-1, 1, -1, 1, 1, 3000);
		return
			VECTOR4_FUZZY_EQUALS(P[0], 1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f) &&
			VECTOR4_FUZZY_EQUALS(P[1], 0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f) &&
			VECTOR4_FUZZY_EQUALS(P[2], 0.000000000f, 0.000000000f, 0.000333444f, 0.000000000f) &&
			VECTOR4_FUZZY_EQUALS(P[3], 0.000000000f, 0.000000000f, -0.000333444f, 1.000000000f);
	});

	ut.addTestCase("Perspective()", []() {
		GMMat4 P = Perspective(1.3f, 1.3f, 1024, 768);
		return
			VECTOR4_FUZZY_EQUALS(P[0], 1.01187372f, 0.000000000f, 0.000000000f, 0.000000000f) &&
			VECTOR4_FUZZY_EQUALS(P[1], 0.000000000f, 1.31543577f, 0.000000000f, 0.000000000f) &&
			VECTOR4_FUZZY_EQUALS(P[2], 0.000000000f, 0.000000000f, -3.00000000f, 1.00000000f) &&
			VECTOR4_FUZZY_EQUALS(P[3], 0.000000000f, 0.000000000f, 3072.00000f, 0.000000000f);
	});

	ut.addTestCase("Unproject()", []() {
		GMMat4 V = LookAt(GMVec3(0, 0, 0), GMVec3(1, 1, 1), GMVec3(0, 1, 0));
		GMMat4 P = Perspective(1.3f, 1.3f, 1024, 768);
		GMVec3 Coord(40, 40, 1);
		GMVec3 W = Normalize(Unproject(Coord, 0, 0, 1024, 768, P, V, Identity<GMMat4>()));
		return VECTOR3_FUZZY_EQUALS(W, -0.2277193f, 0.7483482f, 0.6229918f);
	});

	ut.addTestCase("Dot(GMVec2)", []() {
		GMVec2 V1(1, 2), V2(3, 4);
		gm::GMfloat result = Dot(V1, V2);
		return result == 11;
	});

	ut.addTestCase("Dot(GMVec3)", []() {
		GMVec3 V1(1, 2, 3), V2(4, 5, 6);
		gm::GMfloat result = Dot(V1, V2);
		return result == 32;
	});

	ut.addTestCase("Dot(GMVec4)", []() {
		GMVec4 V1(1, 2, 3, 4), V2(5, 6, 7, 8);
		gm::GMfloat result = Dot(V1, V2);
		return result == 70;
	});

	ut.addTestCase("Normalize(GMVec3)", []() {
		GMVec3 V(1, 2, 3);
		GMVec3 R = Normalize(V);
		gm::GMfloat len = Sqrt(14);
		return VECTOR3_FUZZY_EQUALS(R, 1 / len, 2 / len, 3 / len);
	});

	ut.addTestCase("Normalize(GMVec4)", []() {
		GMVec4 V(1, 2, 3, 4);
		GMVec4 R = Normalize(V);
		gm::GMfloat len = Sqrt(30);
		return VECTOR4_FUZZY_EQUALS(R, 1 / len, 2 / len, 3 / len, 4 / len);
	});

	ut.addTestCase("PlaneNormalize(GMVec4)", []() {
		GMVec4 V(1, 2, 3, 4);
		GMVec4 R = PlaneNormalize(V);
		gm::GMfloat len = Sqrt(14);
		return VECTOR4_FUZZY_EQUALS(R, 1 / len, 2 / len, 3 / len, 4 / len);
	});

	ut.addTestCase("FastNormalize(GMVec3)", []() {
		GMVec3 V(1, 2, 3);
		GMVec3 R = Normalize(V);
		gm::GMfloat len = Sqrt(14);
		return VECTOR3_FUZZY_EQUALS(R, 1 / len, 2 / len, 3 / len);
	});

	ut.addTestCase("Normalize(GMVec3) while Len=0", []() {
		GMVec3 V(0, 0, 0);
		GMVec3 R = Normalize(V);
		bool B1 = VECTOR3_EQUALS(R, 0, 0, 0);

		V = GMVec3(1, 2, 3);
		R = Normalize(V);
		gm::GMfloat len = Sqrt(14);
		bool B2 = VECTOR3_FUZZY_EQUALS(R, 1 / len, 2 / len, 3 / len);

		return B1 && B2;
	});

	// Affines
	ut.addTestCase("Translate(GMVec3)", []() {
		GMMat4 M = Translate(GMVec3(1, 2, 3));
		return
			VECTOR4_EQUALS(M[0], 1, 0, 0, 0) &&
			VECTOR4_EQUALS(M[1], 0, 1, 0, 0) &&
			VECTOR4_EQUALS(M[2], 0, 0, 1, 0) &&
			VECTOR4_EQUALS(M[3], 1, 2, 3, 1);
	});

	ut.addTestCase("Translate(GMVec4)", []() {
		GMMat4 M = Translate(GMVec4(1, 2, 3, 4));
		return
			VECTOR4_EQUALS(M[0], 1, 0, 0, 0) &&
			VECTOR4_EQUALS(M[1], 0, 1, 0, 0) &&
			VECTOR4_EQUALS(M[2], 0, 0, 1, 0) &&
			VECTOR4_EQUALS(M[3], 1, 2, 3, 1);
	});

	ut.addTestCase("Scale(GMVec3)", []() {
		GMMat4 M = Scale(GMVec3(1, 2, 3));
		return
			VECTOR4_EQUALS(M[0], 1, 0, 0, 0) &&
			VECTOR4_EQUALS(M[1], 0, 2, 0, 0) &&
			VECTOR4_EQUALS(M[2], 0, 0, 3, 0) &&
			VECTOR4_EQUALS(M[3], 0, 0, 0, 1);
	});

	// TODO Rotation


	ut.addTestCase("GetTranslationFromMatrix()", []() {
		GMFloat4 F;
		GMMat4 M = Translate(GMVec4(1, 2, 3, 4));
		GetTranslationFromMatrix(M, F);
		return F[0] == 1 && F[1] == 2 && F[2] == 3;
	});

	ut.addTestCase("GetScalingFromMatrix()", []() {
		GMFloat4 F;
		GMMat4 M = Scale(GMVec3(1, 2, 3));
		GetScalingFromMatrix(M, F);
		return F[0] == 1 && F[1] == 2 && F[2] == 3;
	});

	ut.addTestCase("Length(GMVec3)", []() {
		GMVec3 V(1, 2, 3);
		return FuzzyCompare(Sqrt(14), Length(V));
	});

	ut.addTestCase("Length(GMVec4)", []() {
		GMVec4 V(1, 2, 3, 4);
		return FuzzyCompare(Sqrt(30), Length(V));
	});

	ut.addTestCase("LengthSq(GMVec3)", []() {
		GMVec3 V(1, 2, 3);
		return LengthSq(V) == 14;
	});

	ut.addTestCase("Cross(GMVec3)", []() {
		// 注意，只有单位向量叉乘才有意义。这里只进行叉乘的计算。
		GMVec3 V1(1, 2, 3), V2(4, 5, 6);
		GMVec3 R = Cross(V1, V2);
		return VECTOR3_EQUALS(R, -3, 6, -3);
	});

	ut.addTestCase("Transpose(GMMat4)", []() {
		GMFloat16 f16;
		f16[0] = GMFloat4(1, 2, 3, 4);
		f16[1] = GMFloat4(5, 6, 7, 8);
		f16[2] = GMFloat4(9, 10, 11, 12);
		f16[3] = GMFloat4(13, 14, 15, 16);
		GMMat4 M;
		M.setFloat16(f16);
		GMMat4 R = Transpose(M);

		return
			VECTOR4_EQUALS(R[0], 1, 5, 9, 13) &&
			VECTOR4_EQUALS(R[1], 2, 6, 10, 14) &&
			VECTOR4_EQUALS(R[2], 3, 7, 11, 15) &&
			VECTOR4_EQUALS(R[3], 4, 8, 12, 16);
	});

	ut.addTestCase("Inverse(GMMat4)", []() {
		GMFloat16 f16;
		f16[0] = GMFloat4(4, 0, 0, 0);
		f16[1] = GMFloat4(0, 8, 0, 0);
		f16[2] = GMFloat4(0, 0, 16, 0);
		f16[3] = GMFloat4(1, 2, 4, 1);
		GMMat4 M;
		M.setFloat16(f16);
		GMMat4 R = Inverse(M);

		return
			VECTOR4_FUZZY_EQUALS(R[0], .25f, 0, 0, 0) &&
			VECTOR4_FUZZY_EQUALS(R[1], 0, .125f, 0, 0) &&
			VECTOR4_FUZZY_EQUALS(R[2], 0, 0, 0.0625f, 0) &&
			VECTOR4_FUZZY_EQUALS(R[3], -.25f, -.25f, -.25f, 1);
	});

	ut.addTestCase("InverseTranspose(GMMat4)", []() {
		GMFloat16 f16;
		f16[0] = GMFloat4(4, 0, 0, 0);
		f16[1] = GMFloat4(0, 8, 0, 0);
		f16[2] = GMFloat4(0, 0, 16, 0);
		f16[3] = GMFloat4(1, 2, 4, 1);
		GMMat4 M;
		M.setFloat16(f16);
		GMMat4 R = InverseTranspose(M);

		return
			VECTOR4_FUZZY_EQUALS(R[0], .25f, 0, 0, -.25f) &&
			VECTOR4_FUZZY_EQUALS(R[1], 0, .125f, 0, -.25f) &&
			VECTOR4_FUZZY_EQUALS(R[2], 0, 0, 0.0625f, -.25f) &&
			VECTOR4_FUZZY_EQUALS(R[3], 0, 0, 0, 1);
	});

	//TODO Lerp


}