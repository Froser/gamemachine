#ifndef __LINEARMATH_H__
#define __LINEARMATH_H__
#include "common.h"
BEGIN_NS

#if USE_SIMD
#	include <xmmintrin.h>
#	define gm_CastfTo128i(a) (_mm_castps_si128(a))
#	define gm_CastfTo128d(a) (_mm_castps_pd(a))
#	define gm_CastiTo128f(a) (_mm_castsi128_ps(a))
#	define gm_CastdTo128f(a) (_mm_castpd_ps(a))
#	define gm_CastdTo128i(a) (_mm_castpd_si128(a))
#	define gm_Assign128(r0, r1, r2, r3) _mm_setr_ps(r0, r1, r2, r3)
#	define gm_mm_madd_ps(a, b, c) _mm_add_ps(_mm_mul_ps((a), (b)), (c))
#	define gm_shuffle_param(x, y, z, w)  ((x) | ((y) << 2) | ((z) << 4) | ((w) << 6))
#	define gm_pshufd_ps( _a, _mask ) _mm_shuffle_ps((_a), (_a), (_mask) )
#	define gm_splat3_ps( _a, _i ) gm_pshufd_ps((_a), gm_shuffle_param(_i,_i,_i, 3) )
#	define gm_splat_ps( _a, _i )  gm_pshufd_ps((_a), gm_shuffle_param(_i,_i,_i,_i) )
#	define gm_zeroMask (_mm_set_ps(-0.0f, -0.0f, -0.0f, -0.0f))
#	define gm_FFF0Mask (_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF))
#	define gm_v3AbsiMask (_mm_set_epi32(0x00000000, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF))
#	define gm_vAbsMask (_mm_set_epi32( 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF))
#	define gm_vFFF0Mask (_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF))
#	define gm_v3AbsfMask gm_CastiTo128f(gm_v3AbsiMask)
#	define gm_vFFF0fMask gm_CastiTo128f(gm_vFFF0Mask)
#	define gm_vxyzMaskf gm_vFFF0fMask
#	define gm_vAbsfMask gm_CastiTo128f(gm_vAbsMask)
#	define gm_vQInv (_mm_set_ps(+0.0f, -0.0f, -0.0f, -0.0f))
#	define gm_vPPPM (_mm_set_ps(-0.0f, +0.0f, +0.0f, +0.0f))
#	define gm_vOnes (_mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f))
#endif

#include <math.h>

// 数学函数
inline GMfloat gmFabs(GMfloat x) { return fabsf(x); }
inline GMfloat gmCos(GMfloat x) { return cosf(x); }
inline GMfloat gmSin(GMfloat x) { return sinf(x); }
inline GMfloat gmTan(GMfloat x) { return tanf(x); }
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

#define VEC3(v4) linear_math::Vector3(v4[0], v4[1], v4[2])
#define VEC4(v3, v4) linear_math::Vector4(v3, v4[3])

namespace linear_math
{
	template <typename T>
	struct _no_sse
	{
		typedef T ReturnType;

		static ReturnType negate(const ReturnType& left)
		{
			ReturnType result;
			for (GMint n = 0; n < ReturnType::dimension; n++)
				result[n] = -left[n];
			return result;
		}

		static ReturnType add(const ReturnType& left, const ReturnType& right)
		{
			ReturnType result;
			for (GMint n = 0; n < ReturnType::dimension; n++)
				result[n] = left[n] + right[n];
			return result;
		}

		static ReturnType sub(const ReturnType& left, const ReturnType& right)
		{
			ReturnType result;
			for (GMint n = 0; n < ReturnType::dimension; n++)
				result[n] = left[n] - right[n];
			return result;
		}

		static ReturnType mul(const ReturnType& left, GMfloat right)
		{
			ReturnType result;
			for (GMint n = 0; n < ReturnType::dimension; n++)
				result[n] = left[n] * right;
			return result;
		}

		static ReturnType div(const ReturnType& left, GMfloat right)
		{
			ReturnType result;
			for (GMint n = 0; n < ReturnType::dimension; n++)
				result[n] = left[n] / right;
			return result;
		}

		static ReturnType div(GMfloat left, const ReturnType& right)
		{
			ReturnType result;
			for (GMint n = 0; n < ReturnType::dimension; n++)
				result[n] = left / right[n];
			return result;
		}

