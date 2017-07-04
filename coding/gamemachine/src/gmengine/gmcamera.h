#ifndef __GMCAMERA_H__
#define __GMCAMERA_H__
#include "common.h"
#include "foundation/utilities/utilities.h"
BEGIN_NS

class GMSpriteGameObject;

GM_PRIVATE_OBJECT(GMCamera)
{
	Frustum frustum;
	PositionState state;
	CameraLookAt lookAt;
};

class GMCamera
{
	DECLARE_PRIVATE(GMCamera)

public:
	GMCamera();

public:
	void synchronize(GMSpriteGameObject* gameObject);
	void apply();

	Frustum& getFrustum() { D(d); return d->frustum; }
	const PositionState& getPositionState() { D(d); return d->state; }
};

END_NS
#endif