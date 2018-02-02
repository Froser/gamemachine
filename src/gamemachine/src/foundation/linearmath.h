#ifndef __GM_LINEARMATH_H__
#define __GM_LINEARMATH_H__
#include <defines.h>
#include <math.h>
#include <gmdxincludes.h>
#include <gmobject.h>

#if !GM_USE_DX11
// 如果使用DirectX，则不使用glm
#define GLM_FORCE_INLINE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtc/matrix_inverse.inl"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/compatibility.hpp"
#include "glm/gtx/fast_square_root.hpp"
#include "glm/gtx/norm.hpp"
#endif

constexpr gm::GMfloat PI = 3.141592653f;

BEGIN_NS

#ifndef FLT_EPSILON
#	define FLT_EPSILON 1.192092896e-07F
#endif

#define GM_SIMD_EPSILON FLT_EPSILON

// 数学函数
inline GMfloat gmFabs(GMfloat x) { return fabsf(x); }
inline GMfloat gmCos(GMfloat x) { return cosf(x); }
inline GMfloat gmSin(GMfloat x) { return sinf(x); }
inline GMfloat gmTan(GMfloat x) { return tanf(x); }
inline GMfloat gmSqrt(GMfloat x) { return sqrtf(x); }
inline GMfloat gmAcos(GMfloat x)
{
	if (x < GMfloat(-1))
		x = GMfloat(-1);
	if (x > GMfloat(1))
		x = GMfloat(1);
	return acosf(x);
}
inline GMfloat gmAsin(GMfloat x)
{
	if (x < GMfloat(-1))
		x = GMfloat(-1);
	if (x > GMfloat(1))
		x = GMfloat(1);
	return asinf(x);
}
inline GMfloat gmAtan(GMfloat x) { return atanf(x); }
inline GMfloat gmAtan2(GMfloat x, GMfloat y) { return atan2f(x, y); }
inline GMfloat gmExp(GMfloat x) { return expf(x); }
inline GMfloat gmLog(GMfloat x) { return logf(x); }
inline GMfloat gmPow(GMfloat x, GMfloat y) { return powf(x, y); }
inline GMfloat gmFmod(GMfloat x, GMfloat y) { return fmodf(x, y); }
inline GMfloat gmFloor(GMfloat x) { return floor(x); }
inline GMfloat gmMin(GMfloat x, GMfloat y) { return x < y ? x : y; }
inline GMfloat gmMax(GMfloat x, GMfloat y) { return x > y ? x : y; }

END_NS

