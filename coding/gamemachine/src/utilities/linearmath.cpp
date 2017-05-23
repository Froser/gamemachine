#include "stdafx.h"
#include "linearmath.h"

using namespace linear_math;

GMfloat& Vector2::operator [](GMint i)
{
	return m_data[i];
}

const GMfloat& Vector2::operator [](GMint i) const
{
	return m_data[i];
}

GMfloat& Vector3::operator [](GMint i)
{
	return m_data[i];
}

const GMfloat& Vector3::operator [](GMint i) const
{
	return m_data[i];
}

GMfloat& Vector4::operator [](GMint i)
{
	return m_data[i];
}

const GMfloat& Vector4::operator [](GMint i) const
{
	return m_data[i];
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
		__m128	__v = right.m_data[i].get128();
		__m128	__row0 = m_data[0].get128(),
			__row1 = m_data[1].get128(),
			__row2 = m_data[2].get128(),
			__row3 = m_data[3].get128();
		__m128	__x_mul_row0 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(0, 0, 0, 0)), __row0),
			__x_mul_row1 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(1, 1, 1, 1)), __row1),
			__x_mul_row2 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(2, 2, 2, 2)), __row2),
			__x_mul_row3 = _mm_mul_ps(_mm_shuffle_ps(__v, __v, simd_shuffle_param(3, 3, 3, 3)), __row3);
		__result = _mm_add_ps(__x_mul_row0, __x_mul_row1);
		__result = _mm_add_ps(__result, __x_mul_row2);
		__result = _mm_add_ps(__result, __x_mul_row3);
		ret[i].set128(__result);
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