		static ReturnType& self_add(ReturnType& left, GMfloat right)
		{
			for (GMint n = 0; n < ReturnType::dimension; n++)
				left[n] += right;
			return left;
		}

		static ReturnType& self_add(ReturnType& left, const ReturnType& right)
		{
			for (GMint n = 0; n < ReturnType::dimension; n++)
				left[n] += right[n];
			return left;
		}

		static ReturnType& self_sub(ReturnType& left, GMfloat right)
		{
			for (GMint n = 0; n < ReturnType::dimension; n++)
				left[n] -= right;
			return left;
		}

		static ReturnType& self_sub(ReturnType& left, const ReturnType& right)
		{
			for (GMint n = 0; n < ReturnType::dimension; n++)
				left[n] -= right[n];
			return left;
		}

		static ReturnType& self_mul(ReturnType& left, GMfloat right)
		{
			for (GMint n = 0; n < ReturnType::dimension; n++)
				left[n] *= right;
			return left;
		}

		static ReturnType& self_div(ReturnType& left, GMfloat right)
		{
			for (GMint n = 0; n < ReturnType::dimension; n++)
				left[n] /= right;
			return left;
		}
	};

#if USE_SIMD
	template <typename T>
	struct _sse
	{
		typedef T ReturnType;

		static ReturnType negate(const ReturnType& left)
		{
			ReturnType result;
			//TODO 可以优化
			for (GMint n = 0; n < ReturnType::dimension; n++)
				result[n] = -left[n];
			return result;
		}

		static ReturnType add(const ReturnType& left, const ReturnType& right)
		{
			return ReturnType(_mm_add_ps(left.get128(), right.get128()));
		}

		static ReturnType sub(const ReturnType& left, const ReturnType& right)
		{
			return ReturnType(_mm_sub_ps(left.get128(), right.get128()));
		}

		static ReturnType mul(const ReturnType& left, GMfloat right)
		{
			return ReturnType(_mm_mul_ps(left.get128(), _mm_set_ps(right, right, right, right)));
		}

		static ReturnType div(const ReturnType& left, GMfloat right)
		{
			return ReturnType(_mm_div_ps(left.get128(), _mm_set_ps(right, right, right, right)));
		}

		static ReturnType div(GMfloat left, const ReturnType& right)
		{
			return ReturnType(_mm_div_ps(_mm_set_ps(left, left, left, left) , right.get128()));
		}

		static ReturnType& self_add(ReturnType& left, GMfloat right)
		{
			left.set128(_mm_add_ps(left.get128(), _mm_set_ps(right, right, right, right)));
			return left;
		}

		static ReturnType& self_add(ReturnType& left, const ReturnType& right)
		{
			left.set128(_mm_add_ps(left.get128(), right.get128()));
			return left;
		}

		static ReturnType& self_sub(ReturnType& left, GMfloat right)
		{
			left.set128(_mm_sub_ps(left.get128(), _mm_set_ps(right, right, right, right)));
			return left;
		}

		static ReturnType& self_sub(ReturnType& left, const ReturnType& right)
		{
			left.set128(_mm_sub_ps(left.get128(), right.get128()));
			return left;
		}

		static ReturnType& self_mul(ReturnType& left, GMfloat right)
		{
			left.set128(_mm_mul_ps(left.get128(), _mm_set_ps(right, right, right, right)));
			return left;
		}

		static ReturnType& self_div(ReturnType& left, GMfloat right)
		{
			left.set128(_mm_div_ps(left.get128(), _mm_set_ps(right, right, right, right)));
			return left;
		}
	};
#endif

	template <typename T>
	struct _op_selector
	{
#if USE_SIMD
		typedef _sse<T> Op;
#else
		typedef _no_sse<T> Op;
#endif
	};

	// Operators
	template <typename T>
	static inline T operator - (const T& left)
	{
		return _op_selector<T>::Op::negate(left);
	}

	template <typename T>
	static inline T operator + (const T& left, const T& right)
	{
		return _op_selector<T>::Op::add(left, right);
	}

	template <typename T>
	static inline T operator - (const T& left, const T& right)
	{
		return _op_selector<T>::Op::sub(left, right);
	}

