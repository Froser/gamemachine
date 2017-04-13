#include "stdafx.h"
#include "camera.h"
#include <cmath>

void Camera::calcCameraLookAt(const PositionState& state, REF CameraLookAt& lookAt)
{
	lookAt.lookAt[1] = std::sin(state.pitch);
	GMfloat l = std::cos(state.pitch);
	lookAt.lookAt[0] = l * std::sin(state.yaw);
	lookAt.lookAt[2] = -l * std::cos(state.yaw);

	lookAt.position = state.position;
}

void Camera::adjustEyeOffset(const PositionState& state, const vmath::vec3& eyeOffset, REF CameraLookAt& lookAt)
{
	/*
	vmath::vec4 r(lookAt.lookAt[0], lookAt.lookAt[1], lookAt.lookAt[2], 1);
	vmath::vec3 up(0, 1.f, 0);
	vmath::mat4 m = vmath::rotate(DEG(state.yaw), up);
	vmath::vec4 rotatedNormal = r * m;
	GMfloat len = vmath::fast_length(eyeOffset);
	vmath::vec3 offset = VEC3(rotatedNormal) * len;

	lookAt.position += offset;
	*/
}