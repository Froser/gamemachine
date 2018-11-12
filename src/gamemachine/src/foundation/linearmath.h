#ifndef __GM_LINEARMATH_H__
#define __GM_LINEARMATH_H__
#include <defines.h>
#include <math.h>
#include <gmdxincludes.h>

#if !GM_USE_DX11
// 如果使用DirectX，则不使用glm
#define GLM_FORCE_INLINE
#define GLM_ENABLE_EXPERIMENTAL
#include <gmmemory.h>
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

// namespace gmmath {

constexpr gm::GMfloat PI = 3.141592653f;

#ifndef FLT_EPSILON
#	define FLT_EPSILON 1.192092896e-07F
#endif

#define GM_SIMD_EPSILON FLT_EPSILON

// 数学函数
inline gm::GMfloat Fabs(gm::GMfloat x) { return fabsf(x); }
inline gm::GMfloat Cos(gm::GMfloat x) { return cosf(x); }
inline gm::GMfloat Sin(gm::GMfloat x) { return sinf(x); }
inline gm::GMfloat Tan(gm::GMfloat x) { return tanf(x); }
inline gm::GMfloat Sqrt(gm::GMfloat x) { return sqrtf(x); }
inline gm::GMfloat Acos(gm::GMfloat x)
{
	if (x < gm::GMfloat(-1))
		x = gm::GMfloat(-1);
	if (x > gm::GMfloat(1))
		x = gm::GMfloat(1);
	return acosf(x);
}
inline gm::GMfloat Asin(gm::GMfloat x)
{
	if (x < gm::GMfloat(-1))
		x = gm::GMfloat(-1);
	if (x > gm::GMfloat(1))
		x = gm::GMfloat(1);
	return asinf(x);
}
inline gm::GMfloat Atan(gm::GMfloat x) { return atanf(x); }
inline gm::GMfloat Atan2(gm::GMfloat x, gm::GMfloat y) { return atan2f(x, y); }
inline gm::GMfloat Exp(gm::GMfloat x) { return expf(x); }
inline gm::GMfloat Log(gm::GMfloat x) { return logf(x); }
inline gm::GMfloat Pow(gm::GMfloat x, gm::GMfloat y) { return powf(x, y); }
inline gm::GMfloat Fmod(gm::GMfloat x, gm::GMfloat y) { return fmodf(x, y); }
inline gm::GMfloat Floor(gm::GMfloat x) { return floor(x); }
inline gm::GMfloat Ceil(gm::GMfloat x) { return ceil(x); }
inline gm::GMfloat Min(gm::GMfloat x, gm::GMfloat y) { return x < y ? x : y; }
inline gm::GMfloat Max(gm::GMfloat x, gm::GMfloat y) { return x > y ? x : y; }
inline gm::GMint32 Round(gm::GMfloat d)
{
	return d >= 0.0 ? static_cast<gm::GMint32>(d + 0.5) : static_cast<gm::GMint32>(d - static_cast<gm::GMint32>(d - 1) + 0.5) + static_cast<gm::GMint32>(d - 1);
}

template <typename T>
inline T Clamp(T v, T minv, T maxv)
{
	return (Min(Max(v, minv), maxv));
}

template<typename genType>
constexpr genType Radians(genType degrees)
{
	GM_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559, "'radians' only accept floating-point input");
	return degrees * static_cast<genType>(0.01745329251994329576923690768489);
}

inline bool FuzzyCompare(gm::GMfloat p1, gm::GMfloat p2, gm::GMfloat qualifier = 0)
{
	if (qualifier)
		return (Fabs(p1 - p2) <= qualifier);
	
	return (Fabs(p1 - p2) <= 0.00001f * Min(Fabs(p1), Fabs(p2)));
}