	template <typename T>
	static inline T operator * (const T& left, GMfloat right)
	{
		return _op_selector<T>::Op::mul(left, right);
	}

	template <typename T>
	static inline T operator * (GMfloat a, const T& b)
	{
		return _op_selector<T>::Op::mul(b, a);
	}

	template <typename T>
	static inline T operator / (const T& left, GMfloat right)
	{
		return _op_selector<T>::Op::div(left, right);
	}

	template <typename T>
	static inline T operator / (GMfloat left, const T& right)
	{
		return _op_selector<T>::Op::div(left, right);
	}

	template <typename T>
	static inline T& operator += (T& left, const T& right)
	{
		return _op_selector<T>::Op::self_add(left, right);
	}

	template <typename T>
	static inline T& operator += (T& left, GMfloat right)
	{
		return _op_selector<T>::Op::self_add(left, right);
	}

	template <typename T>
	static inline T& operator -= (T& left, GMfloat right)
	{
		return _op_selector<T>::Op::self_sub(left, right);
	}

	template <typename T>
	static inline T& operator *= (T& left, GMfloat right)
	{
		return _op_selector<T>::Op::self_mul(left, right);
	}

	template <typename T>
	static inline T& operator /= (T& left, GMfloat right)
	{
		return _op_selector<T>::Op::self_div(left, right);
	}


#if USE_SIMD
#define DEFINE_VECTOR_DATA(l)					\
	public:										\
	enum { dimension = l };						\
	__m128 get128() const { return m_128; }		\
	void set128(__m128 _128) { m_128 = _128; }	\
												\
	protected:									\
		union									\
		{										\
			GMfloat m_data[4];					\
			__m128 m_128;						\
		};
#else
#define DEFINE_VECTOR_DATA(l)					\
	public:										\
	enum { dimension = l };						\
	protected:									\
		GMfloat m_data[l];
#endif

	GM_ALIGNED_16(class) Vector2
	{
		DEFINE_VECTOR_DATA(2)

	public:
#if USE_SIMD 
		Vector2(__m128 _128) : m_128(_128) {};
#endif
		Vector2() = default;
		Vector2(GMfloat x, GMfloat y)
		{
			m_data[0] = x;
			m_data[1] = y;
		}

		Vector2(const Vector2& right)
		{
#if USE_SIMD
			m_128 = right.m_128;
#else
			m_data[0] = right[0];
			m_data[1] = right[1];
#endif
		}

	public:
		GMfloat& operator [](GMint i);
		const GMfloat& operator [](GMint i) const;
	};

	GM_ALIGNED_16(class) Vector3
	{
		DEFINE_VECTOR_DATA(3)

	public:
#if USE_SIMD
		Vector3(__m128 _128) : m_128(_128) {};
#endif
		Vector3()
		{
#if USE_SIMD
			m_data[3] = 0;
#endif
		}

		Vector3(GMfloat x, GMfloat y, GMfloat z)
		{
			m_data[0] = x;
			m_data[1] = y;
			m_data[2] = z;
#if USE_SIMD
			m_data[3] = 0;
#endif
		}

		Vector3(GMfloat x)
		{
			m_data[0] = x;
			m_data[1] = x;
			m_data[2] = x;
#if USE_SIMD
			m_data[3] = 0;
#endif
		}

		Vector3(const Vector3& right)
		{
#if USE_SIMD
			m_128 = right.m_128;
#else
			m_data[0] = right[0];
			m_data[1] = right[1];
			m_data[2] = right[2];
#endif
		}

		inline GMfloat& x() { return m_data[0]; }
		inline GMfloat& y() { return m_data[1]; }
		inline GMfloat& z() { return m_data[2]; }
		inline const GMfloat& x() const { return m_data[0]; }
		inline const GMfloat& y() const { return m_data[1]; }
		inline const GMfloat& z() const { return m_data[2]; }

	public:
		GMfloat& operator [](GMint i);
		const GMfloat& operator [](GMint i) const;

	public:
		inline static Vector3 fromArray(const GMfloat array[3])
		{
			return Vector3(array[0], array[1], array[2]);
		}
	};

