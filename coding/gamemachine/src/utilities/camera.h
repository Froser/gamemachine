#ifndef __PHOTOGRAPHER_H__
#define __PHOTOGRAPHER_H__
#include "common.h"
#include "vmath.h"
BEGIN_NS

struct CameraLookAt
{
	GMfloat lookAt_x, lookAt_y, lookAt_z;
	GMfloat position_x, position_y, position_z;
};

struct PositionState
{
	GMfloat yaw;
	GMfloat pitch;
	vmath::vec3 position;
	GMfloat pitchLimitRad;
};

inline vmath::mat4 getViewMatrix(const CameraLookAt& lookAt)
{
	return vmath::mat4 (
		vmath::lookat(vmath::vec3(lookAt.position_x, lookAt.position_y, lookAt.position_z),
			vmath::vec3(lookAt.lookAt_x + lookAt.position_x, lookAt.lookAt_y + lookAt.position_y, lookAt.lookAt_z + lookAt.position_z),
			vmath::vec3(0, 1, 0)
		)
	);
}

class Camera
{
public:
	static void calcCameraLookAt(const PositionState& state, REF CameraLookAt* cameraLookAt);
};

END_NS
#endif