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