	GM_ALIGNED_16(class) Vector4
	{
		DEFINE_VECTOR_DATA(4)

	public:
#if USE_SIMD
		Vector4(__m128 _128) : m_128(_128) {};
#endif

		Vector4() = default;

		Vector4(GMfloat i)
		{
			m_data[0] = i;
			m_data[1] = i;
			m_data[2] = i;
			m_data[3] = i;
		}

		Vector4(GMfloat x, GMfloat y, GMfloat z, GMfloat w)
		{
			m_data[0] = x;
			m_data[1] = y;
			m_data[2] = z;
			m_data[3] = w;
		}

		Vector4(const Vector3& v, GMfloat w)
		{
			m_data[0] = v[0];
			m_data[1] = v[1];
			m_data[2] = v[2];
			m_data[3] = w;
		}

		Vector4(const Vector4& right)
		{
#if USE_SIMD
			m_128 = right.m_128;
#else
			m_data[0] = right[0];
			m_data[1] = right[1];
			m_data[2] = right[2];
			m_data[3] = right[3];
#endif
		}

		inline GMfloat& x() { return m_data[0]; }
		inline GMfloat& y() { return m_data[1]; }
		inline GMfloat& z() { return m_data[2]; }
		inline GMfloat& w() { return m_data[3]; }
		inline const GMfloat& x() const { return m_data[0]; }
		inline const GMfloat& y() const { return m_data[1]; }
		inline const GMfloat& z() const { return m_data[2]; }
		inline const GMfloat& w() const { return m_data[3]; }

	public:
		GMfloat& operator [](GMint i);
		const GMfloat& operator [](GMint i) const;
	};

	GM_ALIGNED_16(class) Matrix4x4
	{
	public:
		Matrix4x4() = default;
		Matrix4x4(const Vector4& r1, const Vector4& r2, const Vector4& r3, const Vector4& r4)
		{
			m_data[0] = r1;
			m_data[1] = r2;
			m_data[2] = r3;
			m_data[3] = r4;
		}

	public:
		static Matrix4x4 identity();

	public:
		inline operator Vector4*() { return &m_data[0]; }
		inline operator const Vector4*() const { return &m_data[0]; }

		Matrix4x4 operator *(const Matrix4x4& right) const;
		Matrix4x4 transpose() const;
		const GMfloat* data() const;
		void toArray(GMfloat* array) const;

	private:
		Vector4 m_data[4];
	};

	static inline Vector4 operator* (const Vector4& left, const Matrix4x4& right)
	{
#if USE_SIMD
		__m128 __result;
		__m128	__v = left.get128();
		__m128	__row0 = right[0].get128(),
			__row1 = right[1].get128(),
			__row2 = right[2].get128(),
			__row3 = right[3].get128();
		__m128	__x_mul_row0 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, gm_shuffle_param(0, 0, 0, 0)), __row0),
			__x_mul_row1 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, gm_shuffle_param(1, 1, 1, 1)), __row1),
			__x_mul_row2 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, gm_shuffle_param(2, 2, 2, 2)), __row2),
			__x_mul_row3 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, gm_shuffle_param(3, 3, 3, 3)), __row3);
		__result = _mm_add_ps(__x_mul_row0, __x_mul_row1);
		__result = _mm_add_ps(__result, __x_mul_row2);
		__result = _mm_add_ps(__result, __x_mul_row3);
		return Vector4(__result);
#else
		Vector4 result(0.f);
		result[0] = left[0] * right[0][0] + left[1] * right[1][0] + left[2] * right[2][0] + left[3] * right[3][0];
		result[1] = left[0] * right[0][1] + left[1] * right[1][1] + left[2] * right[2][1] + left[3] * right[3][1];
		result[2] = left[0] * right[0][2] + left[1] * right[1][2] + left[2] * right[2][2] + left[3] * right[3][2];
		result[3] = left[0] * right[0][3] + left[1] * right[1][3] + left[2] * right[2][3] + left[3] * right[3][3];
		return result;
#endif
	}

