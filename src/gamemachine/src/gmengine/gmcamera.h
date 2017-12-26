#ifndef __GMCAMERA_H__
#define __GMCAMERA_H__
#include <gmcommon.h>
#include <tools.h>
BEGIN_NS

//Camera
GM_ALIGNED_STRUCT(GMCameraLookAt)
{
	glm::vec3 lookAt;
	glm::vec3 position;
};

GM_ALIGNED_STRUCT(GMPositionState)
{
	glm::vec3 position;
	glm::quat lookAt;
};

inline glm::mat4 getViewMatrix(const GMCameraLookAt& lookAt)
{
	return glm::lookAt(lookAt.position, lookAt.lookAt + lookAt.position, glm::vec3(0, 1, 0));
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
	void updateViewMatrix(glm::mat4& viewMatrix);

public:
	inline const glm::mat4& getProjectionMatrix() { D(d); return d->projMatrix; }
	inline const glm::mat4& getViewMatrix() { D(d); return d->viewMatrix; }

private:
	void update();
};

GM_PRIVATE_OBJECT(GMCamera)
{
	GMFrustum frustum;
	GMPositionState state;
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

public:
	inline const GMPositionState& getPositionState() { D(d); return d->state; }
	inline const glm::mat4& getProjectionMatrix() { D(d); return getFrustum().getProjectionMatrix(); }
	inline const glm::mat4& getViewMatrix() { D(d); return getFrustum().getViewMatrix(); }
	inline const GMCameraLookAt& getLookAt() { D(d); return d->lookAt; }
};

END_NS
#endif