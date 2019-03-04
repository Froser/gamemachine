#ifndef __GMCAMERA_H__
#define __GMCAMERA_H__
#include <gmcommon.h>
#include <gmtools.h>
#include <gmdxincludes.h>
#include <gmcom.h>

BEGIN_NS

//Camera
GM_ALIGNED_STRUCT(GMCameraLookAt)
{
	GMCameraLookAt() = default;
	GMCameraLookAt(const GMVec3& _lookAt, const GMVec3& _position, const GMVec3& _up)
		: lookAt(_lookAt)
		, position(_position)
		, up(_up)
	{
	}

	GMCameraLookAt(const GMVec3& _lookAt, const GMVec3& _position)
		: lookAt(_lookAt)
		, position(_position)
	{
	}

	GMVec3 lookAt = Zero<GMVec3>(); //!< 摄像机朝向，单位向量指示其方向
	GMVec3 position = Zero<GMVec3>(); //!< 摄像机位置
	GMVec3 up = GMVec3(0, 1, 0);

	static GMCameraLookAt makeLookAt(const GMVec3& _position, const GMVec3& _focusAt, const GMVec3& _up = GMVec3(0, 1, 0))
	{
		return GMCameraLookAt(_focusAt - _position, _position, _up);
	}
};

inline GMMat4 getViewMatrix(const GMCameraLookAt& lookAt)
{
	return LookAt(lookAt.position, lookAt.lookAt + lookAt.position, lookAt.up);
}

//Frustum
enum class GMFrustumType
{
	Perspective,
	Orthographic,
};

GM_PRIVATE_OBJECT(GMFrustum)
{
	GMFrustumType type = GMFrustumType::Perspective;

	union Parameters
	{
		struct
		{
			GMfloat fovy;
			GMfloat aspect;
		};

		struct
		{
			GMfloat left;
			GMfloat right;
			GMfloat bottom;
			GMfloat top;
		};
	} parameters;
	GMfloat n;
	GMfloat f;

	GMMat4 projectionMatrix;
	GMMat4 viewMatrix;
	GMMat4 inverseViewMatrix;

	bool dirty = true;
};

class GMCamera;
class GMSpriteGameObject;
class GMFrustum
{
	GM_DECLARE_PRIVATE_NGO(GMFrustum)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_GETTER(Near, n)
	GM_DECLARE_GETTER(Far, f)
	GM_DECLARE_GETTER(Parameters, parameters)

	friend class GMCamera;

	GMFrustum() = default;
	void setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f);
	void setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);

	void updateViewMatrix(const GMMat4& viewMatrix);

	//! 获取平截头体的6个平面方程。
	/*!
	  获取平截头体的6个平面方程，法线方向朝外。
	  \param planes 得到的平面方程。
	*/
	void getPlanes(GMFrustumPlanes& planes) const;

	const GMMat4& getProjectionMatrix() const;
	const GMMat4& getViewMatrix() const;
	const GMMat4& getInverseViewMatrix() const;

	inline bool isDirty() { D(d); return d->dirty; }
	inline void cleanDirty() { D(d); d->dirty = false; }

private:
	static bool isBoundingBoxInside(const GMFrustumPlanes& planes, const GMVec3 (&vertices)[8]);
};

inline bool operator ==(const GM_PRIVATE_NAME(GMFrustum)::Parameters& lhs, const GM_PRIVATE_NAME(GMFrustum)::Parameters& rhs)
{
	return lhs.fovy == rhs.fovy &&
		lhs.aspect == rhs.aspect &&
		lhs.left == rhs.left &&
		lhs.right == rhs.right &&
		lhs.bottom == rhs.bottom &&
		lhs.top == rhs.top;
}

inline bool operator !=(const GM_PRIVATE_NAME(GMFrustum)::Parameters& lhs, const GM_PRIVATE_NAME(GMFrustum)::Parameters& rhs)
{
	return !(lhs == rhs);
}

GM_PRIVATE_OBJECT(GMCamera)
{
	GMFrustum frustum;
	GMCameraLookAt lookAt;
};

class GM_EXPORT GMCamera : public GMObject
{
	GM_DECLARE_PRIVATE(GMCamera)
	GM_ALLOW_COPY_MOVE(GMCamera)
	GM_DECLARE_PROPERTY(LookAt, lookAt)
	GM_DECLARE_GETTER(Frustum, frustum)

public:
	GMCamera();

public:
	void setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);
	void setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f);
	void updateViewMatrix();
	void lookAt(const GMCameraLookAt& lookAt);

	//! 获取从屏幕出发，变换到世界的一条射线
	/*!
	  将屏幕上的一点，变换到世界坐标。世界坐标的深度值取远平面的值。此方法目前只对透视投影有效。
	  \param x 屏幕上的点的x坐标。
	  \param y 屏幕上的点的y坐标。
	  \param renderRect 渲染矩形大小。
	  \return 世界坐标。
	*/
	GMVec3 getRayToWorld(const GMRect& renderRect, GMint32 x, GMint32 y) const;

	//! 获取平截头体的6个平面方程。
	/*!
	获取平截头体的6个平面方程，法线方向朝外。
	\param planes 得到的平面方程。
	*/
	void getPlanes(GMFrustumPlanes& planes);

public:
	inline bool isDirty() const { D(d); return d->frustum.isDirty(); }
	inline void cleanDirty() { D(d); d->frustum.cleanDirty(); }
	inline const GMMat4& getProjectionMatrix() const { D(d); return d->frustum.getProjectionMatrix(); }
	inline const GMMat4& getViewMatrix() const { D(d); return d->frustum.getViewMatrix(); }
	const GMMat4& getInverseViewMatrix() const { D(d); return d->frustum.getInverseViewMatrix(); }

public:
	static bool isBoundingBoxInside(const GMFrustumPlanes& planes, const GMVec3(&vertices)[8]);
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT_UNALIGNED(GMCameraUtility)
{
	GMCamera* camera;
	GMfloat limitPitch = Radian(85.f);
	GMVec3 position;
	GMVec3 lookAt;
};

//! 用于响应鼠标移动时调整摄像机的一个便捷类
class GM_EXPORT GMCameraUtility
{
	GM_DECLARE_PRIVATE_NGO(GMCameraUtility)
	GM_DECLARE_PROPERTY(LimitPitch, limitPitch)

public:
	GMCameraUtility(GMCamera* camera = nullptr);

public:
	void update(GMfloat yaw, GMfloat pitch);
	void setCamera(GMCamera* camera);
};

END_NS
#endif