#if GM_USE_DX11
#define GMMATH_BEGIN_STRUCT(className, glStruct, dxStruct)	\
	struct className							\
	{											\
		typedef dxStruct DataType;				\
		dxStruct v_;							\
	public:										\
		className() = default;					\
		className(const className& rhs) { v_ = rhs.v_; }

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

#define GMMATH_SET_GET_(basis, n)				\
		void set##basis(gm::GMfloat S)			\
		{										\
			v_ =								\
			DirectX::XMVectorSet##basis(v_, S);	\
		}										\
		gm::GMfloat get##basis() const			\
		{										\
			return DirectX::XMVectorGet##basis(v_);\
		}

#else
#define GMMATH_BEGIN_STRUCT(className, glStruct, dxStruct)	\
	GM_ALIGNED_STRUCT(className)				\
	{											\
		typedef glStruct DataType;				\
		glStruct v_;							\
												\
	public:										\
		className() = default;					\
		className(const className& rhs) { v_ = rhs.v_; }

#define GMMATH_LOAD_FLOAT4						\
		void loadFloat4(GMFloat4& f4) const		\
		{										\
			for (gm::GMint32 i = 0;				\
				i < length(); ++i)				\
			{									\
				f4.v_[i] = v_[i];				\
			}									\
		}

#define GMMATH_SET_FLOAT4(makeFunc)				\
		void setFloat4(const GMFloat4& f4)		\
		{										\
			v_ = makeFunc(&f4[0]);				\
		}

#define GMMATH_SET_GET_(basis, n)				\
		void set##basis(gm::GMfloat S)			\
		{										\
			v_[n] = S;							\
		}										\
		gm::GMfloat get##basis() const			\
		{										\
			return v_[n];						\
		}
#endif
#define GMMATH_END_STRUCT };

#define GMMATH_LEN(len) constexpr static gm::GMint32 length() { return (len); }

struct GMFloat4
{
	GMFloat4() = default;

	GMFloat4(gm::GMfloat x, gm::GMfloat y, gm::GMfloat z, gm::GMfloat w)
#if GM_USE_DX11
		: v_(x, y, z, w)
#endif
	{
#if !GM_USE_DX11
		v_[0] = x;
		v_[1] = y;
		v_[2] = z;
		v_[3] = w;
#endif
	}

#if GM_USE_DX11
	enum XYZW
	{
		X = 0,
		Y,
		Z,
		W,
	};

	DirectX::XMFLOAT4 v_;
	gm::GMfloat& operator[](gm::GMint32 i)
	{
		GM_ASSERT(i < 4);
		return *(&(v_.x) + i);
	}

	const gm::GMfloat& operator[](gm::GMint32 i) const
	{
		GM_ASSERT(i < 4);
		return *(&(v_.x) + i);
	}
#else
	gm::GMfloat v_[4];

	gm::GMfloat& operator[](gm::GMint32 i)
	{
		GM_ASSERT(i < 4);
		GM_ASSERT(v_);
		return v_[i];
	}

	const gm::GMfloat& operator[](gm::GMint32 i) const
	{
		GM_ASSERT(i < 4);
		GM_ASSERT(v_);
		return v_[i];
	}
#endif
};

struct GMFloat16
{
	GMFloat4 v_[4];
	GMFloat4& operator[](gm::GMint32 i)
	{
		return v_[i];
	}
};

struct GMVec4;

GMMATH_BEGIN_STRUCT(GMVec2, glm::vec2, DirectX::XMVECTOR)
GMMATH_LEN(2)
GMMATH_LOAD_FLOAT4
GMMATH_SET_FLOAT4(glm::make_vec2)
GMMATH_SET_GET_(X, 0)
GMMATH_SET_GET_(Y, 1)
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
GMMATH_LEN(3)
GMMATH_SET_FLOAT4(glm::make_vec3)
GMMATH_LOAD_FLOAT4
GMMATH_SET_GET_(X, 0)
GMMATH_SET_GET_(Y, 1)
GMMATH_SET_GET_(Z, 2)
GMMATH_SET_GET_(W, 3)
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
inline GMVec3(const GMVec4& V);
GMMATH_END_STRUCT

GMMATH_BEGIN_STRUCT(GMVec4, glm::vec4, DirectX::XMVECTOR)
GMMATH_LEN(4)
GMMATH_SET_FLOAT4(glm::make_vec4)
GMMATH_LOAD_FLOAT4
GMMATH_SET_GET_(X, 0)
GMMATH_SET_GET_(Y, 1)
GMMATH_SET_GET_(Z, 2)
GMMATH_SET_GET_(W, 3)
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

GMVec4(const GMVec3& V, const gm::GMfloat W)
{
#if GM_USE_DX11
	v_ = V.v_;
	this->setW(W);
#else
	v_ = glm::vec4(V.v_[0], V.v_[1], V.v_[2], W);
#endif
}
GMMATH_END_STRUCT

GMMATH_BEGIN_STRUCT(GMMat4, glm::mat4, DirectX::XMMATRIX)
GMMATH_LEN(4)
void loadFloat16(GMFloat16& f16) const
{
#if GM_USE_DX11
	for (gm::GMint32 i = 0; i < 4; ++i)
	{
		GMFloat4& f4 = f16.v_[i];
		DirectX::XMStoreFloat4(&(f4.v_), v_.r[i]);
	}
#else
	for (gm::GMint32 i = 0; i < 4; ++i)
	{
		GMFloat4& f4 = f16.v_[i];
		(*this)[i].loadFloat4(f4);
	}
#endif
}

void setFloat16(const GMFloat16& f16)
{
#if GM_USE_DX11
	for (gm::GMint32 i = 0; i < 4; ++i)
	{
		const GMFloat4& f4 = f16.v_[i];
		v_.r[i] = DirectX::XMLoadFloat4(&(f4.v_));
	}
#else

	for (gm::GMint32 i = 0; i < 4; ++i)
	{
		const GMFloat4& f4 = f16.v_[i];
		v_[i][0] = f4[0];
		v_[i][1] = f4[1];
		v_[i][2] = f4[2];
		v_[i][3] = f4[3];
	}
#endif
}

GMVec4 operator[](gm::GMint32 i) const
{
	GMVec4 R;
#if GM_USE_DX11
	R.v_ = v_.r[i];
#else
	R.v_ = v_[i];
#endif
	return R;
}
GMMATH_END_STRUCT

GMMATH_BEGIN_STRUCT(GMQuat, glm::quat, DirectX::XMVECTOR)
GMMATH_LEN(4)
GMMATH_SET_GET_(X, 0)
GMMATH_SET_GET_(Y, 1)
GMMATH_SET_GET_(Z, 2)
GMMATH_SET_GET_(W, 3)
GMQuat(gm::GMfloat x, gm::GMfloat y, gm::GMfloat z, gm::GMfloat w)
{
#if GM_USE_DX11
	v_ = DirectX::XMVectorSet(x, y, z, w);
#else
	v_ = glm::quat(w, x, y, z);
#endif
}

GMQuat(const GMVec3& U, const GMVec3& V);
GMMATH_END_STRUCT

//////////////////////////////////////////////////////////////////////////
inline GMMat4 __getIdentityMat4();
inline GMQuat __getIdentityQuat();

template <typename T>
inline T Identity();

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
inline T Zero();

inline GMVec2 operator-(const GMVec2& V);

inline GMVec3 operator-(const GMVec3& V);

inline GMVec4 operator-(const GMVec4& V);

inline GMVec2 operator+(const GMVec2& V1, const GMVec2& V2);

inline GMVec2 operator-(const GMVec2& V1, const GMVec2& V2);

inline GMVec3 operator+(const GMVec3& V1, const GMVec3& V2);

inline GMVec3& operator+=(GMVec3& V1, const GMVec3& V2);

inline GMVec3 operator-(const GMVec3& V1, const GMVec3& V2);

inline GMVec3& operator-=(GMVec3& V1, const GMVec3& V2);

inline GMVec3 operator*(const GMVec3& V1, gm::GMfloat S);

inline GMVec3 operator/(const GMVec3& V1, gm::GMfloat S);

inline GMVec3& operator*=(GMVec3& V1, gm::GMfloat S);

inline GMVec3& operator*=(GMVec3& V1, const GMVec3& V2);

inline GMVec3& operator/=(GMVec3& V1, gm::GMfloat S);

inline GMVec4 operator/(const GMVec4& V1, gm::GMfloat S);

inline GMVec4 operator+(const GMVec4& V1, const GMVec4& V2);

inline GMVec4 operator-(const GMVec4& V1, const GMVec4& V2);

inline GMVec4 operator*(const GMVec4& V1, gm::GMfloat S);

inline GMMat4 operator+(const GMMat4& M1, const GMMat4& M2);

inline GMMat4 operator+=(GMMat4& M1, const GMMat4& M2);

inline bool operator==(const GMVec2& V1, const GMVec2& V2);

inline bool operator==(const GMVec3& V1, const GMVec3& V2);

inline bool operator==(const GMVec4& V1, const GMVec4& V2);

inline bool operator==(const GMQuat& Q1, const GMQuat& Q2);

inline bool operator!=(const GMVec2& V1, const GMVec2& V2);

inline bool operator!=(const GMVec3& V1, const GMVec3& V2);

inline bool operator!=(const GMVec4& V1, const GMVec4& V2);

inline bool operator!=(const GMQuat& Q1, const GMQuat& Q2);

//! 计算两个矩阵相乘的结果。
/*!
	表示先进行M1矩阵变换，然后进行M2矩阵变换。
	\param M1 先进行变换的矩阵。
	\param M2 后进行变换的矩阵。
	\return 变换后的矩阵。
*/
inline GMMat4 operator*(const GMMat4& M1, const GMMat4& M2);

inline GMVec4 operator*(const GMVec4& V, const GMMat4& M);

inline GMVec3 operator*(const GMVec3& V1, const GMVec3& V2);

inline GMVec4 operator*(const GMVec4& V1, const GMVec4& V2);

inline GMVec4 operator*(const GMVec4& V, const GMQuat& Q);

inline GMVec3 operator*(const GMVec3& V, const GMQuat& Q);

//! 计算四元数相乘的结果
/*!
	返回Q*V，表示先进行V变换，再进行Q变换。
	\param V 先进行的变换。
	\param Q 后进行的变换。
	\return 变换后的四元数。
*/
inline GMQuat operator*(const GMQuat& V, const GMQuat& Q);

inline GMMat4 operator*(gm::GMfloat V, const GMMat4& M);

inline GMMat4 QuatToMatrix(const GMQuat& quat);

inline GMMat4 LookAt(const GMVec3& position, const GMVec3& center, const GMVec3& up);

inline gm::GMfloat Dot(const GMVec2& V1, const GMVec2& V2);

inline gm::GMfloat Dot(const GMVec3& V1, const GMVec3& V2);

inline gm::GMfloat Dot(const GMVec4& V1, const GMVec4& V2);

inline GMVec3 Normalize(const GMVec3& V);

inline GMVec4 Normalize(const GMVec4& V);

inline GMVec4 PlaneNormalize(const GMVec4& V);

inline GMQuat Normalize(const GMQuat& Q);

inline GMVec3 FastNormalize(const GMVec3& V);

inline GMVec3 MakeVector3(const gm::GMfloat* f);

inline GMVec3 MakeVector3(const GMVec4& V);

inline GMVec4 CombineVector4(const GMVec3& V1, const GMVec4& V2);

inline GMMat4 Translate(const GMVec3& V);

inline GMMat4 Translate(const GMVec4& V);

inline GMMat4 Scale(const GMVec3& V);

inline GMQuat Rotate(gm::GMfloat Angle, const GMVec3& Axis);

inline GMQuat Rotate(const GMQuat& Start, gm::GMfloat Angle, const GMVec3& Axis);

inline GMMat4 Ortho(gm::GMfloat left, gm::GMfloat right, gm::GMfloat bottom, gm::GMfloat top, gm::GMfloat zNear, gm::GMfloat zFar);

inline void GetTranslationFromMatrix(const GMMat4& M, GMFloat4& F);

inline void GetScalingFromMatrix(const GMMat4& M, GMFloat4& F);

inline void GetTranslationAndScalingFromMatrix(const GMMat4& M, GMFloat4& T, GMFloat4& S);

inline gm::GMfloat Length(const GMVec3& V);

inline gm::GMfloat Length(const GMVec4& V);

inline gm::GMfloat LengthSq(const GMVec3& V);

inline GMVec3 Cross(const GMVec3& V1, const GMVec3& V2);

inline GMMat4 Perspective(gm::GMfloat fovy, gm::GMfloat aspect, gm::GMfloat n, gm::GMfloat f);

inline GMMat4 Transpose(const GMMat4& M);

inline GMMat4 Inverse(const GMMat4& M);

inline GMMat4 InverseTranspose(const GMMat4& M);

inline GMQuat Lerp(const GMQuat& Q1, const GMQuat& Q2, gm::GMfloat T);

//! 获取某个向量到另外一个向量的最小旋转四元数。
/*!
  如果源向量和目标向量几乎相反，那么会生成一个沿着fallbackAxis旋转180度的四元数。<BR>
  如果指定的fallbackAxis为零向量，则会生成一个。
*/
inline GMQuat RotationTo(const GMVec3& src, const GMVec3& dest, const GMVec3& fallbackAxis);

inline GMVec3 Unproject(
	const GMVec3& V,
	gm::GMfloat ViewportX,
	gm::GMfloat ViewportY,
	gm::GMfloat ViewportWidth,
	gm::GMfloat ViewportHeight,
	const GMMat4& Projection,
	const GMMat4& View,
	const GMMat4& World
);

inline void GetFrustumPlanesFromProjectionViewModelMatrix(
	gm::GMfloat FarZ,
	gm::GMfloat NearZ,
	const GMMat4& ProjectionViewModelMatrix,
	GMVec4& FarPlane,
	GMVec4& NearPlane,
	GMVec4& RightPlane,
	GMVec4& LeftPlane,
	GMVec4& TopPlane,
	GMVec4& BottomPlane
);

template <typename T>
inline T Lerp(const T& S, const T& E, gm::GMfloat P);

inline GMVec3 Inhomogeneous(const GMVec4& V);

inline GMMat4 Inhomogeneous(const GMMat4& M);

inline void CopyToArray(const GMVec3& V, gm::GMfloat* array);

inline void CopyToArray(const GMVec4& V, gm::GMfloat* array);

template <typename T>
inline gm::GMfloat* ValuePointer(const T& data);

inline GMVec2 Clamp(const GMVec2& v, gm::GMfloat minv, gm::GMfloat maxv)
{
	GMVec2 r;
	r.setX(Clamp(v.getX(), minv, maxv));
	r.setY(Clamp(v.getY(), minv, maxv));
	return r;
}

inline GMVec3 Clamp(const GMVec3& v, gm::GMfloat minv, gm::GMfloat maxv)
{
	GMVec3 r;
	r.setX(Clamp(v.getX(), minv, maxv));
	r.setY(Clamp(v.getY(), minv, maxv));
	r.setZ(Clamp(v.getZ(), minv, maxv));
	return r;
}

inline GMVec4 Clamp(const GMVec4& v, gm::GMfloat minv, gm::GMfloat maxv)
{
	GMVec4 r;
	r.setX(Clamp(v.getX(), minv, maxv));
	r.setY(Clamp(v.getY(), minv, maxv));
	r.setZ(Clamp(v.getZ(), minv, maxv));
	r.setW(Clamp(v.getW(), minv, maxv));
	return r;
}

#include "linearmath.inl"

// } // End of namespace gmmath

#endif
