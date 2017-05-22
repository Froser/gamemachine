#ifndef __LINEARMATH_H__
#define __LINEARMATH_H__
#include "common.h"
BEGIN_NS

#define VEC3(v4) linear_math::Vector3(v4[0], v4[1], v4[2])
#define VEC4(v3, v4) linear_math::Vector4(v3, v4[3])

namespace linear_math
{
	template <GMint L>
	class GM_ALIGNED_16 VectorBase
	{
	public:
		enum { dimension = L };

	public:
		VectorBase()
		{
			static_assert(L <= 4, "dimension must be less than or equal to 4");
#if USE_SIMD
			m_vec128.m128_u64[0] = 0;
			m_vec128.m128_u64[1] = 0;
#else
			memset(m_data, 0, sizeof(m_data));
#endif
		}

#if USE_SIMD
		VectorBase(__m128 vec128)
			: m_vec128(vec128)
		{
		}

		inline __m128 m128() const
		{
			return m_vec128;
		}

		inline void set_m128(__m128 i)
		{
			m_vec128 = i;
		}
#endif

	public:
		VectorBase& operator = (const VectorBase& right)
		{
#if USE_SIMD
			m_vec128 = right.m_vec128;
#else
			for (GMint i = 0; i < L; i++)
			{
				m_data[i] = right[i];
			}
#endif
			return *this;
		}

		VectorBase operator+(const VectorBase& right) const
		{
			VectorBase result;
			for (int n = 0; n < L; n++)
				result[n] = m_data[n] + right[n];
			return result;
		}

		VectorBase operator-(const VectorBase& right) const
		{
			VectorBase result;
			for (int n = 0; n < L; n++)
				result[n] = m_data[n] - right[n];
			return result;
		}

		VectorBase operator*(GMfloat i) const
		{
			VectorBase result;
			for (int n = 0; n < L; n++)
				result[n] = m_data[n] * i;
			return result;
		}

		VectorBase operator/(GMfloat i) const
		{
			VectorBase result;
			for (int n = 0; n < L; n++)
				result[n] = m_data[n] / i;
			return result;
		}

		VectorBase& operator*=(GMfloat i)
		{
			for (int n = 0; n < L; n++)
				m_data[n] *= i;
			return *this;
		}

		VectorBase& operator/=(GMfloat i)
		{
			for (int n = 0; n < L; n++)
				m_data[n] /= i;
			return *this;
		}

		GMfloat& operator [](GMint i)
		{
			return m_data[i];
		}

		const GMfloat& operator [](GMint i) const
		{
			return m_data[i];
		}

	protected:
		inline void assign(const VectorBase& that)
		{
#if USE_SIMD
			m_vec128 = that.m_vec128;
#else
			for (GMint n = 0; n < L; n++)
				data[n] = that.data[n];
#endif
		}

	protected:
		union
		{
			GMfloat m_data[L];
#if USE_SIMD
			__m128 m_vec128;
#endif
		};
	};

	class Vector2 : public VectorBase<2>
	{
		typedef VectorBase<2> Base;

	public:
		Vector2() {}
		Vector2(GMfloat x, GMfloat y)
		{
			m_data[0] = x;
			m_data[1] = y;
		}
		Vector2(const Base& right)
		{
#if USE_SIMD
			m_vec128 = right.m128();
#else
			m_data[0] = right[0];
			m_data[1] = right[1];
#endif
		}
	};

	class GM_ALIGNED_16 Vector3 : public VectorBase<3>
	{
		typedef VectorBase<3> Base;

	public:
#if USE_SIMD
		Vector3(__m128 vec128) : Base(vec128) {}
#endif

		Vector3() : Base() {}
		Vector3(GMfloat x, GMfloat y, GMfloat z)
			: Base()
		{
			m_data[0] = x;
			m_data[1] = y;
			m_data[2] = z;
		}

		Vector3(GMfloat x)
			: Base()
		{
			m_data[0] = x;
			m_data[1] = x;
			m_data[2] = x;
		}

	public:
		Vector3 operator - () const;
		Vector3 operator + (const Vector3& right) const;
		Vector3 operator - (const Vector3& right) const;
		Vector3 operator * (GMfloat right) const;
		Vector3 operator / (GMfloat right) const;
		Vector3& operator += (const Vector3& right);
		Vector3& operator += (GMfloat right);
		Vector3& operator -= (GMfloat right);
		Vector3& operator *= (GMfloat right);
		Vector3& operator /= (GMfloat right);
	};

	class GM_ALIGNED_16 Vector4 : public VectorBase<4>
	{
		typedef VectorBase<4> Base;

	public:
#if USE_SIMD
		Vector4(__m128 vec128) : Base(vec128) {};
#endif

		Vector4() : Base() {}

		Vector4(GMfloat i)
			: Base()
		{
			m_data[0] = i;
			m_data[1] = i;
			m_data[2] = i;
			m_data[3] = i;
		}

		Vector4(GMfloat x, GMfloat y, GMfloat z, GMfloat w)
			: Base()
		{
			m_data[0] = x;
			m_data[1] = y;
			m_data[2] = z;
			m_data[3] = w;
		}

