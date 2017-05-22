#include "stdafx.h"
#include "linearmath.h"

using namespace linear_math;

template <typename T>
T vec_add(const T& left, const T& right)
{
#if USE_SIMD
	__m128 __left = _mm_load_ps(&left[0]);
	__m128 __right = _mm_load_ps(&right[0]);
	__m128 __result = _mm_add_ps(__left, __right);
	return T(__result);
#else
	return typename T::Base::operator +(right);
#endif
}

template <typename T>
T vec_sub(const T& left, const T& right)
{
#if USE_SIMD
	__m128 __left = _mm_load_ps(&left[0]);
	__m128 __right = _mm_load_ps(&right[0]);
	__m128 __result = _mm_sub_ps(__left, __right);
	return T(__result);
#else
	return typename T::Base::operator -(right);
#endif
}

Vector3 Vector3::operator - () const
{
#if USE_SIMD
	__m128 r = _mm_xor_ps(m_vec128, simd_zeroMask);
	return Vector3(_mm_and_ps(r, _mm_castsi128_ps(simd_FFF0Mask)));
#else
	return Vector3(-m_data[0], -m_data[1], -m_data[2]);
#endif
}

Vector3 Vector3::operator +(const Vector3& right) const
{
	return vec_add(*this, right);
}

Vector3 Vector3::operator -(const Vector3& right) const
{
	return vec_sub(*this, right);
}

Vector3 Vector3::operator *(GMfloat right) const
{
#if USE_SIMD
	GM_SIMD_float c[] = { right, right, right, 0 };
	__m128 __left = _mm_load_ps(&m_data[0]);
	__m128 __right = _mm_load_ps(c);
	__m128 __result = _mm_mul_ps(__left, __right);
	return Vector3(__result);
#else
	return typename T::Base::operator *(right);
#endif
}

Vector3 Vector3::operator /(GMfloat right) const
{
#if USE_SIMD
	GM_SIMD_float _right[] = { right, right, right, 0 };
	__m128 __left = _mm_load_ps(&m_data[0]);
	__m128 __right = _mm_load_ps(_right);
	__m128 __result = _mm_div_ps(__left, __right);
	return Vector3(__result);
#else
	return typename T::Base::operator /(right);
#endif
}

Vector3& Vector3::operator += (const Vector3& right)
{
#if USE_SIMD
	m_vec128 = _mm_add_ps(m_vec128, right.m_vec128);
	return *this;
#else
	return typename T::Base::operator +=(right);
#endif
}

Vector3& Vector3::operator += (GMfloat right)
{
#if USE_SIMD
	GM_SIMD_float _right[] = { right, right, right, 0 };
	__m128 __right = _mm_load_ps(_right);
	m_vec128 = _mm_add_ps(m_vec128, __right);
	return *this;
#else
	return typename T::Base::operator +=(right);
#endif
}

Vector3& Vector3::operator -= (GMfloat right)
{
#if USE_SIMD
	GM_SIMD_float _right[] = { right, right, right, 0 };
	__m128 __right = _mm_load_ps(_right);
	m_vec128 = _mm_sub_ps(m_vec128, __right);
	return *this;
#else
	return typename T::Base::operator -=(right);
#endif
}

Vector3& Vector3::operator *= (GMfloat right)
{
#if USE_SIMD
	GM_SIMD_float _right[] = { right, right, right, 0 };
	__m128 __right = _mm_load_ps(_right);
	m_vec128 = _mm_mul_ps(m_vec128, __right);
	return *this;
#else
	return typename T::Base::operator *=(right);
#endif
}

Vector3& Vector3::operator /= (GMfloat right)
{
#if USE_SIMD
	GM_SIMD_float _right[] = { right, right, right, 0 };
	__m128 __right = _mm_load_ps(_right);
	m_vec128 = _mm_div_ps(m_vec128, __right);
	return *this;
#else
	return typename T::Base::operator /=(right);
#endif
}

