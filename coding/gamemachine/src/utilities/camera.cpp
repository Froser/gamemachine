#include "stdafx.h"
#include "camera.h"
#include <cmath>

void Camera::calcCameraLookAt(const PositionState& state, REF CameraLookAt* lookAt)
{
	lookAt->lookAt_y = std::sin(state.pitch);
	GMfloat l = std::cos(state.pitch);
	lookAt->lookAt_x = l * std::sin(state.yaw);
	lookAt->lookAt_z = -l * std::cos(state.yaw);

	lookAt->position_x = state.positionX;
	lookAt->position_y = state.positionY;
	lookAt->position_z = state.positionZ;
}