		Vector4(const Vector3& v, GMfloat w)
			: Base()
		{
			m_data[0] = v[0];
			m_data[1] = v[1];
			m_data[2] = v[2];
			m_data[3] = w;
		}

	public:
		Vector4 operator - () const;
		Vector4 operator + (const Vector4& right) const;
		Vector4 operator - (const Vector4& right) const;
		Vector4 operator * (GMfloat right) const;
		Vector4 operator / (GMfloat right) const;
		Vector4& operator *= (GMfloat right);
		Vector4& operator /= (GMfloat right);
	};

	class GM_ALIGNED_16 Matrix4x4
	{
	public:
		Matrix4x4() {}
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

// Operators
	static inline Vector3 operator* (GMfloat left, const Vector3& right)
	{
		return right * left;
	}

	static inline Vector3 operator/ (GMfloat left, const Vector3& right)
	{
#if USE_SIMD
		GM_SIMD_float _left[] = { left, left, left, 0 };
		__m128 __left = _mm_load_ps(_left);
		__m128 __result = _mm_div_ps(__left, right.m128());
		return Vector3(__result);
#else
		return typename T::Base::operator +(right);
#endif
	}

	static inline Vector4 operator* (GMfloat left, const Vector4& right)
	{
		return right * left;
	}

	static inline Vector4 operator/ (GMfloat left, const Vector4& right)
	{
#if USE_SIMD
		GM_SIMD_float _left[] = { left, left, left, left };
		__m128 __left = _mm_load_ps(_left);
		__m128 __result = _mm_div_ps(__left, right.m128());
		return Vector4(__result);
#else
		return typename T::Base::operator +(right);
#endif
	}

	static inline Vector4 operator* (const Vector4& left, const Matrix4x4& right)
	{
#if USE_SIMD
		__m128 __result;
		__m128	__v = left.m128();
		__m128	__row0 = right[0].m128(),
			__row1 = right[1].m128(),
			__row2 = right[2].m128(),
			__row3 = right[3].m128();
		__m128	__x_mul_row0 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(0, 0, 0, 0)), __row0),
			__x_mul_row1 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(1, 1, 1, 1)), __row1),
			__x_mul_row2 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(2, 2, 2, 2)), __row2),
			__x_mul_row3 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(3, 3, 3, 3)), __row3);
		__result = _mm_add_ps(__x_mul_row0, __x_mul_row1);
		__result = _mm_add_ps(__result, __x_mul_row2);
		__result = _mm_add_ps(__result, __x_mul_row3);
		return Vector4(__result);
#else
		GMint n, m;
		Vector4 result(0.f);
		for (m = 0; m < 4; m++)
		{
			for (n = 0; n < 4; n++)
			{
				result[n] += left[m] * right[n][m];
			}
		}
		return result;
#endif
	}

// Vector functions
	static inline GMfloat fastInvSqrt(GMfloat x)
	{
		GMfloat xhalf = 0.5f*x;
		int i = *(int*)&x;
		i = 0x5f375a86 - (i >> 1);
		x = *(GMfloat*)&i;
		x = x*(1.5f - xhalf*x*x);
		return x;
	}

	template <typename T>
	static inline GMfloat dot(const T& left, const T& right)
	{
#if USE_SIMD
		__m128 vd = _mm_mul_ps(left.m128(), right.m128());
		__m128 z = _mm_movehl_ps(vd, vd);
		__m128 y = _mm_shuffle_ps(vd, vd, 0x55);
		vd = _mm_add_ss(vd, y);
		vd = _mm_add_ss(vd, z);
		return _mm_cvtss_f32(vd);
#else
		GMfloat result = 0;
		for (int n = 0; n < T::dimension; n++)
		{
			result += left[n] * right[n];
		}
		return result;
#endif
	}

	static inline Vector3 cross(const Vector3& left, const Vector3& right)
	{
#if USE_SIMD
		GM_SIMD_float t0[] = { left[1],  left[2],  left[0],  0 },
			t1[] = { right[2], right[0], right[1], 0 },
			t2[] = { left[2],  left[0],  left[1],  0 },
			t3[] = { right[1], right[2], right[0], 0 };
		__m128 __t0 = _mm_load_ps(t0),
			__t1 = _mm_load_ps(t1),
			__t2 = _mm_load_ps(t2),
			__t3 = _mm_load_ps(t3);
		__m128 __m1 = _mm_mul_ps(__t0, __t1),
			__m2 = _mm_mul_ps(__t2, __t3);
		__m128 __r = _mm_sub_ps(__m1, __m2);
		return Vector3(__r);
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
		return 1.f / (float)fastInvSqrt(lengthSquare(left));
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
		for (int i = 0; i < 3; i++)
		{
			if (!fuzzyCompare(left[i], right[i]))
				return false;
		}
		return true;
	}

	static inline bool equals(const Vector4& left, const Vector4& right)
	{
#if USE_SIMD
		return left.m128().m128_f32 == right.m128().m128_f32;
#else
		for (int i = 0; i < 4; i++)
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
}
END_NS
#endif