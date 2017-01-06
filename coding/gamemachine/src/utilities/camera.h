#ifndef __PHOTOGRAPHER_H__
#define __PHOTOGRAPHER_H__
#include "common.h"
BEGIN_NS

struct CameraLookAt
{
	GMfloat lookAt_x, lookAt_y, lookAt_z;
	GMfloat position_x, position_y, position_z;
};

struct PositionState
{
	GMfloat lookAtRad;
	GMfloat lookUpRad;
	GMfloat positionX, positionY, positionZ;
	GMfloat lookUpLimitRad;
};

class Camera
{
public:
	static void calcCameraLookAt(const PositionState& state, REF CameraLookAt* cameraLookAt);
};

END_NS
#endif