#if GM_USE_DX11
#define GMMATH_BEGIN_STRUCT(className, glStruct, dxStruct)	\
	GM_ALIGNED_STRUCT(className)				\
	{											\
		dxStruct v_;							\
	public:										\
		className() = default;					\
		className(const className& rhs);

#define GMMATH_LOAD_FLOAT4						\
		void loadFloat4(GMFloat4& f4) const		\
		{										\
			DirectX::XMStoreFloat4(&f4.v_, v_);	\
		}

#define GMMATH_SET_FLOAT4(unused)				\
		void setFloat4(const GMFloat4& f4)		\
		{										\
			v_ = DirectX::XMLoadFloat4(&f4.v_);	\
		}

#else
#define GMMATH_BEGIN_STRUCT(className, glStruct, dxStruct)	\
	GM_ALIGNED_STRUCT(className)				\
	{											\
		glStruct v_;							\
												\
	public:										\
		className() = default;

#define GMMATH_LOAD_FLOAT4						\
		void loadFloat4(GMFloat4* f4) const		\
		{										\
			GM_ASSERT(false);					\
		}

#define GMMATH_SET_FLOAT4(makeFunc)				\
		void setFloat4(const GMFloat4& f4)		\
		{										\
			_v = makeFunc(&f4);					\
		}
#endif
#define GMMATH_END_STRUCT };

struct GMFloat4
{
#if GM_USE_DX11
	enum XYZW
	{
		X = 0,
		Y,
		Z,
		W,
	};

	DirectX::XMFLOAT4 v_;
	gm::GMfloat& operator[](gm::GMint i)
	{
		return i == X ? v_.x :
			i == Y ? v_.y :
			i == Z ? v_.z :
			i == W ? v_.w :
			(GM_ASSERT(false), v_.x);
	}
#else
	gm::GMfloat v_[4];

	gm::GMfloat& operator[](XYZW i)
	{
		return v_[i];
	}
#endif
};

struct GMFloat16
{
	GMFloat4 v_[4];
	GMFloat4 operator[](gm::GMint i)
	{
		return v_[i];
	}
};

GMMATH_BEGIN_STRUCT(GMVec2, glm::vec2, DirectX::XMVECTOR)
GMMATH_LOAD_FLOAT4
GMMATH_SET_FLOAT4(glm::make_vec2)
#if GM_USE_DX11
GMVec2(gm::GMfloat v0)
{
	v_ = DirectX::XMVectorSet(v0, v0, 0, 0);
}

GMVec2(gm::GMfloat v0, gm::GMfloat v1)
{
	v_ = DirectX::XMVectorSet(v0, v1, 0, 0);
}
#else
GMVec2(gm::GMfloat v0)
	: v_(v0)
{
}

GMVec2(gm::GMfloat v0, gm::GMfloat v1)
	: v_(v0, v1)
{
}
#endif
GMMATH_END_STRUCT

GMMATH_BEGIN_STRUCT(GMVec3, glm::vec3, DirectX::XMVECTOR)
GMMATH_SET_FLOAT4(glm::make_vec3)
GMMATH_LOAD_FLOAT4
#if GM_USE_DX11
GMVec3(gm::GMfloat v0)
{
	v_ = DirectX::XMVectorSet(v0, v0, v0, 0);
}

GMVec3(gm::GMfloat v0, gm::GMfloat v1, gm::GMfloat v2)
{
	v_ = DirectX::XMVectorSet(v0, v1, v2, 0);
}
#else
GMVec3(gm::GMfloat v0)
	: v_(v0)
{
}

GMVec3(gm::GMfloat v0, gm::GMfloat v1, gm::GMfloat v2)
	: v_(v0, v1, v2)
{
}
#endif
GMMATH_END_STRUCT

GMMATH_BEGIN_STRUCT(GMVec4, glm::vec4, DirectX::XMVECTOR)
GMMATH_SET_FLOAT4(glm::make_vec4)
GMMATH_LOAD_FLOAT4
#if GM_USE_DX11
GMVec4(gm::GMfloat v0)
{
	v_ = DirectX::XMVectorSet(v0, v0, v0, v0);
}

GMVec4(gm::GMfloat v0, gm::GMfloat v1, gm::GMfloat v2, gm::GMfloat v3)
{
	v_ = DirectX::XMVectorSet(v0, v1, v2, v3);
}
#else
GMVec4(gm::GMfloat v0)
	: v_(v0)
{
}

GMVec4(gm::GMfloat v0, gm::GMfloat v1, gm::GMfloat v2, gm::GMfloat v3)
	: v_(v0, v1, v2, v3)
{
}
#endif
GMMATH_END_STRUCT

GMMATH_BEGIN_STRUCT(GMMat4, glm::mat4, DirectX::XMMATRIX)
void loadFloat16(GMFloat16& f16) const
{
#if GM_USE_DX11
	for (gm::GMint i = 0; i < 4; ++i)
	{
		GMFloat4& f4 = f16.v_[i];
		XMStoreFloat4(&(f4.v_), v_.r[i]);
	}
#else
	GM_ASSERT(false);
#endif
}
GMMATH_END_STRUCT

GMMATH_BEGIN_STRUCT(GMQuat, glm::quat, DirectX::XMVECTOR)
GMMATH_END_STRUCT

//////////////////////////////////////////////////////////////////////////
inline GMMat4 __getIdentityMat4();
inline GMQuat __getIdentityQuat();

template <typename T>
T zero()
{
	return T(0);
}

template <typename T>
T Identity();

template <>
inline GMMat4 Identity()
{
	return __getIdentityMat4();
}

template <>
inline GMQuat Identity()
{
	return __getIdentityQuat();
}

template <typename T>
T Zero();

inline GMVec2 operator-(const GMVec2& V);

inline GMVec3 operator-(const GMVec3& V);

inline GMVec4 operator-(const GMVec4& V);

inline GMVec2 operator+(const GMVec2& V1, const GMVec2& V2);

inline GMVec2 operator-(const GMVec2& V1, const GMVec2& V2);

inline GMVec3 operator+(const GMVec3& V1, const GMVec3& V2);

inline GMVec3 operator-(const GMVec3& V1, const GMVec3& V2);

inline GMVec4 operator+(const GMVec4& V1, const GMVec4& V2);

inline GMVec4 operator-(const GMVec4& V1, const GMVec4& V2);

//! 计算两个矩阵相乘的结果。
/*!
	表示先进行M1矩阵变换，然后进行M2矩阵变换。
	\param M1 先进行变换的矩阵。
	\param M2 后进行变换的矩阵。
	\return 变换后的矩阵。
*/
inline GMMat4 operator*(const GMMat4& M1, const GMMat4& M2);

inline GMVec4 operator*(const GMVec4& V, const GMMat4& M);

inline GMMat4 QuatToMatrix(const GMQuat& quat);

inline GMMat4 LookAt(const GMVec3& position, const GMVec3& center, const GMVec3& up);

inline gm::GMfloat Dot(const GMVec2& V1, const GMVec2& V2);

inline gm::GMfloat Dot(const GMVec3& V1, const GMVec3& V2);

inline gm::GMfloat Dot(const GMVec4& V1, const GMVec4& V2);

inline GMVec3 Normalize(const GMVec3& V);

inline GMVec3 FastNormalize(const GMVec3& V);

inline GMVec3 MakeVector3(const gm::GMfloat* f);

inline GMMat4 Translate(const GMVec3& V);

inline GMMat4 Translate(const GMVec4& V);

inline GMMat4 Ortho(gm::GMfloat left, gm::GMfloat right, gm::GMfloat bottom, gm::GMfloat top, gm::GMfloat zNear, gm::GMfloat zFar);

inline void GetTranslationFromMatrix(const GMMat4& M, OUT GMFloat4& F);

inline void GetScalingFromMatrix(const GMMat4& M, OUT GMFloat4& F);

#include "linearmath.inl"

#endif
