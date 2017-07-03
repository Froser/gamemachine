#ifndef __LINEARMATH_H__
#define __LINEARMATH_H__
#include "common.h"
BEGIN_NS

#if USE_SIMD
#	include <xmmintrin.h>
#	define _mm_madd_ps(a, b, c) _mm_add_ps(_mm_mul_ps((a), (b)), (c))
#	define simd_pshufd_ps( _a, _mask ) _mm_shuffle_ps((_a), (_a), (_mask) )
#	define simd_shuffle_param(x, y, z, w)  ((x) | ((y) << 2) | ((z) << 4) | ((w) << 6))
#	define simd_zeroMask (_mm_set_ps(-0.0f, -0.0f, -0.0f, -0.0f))
#	define simd_FFF0Mask (_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF))
#endif

#include <math.h>

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

	public:
		GMfloat& operator [](GMint i);
		const GMfloat& operator [](GMint i) const;
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
		__m128	__x_mul_row0 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(0, 0, 0, 0)), __row0),
			__x_mul_row1 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(1, 1, 1, 1)), __row1),
			__x_mul_row2 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(2, 2, 2, 2)), __row2),
			__x_mul_row3 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(3, 3, 3, 3)), __row3);
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

		T = simd_pshufd_ps(left.get128(), simd_shuffle_param(1, 2, 0, 3));	//	(Y Z X 0)
		V = simd_pshufd_ps(right.get128(), simd_shuffle_param(1, 2, 0, 3));	//	(Y Z X 0)

		V = _mm_mul_ps(V, left.get128());
		T = _mm_mul_ps(T, right.get128());
		V = _mm_sub_ps(V, T);

		V = simd_pshufd_ps(V, simd_shuffle_param(1, 2, 0, 3));
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

	GM_ALIGNED_16(class) Quarternion
	{
		DEFINE_VECTOR_DATA(4)

	public:
		Quarternion(GMfloat x, GMfloat y, GMfloat z, GMfloat w)
		{
			m_data[0] = x;
			m_data[1] = x;
			m_data[2] = x;
			m_data[3] = x;
		}
	};
}
END_NS
#endif
