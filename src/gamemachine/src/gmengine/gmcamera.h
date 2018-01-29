#ifndef __GMCAMERA_H__
#define __GMCAMERA_H__
#include <gmcommon.h>
#include <tools.h>
#include <gmdxincludes.h>

BEGIN_NS

//Camera
GM_ALIGNED_STRUCT(GMCameraLookAt)
{
	GMCameraLookAt() = default;
	GMCameraLookAt(const glm::vec3& _lookAt, const glm::vec3& _position, const glm::vec3& _up)
		: lookAt(_lookAt)
		, position(_position)
		, up(_up)
	{
	}

	GMCameraLookAt(const glm::vec3& _lookAt, const glm::vec3& _position)
		: lookAt(_lookAt)
		, position(_position)
	{
	}

	glm::vec3 lookAt = glm::zero<glm::vec3>(); //!< 摄像机朝向，单位向量指示其方向
	glm::vec3 position = glm::zero<glm::vec3>(); //!< 摄像机位置
	glm::vec3 up = glm::vec3(0, 1, 0);
};

inline glm::mat4 getViewMatrix(const GMCameraLookAt& lookAt)
{
	return glm::lookAt(lookAt.position, lookAt.lookAt + lookAt.position, lookAt.up);
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
	GMPlane planes[6];
	union
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
	GMfloat n;
	GMfloat f;

	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

#if GM_USE_DX11
	//DirectX
	D3DXMATRIX dxProjMatrix;
	D3DXMATRIX dxViewMatrix;
#endif
};

class GMSpriteGameObject;
class GMFrustum : public GMObject
{
	DECLARE_PRIVATE(GMFrustum)

public:
	GMFrustum() = default;
	void setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f);
	void setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);

public:
	bool isPointInside(const glm::vec3& point);
	bool isBoundingBoxInside(const glm::vec3* vertices);
	void updateViewMatrix(const glm::mat4& viewMatrix);

public:
	inline GMFrustumType getType() { D(d); return d->type; }
	inline GMfloat getNear() { D(d); return d->n; }
	inline GMfloat getFar() { D(d); return d->f; }
	inline GMfloat getFovy() { D(d); GM_ASSERT(getType() == GMFrustumType::Perspective); return d->fovy; }

public:
	const glm::mat4& getProjectionMatrix();
	const glm::mat4& getViewMatrix();

#if GM_USE_DX11
	const D3DXMATRIX& getDxProjectionMatrix();
	const D3DXMATRIX& getDxViewMatrix();
#endif

private:
	void update();
	void dxUpdate();
};

GM_PRIVATE_OBJECT(GMCamera)
{
	GMFrustum frustum;
	GMCameraLookAt lookAt;
};

class GMCamera
{
	DECLARE_PRIVATE_NGO(GMCamera)

public:
	GMCamera();

public:
	void setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);
	void setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f);

	void synchronize(GMSpriteGameObject* gameObject);
	void synchronizeLookAt();

	void lookAt(const GMCameraLookAt& lookAt);
	GMFrustum& getFrustum() { D(d); return d->frustum; }

	//! 获取从屏幕出发，变换到世界的一条射线
	/*!
	  将屏幕上的一点，变换到世界坐标。世界坐标的深度值取远平面的值。此方法目前只对透视投影有效。
	  \param x 屏幕上的点的x坐标。
	  \param y 屏幕上的点的y坐标。
	  \return 世界坐标。
	*/
	glm::vec3 getRayToWorld(GMint x, GMint y) const;

public:
	inline const glm::mat4& getProjectionMatrix() { D(d); return getFrustum().getProjectionMatrix(); }
	inline const glm::mat4& getViewMatrix() { D(d); return getFrustum().getViewMatrix(); }
	inline const GMCameraLookAt& getLookAt() { D(d); return d->lookAt; }
};

END_NS
#endif