Vector4 Vector4::operator - () const
{
	//TODO 可以优化
	return Vector4(-m_data[0], -m_data[1], -m_data[2], -m_data[3]);
}

Vector4 Vector4::operator +(const Vector4& right) const
{
	return vec_add(*this, right);
}

Vector4 Vector4::operator -(const Vector4& right) const
{
	return vec_sub(*this, right);
}

Vector4 Vector4::operator *(GMfloat right) const
{
#if USE_SIMD
	GM_SIMD_float c[] = { right, right, right, 0 };
	__m128 __right = _mm_load_ps(c);
	__m128 __result = _mm_mul_ps(m_vec128, __right);
	return Vector4(__result);
#else
	return typename T::Base::operator *(right);
#endif
}

Vector4 Vector4::operator /(GMfloat right) const
{
#if USE_SIMD
	GM_SIMD_float c[] = { right, right, right, right };
	__m128 __right = _mm_load_ps(c);
	__m128 __result = _mm_div_ps(m_vec128, __right);
	return Vector4(__result);
#else
	return typename T::Base::operator /(right);
#endif
}

Vector4& Vector4::operator *= (GMfloat right)
{
#if USE_SIMD
	GM_SIMD_float _right[] = { right, right, right, right };
	__m128 __right = _mm_load_ps(_right);
	m_vec128 = _mm_mul_ps(m_vec128, __right);
	return *this;
#else
	return typename T::Base::operator *=(right);
#endif
}

Vector4& Vector4::operator /= (GMfloat right)
{
#if USE_SIMD
	GM_SIMD_float _right[] = { right, right, right, right };
	__m128 __right = _mm_load_ps(_right);
	m_vec128 = _mm_div_ps(m_vec128, __right);
	return *this;
#else
	return typename T::Base::operator /=(right);
#endif
}

Matrix4x4 Matrix4x4::identity()
{
#if USE_SIMD
	static const Matrix4x4
		identityMatrix(
			_mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f),
			_mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f),
			_mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f),
			_mm_set_ps(1.0f, 1.0f, 0.0f, 0.0f));
#else
	static const Matrix4x4
		identityMatrix(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		);
#endif
	return identityMatrix;
}

Matrix4x4 Matrix4x4::operator *(const Matrix4x4& right) const
{
#if USE_SIMD
	Matrix4x4 ret;
	__m128 __result;
	for (int i = 0; i < 4; i++)
	{
		__m128	__v = right.m_data[i].m128();
		__m128	__row0 = m_data[0].m128(),
			__row1 = m_data[1].m128(),
			__row2 = m_data[2].m128(),
			__row3 = m_data[3].m128();
		__m128	__x_mul_row0 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(0, 0, 0, 0)), __row0),
			__x_mul_row1 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(1, 1, 1, 1)), __row1),
			__x_mul_row2 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(2, 2, 2, 2)), __row2),
			__x_mul_row3 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(3, 3, 3, 3)), __row3);
		__result = _mm_add_ps(__x_mul_row0, __x_mul_row1);
		__result = _mm_add_ps(__result, __x_mul_row2);
		__result = _mm_add_ps(__result, __x_mul_row3);
		ret[i].set_m128(__result);
	}
	return ret;
#else
	Matrix4x4 result(0);
	for (GMint j = 0; j < w; j++)
	{
		for (GMint i = 0; i < w; i++)
		{
			GMfloat sum = 0;
			for (GMint n = 0; n < w; n++)
			{
				sum += m_data[n][i] * right[j][n];
			}
			result[j][i] = sum;
		}
	}
	return result;
#endif
}

Matrix4x4 Matrix4x4::transpose() const
{
	Matrix4x4 result;
	GMint x, y;

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			result[x][y] = m_data[y][x];
		}
	}

	return result;
}

const GMfloat* Matrix4x4::data() const
{
	return &m_data[0][0];
}