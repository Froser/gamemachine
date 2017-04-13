#ifndef __PHOTOGRAPHER_H__
#define __PHOTOGRAPHER_H__
#include "common.h"
#include "vmath.h"
BEGIN_NS

struct CameraLookAt
{
	vmath::vec3 lookAt;
	vmath::vec3 position;
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
	return vmath::lookat(lookAt.position, lookAt.lookAt + lookAt.position, vmath::vec3(0, 1, 0));
}

class Camera
{
public:
	static void calcCameraLookAt(const PositionState& state, REF CameraLookAt& lookAt);
	static void adjustEyeOffset(const PositionState& state, const vmath::vec3& eyeOffset, REF CameraLookAt& lookAt);
};

END_NS
#endif