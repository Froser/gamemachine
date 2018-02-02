#ifdef GM_USE_DX11
#	define GMMATH_COPY_CONSTRUCTOR(className)		\
	className::className(const className& rhs)		\
	{												\
		if (IS_OPENGL)								\
		{											\
			gl_ = rhs.gl_;							\
		}											\
		else if (IS_DX)								\
		{											\
			dx_ = rhs.dx_;							\
		}											\
	}
#else
#	define GMMATH_COPY_CONSTRUCTOR(className)		\
	className::className(const className& rhs)		\
	{												\
		gl_ = rhs.gl_;								\
	}
#endif

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

namespace gmmath
{
	GMMATH_COPY_CONSTRUCTOR(GMVec2);
	GMMATH_COPY_CONSTRUCTOR(GMVec3);
	GMMATH_COPY_CONSTRUCTOR(GMVec4);
	GMMATH_COPY_CONSTRUCTOR(GMMat4);
	GMMATH_COPY_CONSTRUCTOR(GMQuat);

	GMVec2 __getZeroVec2()
	{
		GMVec2 v;
#if GM_USE_DX11
		if (IS_OPENGL)
		{
			v.gl_ = glm::zero<glm::vec2>();
		}
		else if (IS_DX)
		{
		}
#else
		v.gl_ = glm::zero<glm::vec2>();
#endif
		return v;
	}

	GMVec3 __getZeroVec3()
	{
		GMVec3 v;
#if GM_USE_DX11
		if (IS_OPENGL)
		{
			v.gl_ = glm::zero<glm::vec3>();
		}
		else if (IS_DX)
		{
		}
#else
		v.gl_ = glm::zero<glm::vec3>();
#endif
		return v;
	}

	GMVec4 __getZeroVec4()
	{
		GMVec4 v;
#if GM_USE_DX11
		if (IS_OPENGL)
		{
			v.gl_ = glm::zero<glm::vec4>();
		}
		else if (IS_DX)
		{
		}
#else
		v.gl_ = glm::zero<glm::vec4>();
#endif
		return v;
	}

	GMMat4 __getIdentityMat4()
	{
		GMMat4 v;
#if GM_USE_DX11
		if (IS_OPENGL)
		{
			v.gl_ = glm::identity<glm::mat4>();
		}
		else if (IS_DX)
		{
		}
#else
		v.gl_ = glm::identity<glm::mat4>();
#endif
		return v;
	}

	GMMat4 __mul(const GMMat4& M1, const GMMat4& M2)
	{
		GMMat4 result;
#if GM_USE_DX11
		if (IS_OPENGL)
		{
			// opengl为列优先，为了计算先M1变换，再M2变换，应该用M2 * M1
			result.gl_ = M2.gl_ * M1.gl_;
		}
		else if (IS_DX)
		{
			result.dx_ = DirectX::XMMatrixMultiply(M1.dx_, M2.dx_);
		}
#else
		result.dx_ = DirectX::XMMatrixMultiply(M1.dx_, M2.dx_);
#endif
		return result;
	}

	GMVec4 __mul(const GMVec4& V, const GMMat4& M)
	{
		GMVec4 result;
#if GM_USE_DX11
		if (IS_OPENGL)
		{
			// opengl为列优先，为了计算先M1变换，再M2变换，应该用M2 * M1
			result.gl_ = M.gl_ * V.gl_;
		}
		else if (IS_DX)
		{
			//D3DXMatrixMultiply(&result.dx_, &M1.dx_, &M2.dx_);
			result.dx_ = DirectX::XMVector3Transform(V.dx_, M.dx_);
		}
#else
		result.dx_ = DirectX::XMVector3Transform(V.dx_, M.dx_);
#endif
		return result;
	}
}