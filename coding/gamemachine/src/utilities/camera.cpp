#include "stdafx.h"
#include "camera.h"
#include <cmath>

void Camera::calcCameraLookAt(const PositionState& state, REF CameraLookAt* lookAt)
{
	lookAt->lookAt_y = std::sin(state.lookUpRad);
	GMfloat l = std::cos(state.lookUpRad);
	lookAt->lookAt_x = l * std::sin(state.lookAtRad);
	lookAt->lookAt_z = -l * std::cos(state.lookAtRad);

	lookAt->position_x = state.positionX;
	lookAt->position_y = state.positionY;
	lookAt->position_z = state.positionZ;
}