#if !GM_USE_DX11
namespace glm
{
	inline vec4 combine_vec4(const vec3& v3, const vec4& v4)
	{
		return vec4(v3, v4[3]);
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
	inline glm::quat& identity()
	{
		static glm::quat q(1, 0, 0, 0);
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

	inline vec3 inhomogeneous(const vec4& v4)
	{
		return vec3(v4.x / v4.w, v4.y / v4.w, v4.z / v4.w);
	}

	inline vec3 make_vec3(const gm::GMfloat(&v)[3])
	{
		return make_vec3(static_cast<const gm::GMfloat*>(v));
	}

	template <typename T>
	inline gm::GMfloat lengthSq(const T& left)
	{
		return dot(left, left);
	}

	inline vec3 safeNormalize(const vec3& vec, const vec3& n = vec3(1, 0, 0))
	{
		gm::GMfloat l2 = glm::length2(vec);
		if (l2 >= FLT_EPSILON*FLT_EPSILON)
		{
			return vec / Sqrt(l2);
		}
		else
		{
			return n;
		}
	}
}
#endif

const GMQuat g_GMQuat_identity = Identity<GMQuat>();

inline GMVec3::GMVec3(const GMVec4& V)
{
#if GM_USE_DX11
	v_ = V.v_;
#else
	v_ = glm::vec3(V.v_[0], V.v_[1], V.v_[2]);
#endif
}

template <>
inline GMVec2 Zero()
{
	GMVec2 V;
#if GM_USE_DX11
	V.v_ = DirectX::XMVectorZero();
#else
	V.v_ = glm::zero<glm::vec2>();
#endif
	return V;
}

template <>
inline GMVec3 Zero()
{
	GMVec3 V;
#if GM_USE_DX11
	V.v_ = DirectX::XMVectorZero();
#else
	V.v_ = glm::zero<glm::vec3>();
#endif
	return V;
}

template <>
inline GMVec4 Zero()
{
	GMVec4 V;
#if GM_USE_DX11
	V.v_ = DirectX::XMVectorZero();
#else
	V.v_ = glm::zero<glm::vec4>();
#endif
	return V;
}

template <>
inline GMMat4 Zero()
{
	GMMat4 V;
#if GM_USE_DX11
	V.v_.r[0] = DirectX::g_XMZero;
	V.v_.r[1] = DirectX::g_XMZero;
	V.v_.r[2] = DirectX::g_XMZero;
	V.v_.r[3] = DirectX::g_XMZero;
#else
	V.v_ = glm::zero<glm::mat4>();
#endif
	return V;
}

inline GMMat4 __getIdentityMat4()
{
	GMMat4 v;
#if GM_USE_DX11
	v.v_ = DirectX::XMMatrixIdentity();
#else
	v.v_ = glm::identity<glm::mat4>();
#endif
	return v;
}

inline GMQuat __getIdentityQuat()
{
	GMQuat v;
#if GM_USE_DX11
	v.v_ = DirectX::XMQuaternionIdentity();
#else
	v.v_ = glm::identity<glm::quat>();
#endif
	return v;
}

inline GMVec2 operator-(const GMVec2& V)
{
	GMVec2 R;
#if GM_USE_DX11
	R.v_ = DirectX::operator-(V.v_);
#else
	R.v_ = -V.v_;
#endif
	return R;
}

inline GMVec3 operator-(const GMVec3& V)
{
	GMVec3 R;
#if GM_USE_DX11
	R.v_ = DirectX::operator-(V.v_);
#else
	R.v_ = -V.v_;
#endif
	return R;
}

inline GMVec4 operator-(const GMVec4& V)
{
	GMVec4 R;
#if GM_USE_DX11
	R.v_ = DirectX::operator-(V.v_);
#else
	R.v_ = -V.v_;
#endif
	return R;
}

inline GMVec2 operator+(const GMVec2& V1, const GMVec2& V2)
{
	GMVec2 V;
#if GM_USE_DX11
	V.v_ = DirectX::operator+(V1.v_, V2.v_);
#else
	V.v_ = V1.v_ + V2.v_;
#endif
	return V;
}

inline GMVec2 operator-(const GMVec2& V1, const GMVec2& V2)
{
	GMVec2 V;
#if GM_USE_DX11
	V.v_ = DirectX::operator-(V1.v_, V2.v_);
#else
	V.v_ = V1.v_ - V2.v_;
#endif;
	return V;
}

inline GMVec3 operator+(const GMVec3& V1, const GMVec3& V2)
{
	GMVec3 V;
#if GM_USE_DX11
	V.v_ = DirectX::operator+(V1.v_, V2.v_);
#else
	V.v_ = V1.v_ + V2.v_;
#endif
	return V;
}

inline GMVec3& operator+=(GMVec3& V1, const GMVec3& V2)
{
#if GM_USE_DX11
	DirectX::operator+=(V1.v_, V1.v_);
#else
	V1 = V1 + V2;
#endif
	return V1;
}

inline GMVec3 operator-(const GMVec3& V1, const GMVec3& V2)
{
	GMVec3 V;
#if GM_USE_DX11
	V.v_ = DirectX::operator-(V1.v_, V2.v_);
#else
	V.v_ = V1.v_ - V2.v_;
#endif
	return V;
}

inline GMVec3& operator-=(GMVec3& V1, const GMVec3& V2)
{
#if GM_USE_DX11
	DirectX::operator-=(V1.v_, V2.v_);
#else
	V1 = V1 - V2;
#endif
	return V1;
}

inline GMVec3 operator*(const GMVec3& V1, gm::GMfloat S)
{
	GMVec3 V;
#if GM_USE_DX11
	V.v_ = DirectX::operator*(V1.v_, S);
#else
	V.v_ = V1.v_ * S;
#endif
	return V;
}

inline GMVec3& operator*=(GMVec3& V1, gm::GMfloat S)
{
#if GM_USE_DX11
	DirectX::operator*=(V1.v_, S);
#else
	V1 = V1 * S;
#endif;
	return V1;
}

inline GMVec3 operator/(const GMVec3& V1, gm::GMfloat S)
{
	GMVec3 V;
#if GM_USE_DX11
	V.v_ = DirectX::operator/(V1.v_, S);
#else
	V.v_ = V1.v_ / S;
#endif
	return V;
}

inline GMVec3& operator/=(GMVec3& V1, gm::GMfloat S)
{
#if GM_USE_DX11
	DirectX::operator/=(V1.v_, S);
#else
	V1 = V1 / S;
#endif
	return V1;
}

inline GMVec4 operator+(const GMVec4& V1, const GMVec4& V2)
{
	GMVec4 V;
#if GM_USE_DX11
	V.v_ = DirectX::operator+(V1.v_, V2.v_);
#else
	V.v_ = V1.v_ + V2.v_;
#endif;
	return V;
}

inline GMVec4 operator-(const GMVec4& V1, const GMVec4& V2)
{
	GMVec4 V;
#if GM_USE_DX11
	V.v_ = DirectX::operator-(V1.v_, V2.v_);
#else
	V.v_ = V1.v_ - V2.v_;
#endif
	return V;
}

inline GMVec4 operator*(const GMVec4& V, gm::GMfloat S)
{
	GMVec4 R;
#if GM_USE_DX11
	R.v_ = DirectX::operator*(V.v_, S);
#else
	R.v_ = V.v_ * S;
#endif
	return R;
}

inline bool operator==(const GMVec3& V1, const GMVec3& V2)
{
#if GM_USE_DX11
	return DirectX::XMVector3Equal(V1.v_, V2.v_);
#else
	return V1.v_ == V2.v_;
#endif;
}

inline GMMat4 operator*(const GMMat4& M1, const GMMat4& M2)
{
	GMMat4 R;
#if GM_USE_DX11
	R.v_ = M1.v_.operator*(M2.v_);
#else
	// opengl为列优先，为了计算先M1变换，再M2变换，应该用M2 * M1
	R.v_ = M2.v_ * M1.v_;
#endif
	return R;
}

inline GMVec3 operator*(const GMVec3& V1, const GMVec3& V2)
{
	GMVec3 R;
#if GM_USE_DX11
	R.v_ = DirectX::operator*(V1.v_, V2.v_);
#else
	R.v_ = V1.v_ * V2.v_;
#endif
	return R;
}

inline GMVec4 operator*(const GMVec4& V, const GMMat4& M)
{
	GMVec4 R;
#if GM_USE_DX11
	R.v_ = DirectX::XMVector4Transform(V.v_, M.v_);
#else
	R.v_ = M.v_ * V.v_;
#endif
	return R;
}

inline GMVec4 operator*(const GMVec4& V, const GMQuat& Q)
{
#if GM_USE_DX11
	return V * QuatToMatrix(Q);
#else
	GMVec4 R;
	R.v_ = Q.v_ * V.v_;
	return R;
#endif
}

inline GMVec3 operator*(const GMVec3& V, const GMQuat& Q)
{
	GMVec3 R;
#if GM_USE_DX11
	R.v_ = DirectX::XMVector3Transform(V.v_, QuatToMatrix(Q).v_);
#else
	R.v_ = Q.v_ * V.v_;
#endif
	return R;
}

inline GMMat4 QuatToMatrix(const GMQuat& quat)
{
	GMMat4 mat;
#if GM_USE_DX11
	mat.v_ = DirectX::XMMatrixRotationQuaternion(quat.v_);
#else
	mat.v_ = glm::mat4_cast(quat.v_);
#endif
	return mat;
}

inline GMMat4 LookAt(const GMVec3& position, const GMVec3& center, const GMVec3& up)
{
	GMMat4 mat;
#if GM_USE_DX11
	mat.v_ = DirectX::XMMatrixLookAtLH(position.v_, center.v_, up.v_);
#else
	mat.v_ = glm::lookAt(position.v_, center.v_, up.v_);
#endif
	return mat;
}

inline gm::GMfloat Dot(const GMVec2& V1, const GMVec2& V2)
{
#if GM_USE_DX11
	return DirectX::XMVectorGetX(DirectX::XMVector2Dot(V1.v_, V2.v_));
#else
	return glm::dot(V1.v_, V2.v_);
#endif
}

inline gm::GMfloat Dot(const GMVec3& V1, const GMVec3& V2)
{
#if GM_USE_DX11
	return DirectX::XMVectorGetX(DirectX::XMVector3Dot(V1.v_, V2.v_));
#else
	return glm::dot(V1.v_, V2.v_);
#endif
}

inline gm::GMfloat Dot(const GMVec4& V1, const GMVec4& V2)
{
#if GM_USE_DX11
	return DirectX::XMVectorGetX(DirectX::XMVector4Dot(V1.v_, V2.v_));
#else
	return glm::dot(V1.v_, V2.v_);
#endif
}

inline GMVec3 Normalize(const GMVec3& V)
{
	GMVec3 R;
#if GM_USE_DX11
	R.v_ = DirectX::XMVector3Normalize(V.v_);
#else
	R.v_ = glm::normalize(R.v_);
#endif
	return R;
}

inline GMVec3 FastNormalize(const GMVec3& V)
{
	GMVec3 R;
#if GM_USE_DX11
	R.v_ = DirectX::XMVector3Normalize(V.v_);
#else
	R.v_ = glm::fastNormalize(R.v_);
#endif
	return R;
}

inline GMVec3 SafeNormalize(const GMVec3& V, const GMVec3& Default)
{
	GMVec3 R;
#if GM_USE_DX11
	gm::GMfloat len = Length(V);
	if (len >= FLT_EPSILON * FLT_EPSILON)
	{
		gm::GMfloat sl = Sqrt(len);
		return V * (1.f / sl);
	}
	else
	{
		return Default;
	}
#else
	R.v_ = glm::safeNormalize(V.v_, Default.v_);
#endif
	return R;
}

inline GMVec3 MakeVector3(const gm::GMfloat* f)
{
	return GMVec3(f[0], f[1], f[2]);
}

inline GMVec3 MakeVector3(const GMVec4& V)
{
	GMVec3 R;
#if GM_USE_DX11
	R.v_ = V.v_;
#else
	R.v_ = glm::make_vec3(V.v_);
#endif
	return R;
}

inline GMVec4 CombineVector4(const GMVec3& V1, const GMVec4& V2)
{
	GMVec4 R;
#if GM_USE_DX11
	R.v_ = V1.v_;
	R.setW(V2.getW());
#else
	R.v_ = glm::combine_vec4(V1.v_, V2.v_);
#endif
	return R;
}

inline GMMat4 Translate(const GMVec3& V)
{
	GMMat4 M;
#if GM_USE_DX11
	M.v_ = DirectX::XMMatrixTranslationFromVector(V.v_);
#else
	M.v_ = glm::translate(V.v_);
#endif
	return M;
}

inline GMMat4 Translate(const GMVec4& V)
{
	GMMat4 M;
#if GM_USE_DX11
	M.v_ = DirectX::XMMatrixTranslationFromVector(V.v_);
#else
	M.v_ = glm::translate(V.v_);
#endif
	return M;
}

inline GMMat4 Scale(const GMVec3& V)
{
	GMMat4 M;
#if GM_USE_DX11
	M.v_ = DirectX::XMMatrixScalingFromVector(V.v_);
#else
	M.v_ = glm::scale(V.v_);
#endif
	return M;
}

inline GMQuat Rotate(gm::GMfloat Angle, const GMVec3& Axis)
{
	return Rotate(g_GMQuat_identity, Angle, Axis);
}

inline GMQuat Rotate(const GMQuat& Start, gm::GMfloat Angle, const GMVec3& Axis)
{
	GMQuat Q;
#if GM_USE_DX11
	Q.v_ = DirectX::XMQuaternionMultiply(Start.v_, DirectX::XMQuaternionRotationAxis(Axis.v_, Angle));

#else
	Q.v_ = glm::rotate(Start.v_, Angle, Axis.v_);
#endif
	return Q;
}

inline void GetTranslationFromMatrix(const GMMat4& M, OUT GMFloat4& F)
{
	GMFloat16 f16;
	M.loadFloat16(f16);
	F[0] = f16[3][0];
	F[1] = f16[3][1];
	F[2] = f16[3][2];
}

inline void GetScalingFromMatrix(const GMMat4& M, OUT GMFloat4& F)
{
	GMFloat16 f16;
	M.loadFloat16(f16);
	F[0] = f16[0][0];
	F[1] = f16[1][1];
	F[2] = f16[2][2];
}

inline GMMat4 Ortho(gm::GMfloat left, gm::GMfloat right, gm::GMfloat bottom, gm::GMfloat top, gm::GMfloat n, gm::GMfloat f)
{
	GMMat4 M;
#if GM_USE_DX11
	M.v_ = DirectX::XMMatrixOrthographicOffCenterLH(left, right, bottom, top, n, f);
#else
	M.v_ = glm::ortho(left, right, bottom, top, n, f);
#endif
	return M;
}

inline gm::GMint Length(const GMVec3& V)
{
#if GM_USE_DX11
	GMFloat4 f;
	DirectX::XMStoreFloat4(&f.v_, DirectX::XMVector3Length(V.v_));
	return f[0];
#else
	return glm::length(V.v_);
#endif
}

inline gm::GMint Length(const GMVec4& V)
{
#if GM_USE_DX11
	return DirectX::XMVectorGetX(DirectX::XMVector4Length(V.v_));
#else
	return glm::length(V.v_);
#endif
}

inline gm::GMint LengthSq(const GMVec3& V)
{
#if GM_USE_DX11
	return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(V.v_));
#else
	return glm::lengthSq(V.v_);
#endif
}

inline GMVec3 Cross(const GMVec3& V1, const GMVec3& V2)
{
	GMVec3 R;
#if GM_USE_DX11
	R.v_ = DirectX::XMVector3Cross(V1.v_, V2.v_);
#else
	R.v_ = glm::cross(V1.v_, V2.v_);
#endif
	return R;
}

inline GMMat4 Perspective(gm::GMfloat fovy, gm::GMfloat aspect, gm::GMfloat n, gm::GMfloat f)
{
	GMMat4 R;
#if GM_USE_DX11
	R.v_ = DirectX::XMMatrixPerspectiveFovLH(fovy, aspect, n, f);
#else
	R.v_ = glm::perspective(fovy, aspect, n, f);
#endif
	return R;
}

inline GMMat4 Transpose(const GMMat4& M)
{
	GMMat4 R;
#if GM_USE_DX11
	R.v_ = DirectX::XMMatrixTranspose(M.v_);
#else
	R.v_ = glm::transpose(M.v_);
#endif
	return R;
}

inline GMMat4 Inverse(const GMMat4& M)
{
	GMMat4 R;
#if GM_USE_DX11
	R.v_ = DirectX::XMMatrixInverse(nullptr, M.v_);
#else
	R.v_ = glm::inverse(M.v_);
#endif
	return R;
}

inline GMMat4 InverseTranspose(const GMMat4& M)
{
#if GM_USE_DX11
	return Transpose(Inverse(M));
#else
	GMMat4 R;
	R.v_ = glm::inverseTranspose(M.v_);
	return R;
#endif
}

inline GMQuat Lerp(const GMQuat& Q1, const GMQuat& Q2, gm::GMfloat T)
{
	GMQuat R;
#if GM_USE_DX11
	R.v_ = DirectX::XMQuaternionSlerp(Q1.v_, Q2.v_, T);
#else
	R.v_ = glm::lerp(Q1.v_, Q2.v_, T);
#endif
	return R;
}

template <typename T>
inline T Lerp(const T& S, const T& E, gm::GMfloat P)
{
	return (E - S) * P + S;
}

inline GMVec3 Inhomogeneous(const GMVec4& V)
{
	GMVec3 R;
#if GM_USE_DX11
	R.v_ = V.v_;
	DirectX::operator/=(R.v_, V.getW());
#else
	R.v_ = glm::inhomogeneous(V.v_);
#endif
	return R;
}

inline GMMat4 Inhomogeneous(const GMMat4& M)
{
	GMMat4 R(M);
	GMFloat16 f16;
	R.loadFloat16(f16);
	f16[0][3] =
	f16[1][3] =
	f16[2][3] =
	f16[3][0] =
	f16[3][1] =
	f16[3][2] = 0.f;
	f16[3][3] = 1.f;
	R.setFloat16(f16);
	return R;
}

inline void CopyToArray(const GMVec3& V, gm::GMfloat* array)
{
	GMFloat4 f4;
	V.loadFloat4(f4);
	array[0] = f4[0];
	array[1] = f4[1];
	array[2] = f4[2];
}

inline void CopyToArray(const GMVec4& V, gm::GMfloat* array)
{
	GMFloat4 f4;
	V.loadFloat4(f4);
	array[0] = f4[0];
	array[1] = f4[1];
	array[2] = f4[2];
	array[3] = f4[3];
}

#if GM_USE_DX11
template <typename T>
inline gm::GMfloat* ValuePointer(const T& data)
{
	return (gm::GMfloat*)(&data.v_);
}

template <>
inline gm::GMfloat* ValuePointer(const GMMat4& data)
{
	return (gm::GMfloat*)(&(data.v_));
}
#else
template <typename T>
inline gm::GMfloat* ValuePointer(const T& data)
{
	return (gm::GMfloat*)(&data.v_.x);
}

template <>
inline gm::GMfloat* ValuePointer(const GMMat4& data)
{
	return (gm::GMfloat*)(&data.v_[0]);
}
#endif

template <>
inline gm::GMfloat* ValuePointer(const GMFloat4& data)
{
	return (gm::GMfloat*)(&(data[0]));
}