// AlignedVector functions
	static inline GMfloat fastInvSqrt(GMfloat x)
	{
		GMfloat xhalf = 0.5f*x;
		GMint i = *(GMint*)&x;
		i = 0x5f375a86 - (i >> 1);
		x = *(GMfloat*)&i;
		x = x*(1.5f - xhalf*x*x);
		return x;
	}

	template <typename T>
	static inline GMfloat dot(const T& left, const T& right)
	{
#if USE_SIMD
		__m128 vd = _mm_mul_ps(left.get128(), right.get128());
		__m128 z = _mm_movehl_ps(vd, vd);
		__m128 y = _mm_shuffle_ps(vd, vd, 0x55);
		vd = _mm_add_ss(vd, y);
		vd = _mm_add_ss(vd, z);
		return _mm_cvtss_f32(vd);
#else
		GMfloat result = 0;
		for (GMint n = 0; n < T::dimension; n++)
		{
			result += left[n] * right[n];
		}
		return result;
#endif
	}

	static inline Vector3 cross(const Vector3& left, const Vector3& right)
	{
#if USE_SIMD
		__m128 T, V;

		T = gm_pshufd_ps(left.get128(), gm_shuffle_param(1, 2, 0, 3));	//	(Y Z X 0)
		V = gm_pshufd_ps(right.get128(), gm_shuffle_param(1, 2, 0, 3));	//	(Y Z X 0)

		V = _mm_mul_ps(V, left.get128());
		T = _mm_mul_ps(T, right.get128());
		V = _mm_sub_ps(V, T);

		V = gm_pshufd_ps(V, gm_shuffle_param(1, 2, 0, 3));
		return Vector3(V);
#else
		return Vector3(left[1] * right[2] - right[1] * left[2],
			left[2] * right[0] - right[2] * left[0],
			left[0] * right[1] - right[0] * left[1]);
#endif
	}

	static inline GMfloat length(const Vector3& left)
	{
		return sqrtf(dot(left, left));
	}

	static inline GMfloat lengthSquare(const Vector3& left)
	{
		return dot(left, left);
	}

	static inline GMfloat fast_length(const Vector3& left)
	{
		return 1.f / (GMfloat)fastInvSqrt(lengthSquare(left));
	}

	static inline Vector3 normalize(const Vector3& left)
	{
		return left * fastInvSqrt(lengthSquare(left));
	}

	static inline Vector3 precise_normalize(const Vector3& left)
	{
		return left / length(left);
	}

	static inline Matrix4x4 translate(const Vector3& t)
	{
		return Matrix4x4(Vector4(1.0f, 0.0f, 0.0f, 0.0f),
			Vector4(0.0f, 1.0f, 0.0f, 0.0f),
			Vector4(0.0f, 0.0f, 1.0f, 0.0f),
			Vector4(t[0], t[1], t[2], 1.0f));
	}

	static inline Matrix4x4 lookat(const Vector3& eye, const Vector3& center, const Vector3& up)
	{
		const Vector3 f = normalize(center - eye);
		const Vector3 upN = normalize(up);
		const Vector3 s = normalize(cross(f, upN));
		const Vector3 u = normalize(cross(s, f));
		const Matrix4x4 M = Matrix4x4(Vector4(s[0], u[0], -f[0], 0),
			Vector4(s[1], u[1], -f[1], (0)),
			Vector4(s[2], u[2], -f[2], (0)),
			Vector4(0, 0, 0, 1));

		return M * translate(-eye);
	}

	static inline Matrix4x4 scale(GMfloat x)
	{
		return Matrix4x4(Vector4(x, 0.0f, 0.0f, 0.0f),
			Vector4(0.0f, x, 0.0f, 0.0f),
			Vector4(0.0f, 0.0f, x, 0.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	static inline Matrix4x4 scale(GMfloat x, GMfloat y, GMfloat z)
	{
		return Matrix4x4(Vector4(x, 0.0f, 0.0f, 0.0f),
			Vector4(0.0f, y, 0.0f, 0.0f),
			Vector4(0.0f, 0.0f, z, 0.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	static inline Matrix4x4 scale(const Vector3& v)
	{
		return scale(v[0], v[1], v[2]);
	}

	static inline Matrix4x4 frustum(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f)
	{
		Matrix4x4 result(Matrix4x4::identity());

		if ((right == left) ||
			(top == bottom) ||
			(n == f) ||
			(n < 0.0) ||
			(f < 0.0))
			return result;

		result[0][0] = (2.0f * n) / (right - left);
		result[1][1] = (2.0f * n) / (top - bottom);

		result[2][0] = (right + left) / (right - left);
		result[2][1] = (top + bottom) / (top - bottom);
		result[2][2] = -(f + n) / (f - n);
		result[2][3] = -1.0f;

		result[3][2] = -(2.0f * f * n) / (f - n);
		result[3][3] = 0.0f;

		return result;
	}

	static inline bool fuzzyCompare(GMfloat p1, GMfloat p2)
	{
		return (fabs(p1 - p2) <= 0.01f);
	}

	static inline bool equals(const Vector3& left, const Vector3& right)
	{
		//TODO 可以优化(SIMD)
		for (GMint i = 0; i < 3; i++)
		{
			if (!fuzzyCompare(left[i], right[i]))
				return false;
		}
		return true;
	}

	static inline bool equals(const Vector4& left, const Vector4& right)
	{
#if USE_SIMD
		return left.get128().m128_f32 == right.get128().m128_f32;
#else
		for (GMint i = 0; i < 4; i++)
		{
			if (!fuzzyCompare(left[i], right[i]))
				return false;
		}
		return true;
#endif
	}

	static inline Matrix4x4 perspective(float fovy, float aspect, float n, float f)
	{
		float q = 1.0f / tan(RAD(0.5f * fovy));
		float A = q / aspect;
		float B = (n + f) / (n - f);
		float C = (2.0f * n * f) / (n - f);

		Matrix4x4 result;

		result[0] = Vector4(A, 0.0f, 0.0f, 0.0f);
		result[1] = Vector4(0.0f, q, 0.0f, 0.0f);
		result[2] = Vector4(0.0f, 0.0f, B, -1.0f);
		result[3] = Vector4(0.0f, 0.0f, C, 0.0f);

		return result;
	}

	GM_ALIGNED_16(class) Quaternion
	{
		DEFINE_VECTOR_DATA(4)

	public:
		Quaternion()
			: Quaternion(0, 0, 0, 1)
		{
		}

		Quaternion(GMfloat x, GMfloat y, GMfloat z, GMfloat w)
		{
			m_data[0] = x;
			m_data[1] = y;
			m_data[2] = z;
			m_data[3] = w;
		}

#if USE_SIMD
		Quaternion(__m128 _128) : m_128(_128) {}
#endif

		inline GMfloat& x() { return m_data[0]; }
		inline GMfloat& y() { return m_data[1]; }
		inline GMfloat& z() { return m_data[2]; }
		inline GMfloat& w() { return m_data[3]; }
		inline const GMfloat& x() const { return m_data[0]; }
		inline const GMfloat& y() const { return m_data[1]; }
		inline const GMfloat& z() const { return m_data[2]; }
		inline const GMfloat& w() const { return m_data[3]; }

		void setValue(GMfloat x, GMfloat y, GMfloat z, GMfloat w)
		{
			m_data[0] = x;
			m_data[1] = y;
			m_data[2] = z;
			m_data[3] = w;
		}

		void setRotation(const Vector3& axis, GMfloat angle)
		{
			GMfloat d = length(axis);
			ASSERT(d != GMfloat(0.0));
			GMfloat s = gmSin(angle * 0.5f) / d;
			setValue(axis.x() * s, axis.y() * s, axis.z() * s,
				gmCos(angle * 0.5f));
		}

		void setEuler(const GMfloat& yaw, const GMfloat& pitch, const GMfloat& roll)
		{
			GMfloat halfYaw = GMfloat(yaw) * GMfloat(0.5);
			GMfloat halfPitch = GMfloat(pitch) * GMfloat(0.5);
			GMfloat halfRoll = GMfloat(roll) * GMfloat(0.5);
			GMfloat cosYaw = gmCos(halfYaw);
			GMfloat sinYaw = gmSin(halfYaw);
			GMfloat cosPitch = gmCos(halfPitch);
			GMfloat sinPitch = gmSin(halfPitch);
			GMfloat cosRoll = gmCos(halfRoll);
			GMfloat sinRoll = gmSin(halfRoll);
			setValue(cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
				cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,
				sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw,
				cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw);
		}

		Quaternion inverse() const
		{
#if USE_SIMD
			return Quaternion(_mm_xor_ps(get128(), gm_vQInv));
#else	
			return Quaternion(-m_data[0], -m_data[1], -m_data[2], m_data[3]);
#endif
		}

		Quaternion& normalize()
		{
#if USE_SIMD
			__m128 vd;
			vd = _mm_mul_ps(m_128, m_128);
			__m128 t = _mm_movehl_ps(vd, vd);
			vd = _mm_add_ps(vd, t);
			t = _mm_shuffle_ps(vd, vd, 0x55);
			vd = _mm_add_ss(vd, t);

			vd = _mm_sqrt_ss(vd);
			vd = _mm_div_ss(gm_vOnes, vd);
			vd = gm_pshufd_ps(vd, 0); // splat
			m_128 = _mm_mul_ps(m_128, vd);
			return *this;
#else
			return *this /= length();
#endif
		}

		Quaternion& operator*=(const Quaternion& q)
		{
			setValue(
				m_data[3] * q.x() + m_data[0] * q.m_data[3] + m_data[1] * q.z() - m_data[2] * q.y(),
				m_data[3] * q.y() + m_data[1] * q.m_data[3] + m_data[2] * q.x() - m_data[0] * q.z(),
				m_data[3] * q.z() + m_data[2] * q.m_data[3] + m_data[0] * q.y() - m_data[1] * q.x(),
				m_data[3] * q.m_data[3] - m_data[0] * q.x() - m_data[1] * q.y() - m_data[2] * q.z());
			return *this;
		}

		Matrix4x4 toMatrix()
		{
			Matrix4x4 matrix;
			GMfloat xx = m_data[0] * m_data[0];
			GMfloat yy = m_data[1] * m_data[1];
			GMfloat zz = m_data[2] * m_data[2];
			GMfloat xy = m_data[0] * m_data[1];
			GMfloat xz = m_data[0] * m_data[2];
			GMfloat yz = m_data[1] * m_data[2];
			GMfloat wx = m_data[3] * m_data[0];
			GMfloat wy = m_data[3] * m_data[1];
			GMfloat wz = m_data[3] * m_data[2];

			matrix[0][0] = 1 - 2 * (yy + zz);
			matrix[1][0] = 2 * (xy - wz);
			matrix[2][0] = 2 * (xz + wy);
			matrix[0][1] = 2 * (xy + wz);
			matrix[1][1] = 1 - 2 * (xx + zz);
			matrix[2][1] = 2 * (yz - wx);
			matrix[0][2] = 2 * (xz - wy);
			matrix[1][2] = 2 * (yz + wx);
			matrix[2][2] = 1 - 2 * (xx + yy);
			matrix[3][0] = matrix[3][1] = matrix[3][2] = 0.0f;
			matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0f;
			matrix[3][3] = 1.0f;
			return matrix;
		}
	};

	inline Quaternion operator*(const Quaternion& q1, const Quaternion& q2)
	{
		return Quaternion(
			q1.w() * q2.x() + q1.x() * q2.w() + q1.y() * q2.z() - q1.z() * q2.y(),
			q1.w() * q2.y() + q1.y() * q2.w() + q1.z() * q2.x() - q1.x() * q2.z(),
			q1.w() * q2.z() + q1.z() * q2.w() + q1.x() * q2.y() - q1.y() * q2.x(),
			q1.w() * q2.w() - q1.x() * q2.x() - q1.y() * q2.y() - q1.z() * q2.z());
	}

	inline Quaternion operator*(const Quaternion& q, const Vector3& w)
	{
		return Quaternion(
			q.w() * w.x() + q.y() * w.z() - q.z() * w.y(),
			q.w() * w.y() + q.z() * w.x() - q.x() * w.z(),
			q.w() * w.z() + q.x() * w.y() - q.y() * w.x(),
			-q.x() * w.x() - q.y() * w.y() - q.z() * w.z());
	}

	inline Vector3 quatRotate(const Quaternion& rotation, const Vector3& v)
	{
		Quaternion q = rotation * v;
		q *= rotation.inverse();
#if USE_SIMD
		return Vector3(_mm_and_ps(q.get128(), gm_vFFF0fMask));
#else	
		return Vector3(q.x(), q.y(), q.z());
#endif
	}
}
END_NS
#endif
