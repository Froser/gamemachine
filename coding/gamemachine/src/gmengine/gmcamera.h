#ifndef __GMCAMERA_H__
#define __GMCAMERA_H__
#include <gmcommon.h>
#include <utilities.h>
BEGIN_NS

class GMSpriteGameObject;

class GMFrustum : public GMObject
{
	DECLARE_PRIVATE(GMFrustum)

public:
	GMFrustum() = default;
	void initOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f);
	void initPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);

public:
	void update();
	bool isPointInside(const glm::vec3& point);
	bool isBoundingBoxInside(const glm::vec3* vertices);
	glm::mat4 getProjection();
	void updateViewMatrix(glm::mat4& viewMatrix, glm::mat4& projMatrix);
};

GM_PRIVATE_OBJECT(GMCamera)
{
	GMFrustum frustum;
	PositionState state;
	CameraLookAt lookAt;
};

class GMCamera
{
	DECLARE_PRIVATE(GMCamera)

public:
	GMCamera();

public:
	void setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);
	void setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f);

	void synchronize(GMSpriteGameObject* gameObject);
	void synchronizeLookAt();

	void lookAt(const CameraLookAt& lookAt);

	GMFrustum& getFrustum() { D(d); return d->frustum; }
	const PositionState& getPositionState() { D(d); return d->state; }
};

END_NS
#endif