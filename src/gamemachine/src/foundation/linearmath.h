#ifndef __GM_LINEARMATH_H__
#define __GM_LINEARMATH_H__

// GLM
#if !USE_SIMD
	#define GLM_FORCE_PURE
#endif

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

// GM
#include <defines.h>
#include <math.h>

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

namespace glm
{
	inline vec4 combine_vec4(const vec3& v3, const vec4& v4)
	{
		return glm::vec4(v3, v4[3]);
	}

	// identity
	template <typename T>
	inline T& identity();

	template <>
	inline mat4& identity()
	{
		static mat4 m(1.f);
		return m;
	}

	template <>
	inline mat3& identity()
	{
		static mat3 m(1.f);
		return m;
	}

	template <>
	inline mat2& identity()
	{
		static mat2 m(1.f);
		return m;
	}

	template <>
	inline quat& identity()
	{
		static quat q(1, 0, 0, 0);
		return q;
	}

	//transform
	inline mat4 scale(const vec3& v)
	{
		return scale(identity<mat4>(), v);
	}

	inline mat4 scale(gm::GMfloat x, gm::GMfloat y, gm::GMfloat z)
	{
		return scale(identity<mat4>(), vec3(x, y, z));
	}

	inline mat4 translate(const vec3& v)
	{
		return translate(identity<mat4>(), v);
	}

	inline void copyToArray(const mat4& mat, gm::GMfloat* value)
	{
		const auto v = value_ptr(mat);
		memcpy_s(value, sizeof(gm::GMfloat) * 16, v, sizeof(gm::GMfloat) * 16);
	}

	inline void copyToArray(const vec4& vec, gm::GMfloat* value)
	{
		const auto v = value_ptr(vec);
		memcpy_s(value, sizeof(gm::GMfloat) * 4, v, sizeof(gm::GMfloat) * 4);
	}

	inline void copyToArray(const vec3& vec, gm::GMfloat* value)
	{
		const auto v = value_ptr(vec);
		memcpy_s(value, sizeof(gm::GMfloat) * 3, v, sizeof(gm::GMfloat) * 3);
	}

	inline bool fuzzyCompare(gm::GMfloat p1, gm::GMfloat p2)
	{
		return (gm::gmFabs(p1 - p2) <= 0.01f);
	}

	inline vec3 toInhomogeneous(const vec4& v4)
	{
		return vec3(v4.x / v4.x, v4.y / v4.w, v4.z / v4.w);
	}

	inline vec4 toHomogeneous(const vec3& v3)
	{
		return vec4(v3.x, v3.y, v3.z, 1);
	}

	inline vec3 make_vec3(const gm::GMfloat(&v)[3])
	{
		return make_vec3(static_cast<const gm::GMfloat*>(v));
	}

	template <typename T>
	inline gm::GMfloat lengthSquare(const T& left)
	{
		return dot(left, left);
	}

	inline void getScalingFromMatrix(const mat4& mat, gm::GMfloat* out)
	{
		out[0] = mat[0][0];
		out[1] = mat[1][1];
		out[2] = mat[2][2];
	}

	inline void getTranslationFromMatrix(const mat4& mat, gm::GMfloat* out)
	{
		out[0] = mat[3][0];
		out[1] = mat[3][1];
		out[2] = mat[3][2];
	}

	inline gm::GMfloat lerp(const gm::GMfloat& start, const gm::GMfloat& end, gm::GMfloat percentage)
	{
		return percentage * (end - start) + start;
	}

	inline glm::vec3 safeNormalize(const glm::vec3& vec, const glm::vec3& n = glm::vec3(1, 0, 0))
	{
		gm::GMfloat l2 = glm::length2(vec);
		if (l2 >= FLT_EPSILON*FLT_EPSILON)
		{
			return vec / gm::gmSqrt(l2);
		}
		else
		{
			return n;
		}
	}
}
#endif
