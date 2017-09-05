#ifndef __GMCAMERA_H__
#define __GMCAMERA_H__
#include "common.h"
#include <utilities.h>
BEGIN_NS

class GMSpriteGameObject;

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
	void initPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);
	void initOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f);

	void synchronize(GMSpriteGameObject* gameObject);
	void apply();

	GMFrustum& getFrustum() { D(d); return d->frustum; }
	const PositionState& getPositionState() { D(d); return d->state; }
};

END_NS
#endif