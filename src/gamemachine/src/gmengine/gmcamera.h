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
	GMCameraLookAt(const GMVec3& _lookDirection, const GMVec3& _position, const GMVec3& _up)
		: lookDirection(_lookDirection)
		, position(_position)
		, up(_up)
	{
	}

	GMCameraLookAt(const GMVec3& _lookDirection, const GMVec3& _position)
		: lookDirection(_lookDirection)
		, position(_position)
	{
	}

	GMVec3 lookDirection = Zero<GMVec3>(); //!< 摄像机朝向，单位向量指示其方向。它不一定是单位向量。用于指示方向时需要单位化。
	GMVec3 position = Zero<GMVec3>(); //!< 摄像机位置
	GMVec3 up = GMVec3(0, 1, 0);

	bool operator == (const GMCameraLookAt& rhs)
	{
		return this->lookDirection == rhs.lookDirection && this->position == rhs.position && this->up == rhs.up;
	}

	bool operator != (const GMCameraLookAt& rhs)
	{
		return !(*this == rhs);
	}

	static GMCameraLookAt makeLookAt(const GMVec3& _position, const GMVec3& _focusAt, const GMVec3& _up = GMVec3(0, 1, 0))
	{
		return GMCameraLookAt(_focusAt - _position, _position, _up);
	}
};

inline GMMat4 getViewMatrix(const GMCameraLookAt& lookAt)
{
	return LookAt(lookAt.position, lookAt.lookDirection + lookAt.position, lookAt.up);
}

//Frustum
enum class GMFrustumType
{
	Perspective,
	Orthographic,
};

class GMCamera;
class GMSpriteGameObject;
union GMFrustumParameters
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
};

GM_PRIVATE_CLASS(GMFrustum);
class GMFrustum
{
	GM_DECLARE_PRIVATE(GMFrustum)
	GM_DISABLE_COPY_ASSIGN(GMFrustum)
	GM_DECLARE_GETTER_ACCESSOR(GMfloat, Near, public)
	GM_DECLARE_GETTER_ACCESSOR(GMfloat, Far, public)
	GM_DECLARE_GETTER_ACCESSOR(GMFrustumParameters, Parameters, public)

	friend class GMCamera;

	GMFrustum();
	~GMFrustum();

	bool operator== (const GMFrustum&);
	bool operator!= (const GMFrustum&);

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

	bool isDirty();
	void cleanDirty();

private:
	static bool isBoundingBoxInside(const GMFrustumPlanes& planes, const GMVec3 (&vertices)[8]);
};

inline bool operator ==(const GMFrustumParameters& lhs, const GMFrustumParameters& rhs)
{
	return lhs.fovy == rhs.fovy &&
		lhs.aspect == rhs.aspect &&
		lhs.left == rhs.left &&
		lhs.right == rhs.right &&
		lhs.bottom == rhs.bottom &&
		lhs.top == rhs.top;
}

inline bool operator !=(const GMFrustumParameters& lhs, const GMFrustumParameters& rhs)
{
	return !(lhs == rhs);
}

GM_PRIVATE_CLASS(GMCamera);
class GM_EXPORT GMCamera : public IDestroyObject
{
	GM_DECLARE_PRIVATE(GMCamera)
	GM_DECLARE_GETTER_ACCESSOR(GMCameraLookAt, LookAt, public)
	GM_DECLARE_GETTER_ACCESSOR(GMFrustum, Frustum, public)

public:
	GMCamera();
	GMCamera(const GMCamera&);
	GMCamera(GMCamera&&) GM_NOEXCEPT;
	GMCamera& operator=(const GMCamera&);
	GMCamera& operator=(GMCamera&&) GM_NOEXCEPT;
	~GMCamera();

public:
	bool operator==(const GMCamera& rhs);
	bool operator!=(const GMCamera& rhs);

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

	bool isDirty() const;
	void cleanDirty();
	const GMMat4& getProjectionMatrix() const;
	const GMMat4& getViewMatrix() const;
	const GMMat4& getInverseViewMatrix() const;

public:
	static bool isBoundingBoxInside(const GMFrustumPlanes& planes, const GMVec3(&vertices)[8]);
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_CLASS(GMCameraUtility);
//! 用于响应鼠标移动时调整摄像机的一个便捷类
class GM_EXPORT GMCameraUtility
{
	GM_DECLARE_PRIVATE(GMCameraUtility)
	GM_DECLARE_PROPERTY(GMfloat, LimitPitch)

public:
	GMCameraUtility(GMCamera* camera = nullptr);
	~GMCameraUtility();

public:
	void update(GMfloat yaw, GMfloat pitch);
	void setCamera(GMCamera* camera);
};

GM_PRIVATE_OBJECT_UNALIGNED(GMCameraWalker)
{
	GMCamera* camera;
};

END_NS
#endif